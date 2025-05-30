// SPDX-License-Identifier: LGPL-2.1
/*
 *
 *   Copyright (C) International Business Machines  Corp., 2002, 2011
 *                 Etersoft, 2012
 *   Author(s): Steve French (sfrench@us.ibm.com)
 *              Jeremy Allison (jra@samba.org) 2006
 *              Pavel Shilovsky (pshilovsky@samba.org) 2012
 *
 */

#include <linux/fs.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/net.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <asm/processor.h>
#include <linux/mempool.h>
#include <linux/highmem.h>
#include <crypto/aead.h>
#include "cifsglob.h"
#include "cifsproto.h"
#include "smb2proto.h"
#include "cifs_debug.h"
#include "smb2status.h"
#include "smb2glob.h"

static int
smb3_crypto_shash_allocate(struct TCP_Server_Info *server)
{
	struct cifs_secmech *p = &server->secmech;
	int rc;

	rc = cifs_alloc_hash("hmac(sha256)", &p->hmacsha256);
	if (rc)
		goto err;

	rc = cifs_alloc_hash("cmac(aes)", &p->aes_cmac);
	if (rc)
		goto err;

	return 0;
err:
	cifs_free_hash(&p->hmacsha256);
	return rc;
}

int
smb311_crypto_shash_allocate(struct TCP_Server_Info *server)
{
	struct cifs_secmech *p = &server->secmech;
	int rc = 0;

	rc = cifs_alloc_hash("hmac(sha256)", &p->hmacsha256);
	if (rc)
		return rc;

	rc = cifs_alloc_hash("cmac(aes)", &p->aes_cmac);
	if (rc)
		goto err;

	rc = cifs_alloc_hash("sha512", &p->sha512);
	if (rc)
		goto err;

	return 0;

err:
	cifs_free_hash(&p->aes_cmac);
	cifs_free_hash(&p->hmacsha256);
	return rc;
}


static
int smb3_get_sign_key(__u64 ses_id, struct TCP_Server_Info *server, u8 *key)
{
	struct cifs_chan *chan;
	struct TCP_Server_Info *pserver;
	struct cifs_ses *ses = NULL;
	int i;
	int rc = 0;
	bool is_binding = false;

	spin_lock(&cifs_tcp_ses_lock);

	/* If server is a channel, select the primary channel */
	pserver = SERVER_IS_CHAN(server) ? server->primary_server : server;

	list_for_each_entry(ses, &pserver->smb_ses_list, smb_ses_list) {
		if (ses->Suid == ses_id)
			goto found;
	}
	trace_smb3_ses_not_found(ses_id);
	cifs_server_dbg(FYI, "%s: Could not find session 0x%llx\n",
			__func__, ses_id);
	rc = -ENOENT;
	goto out;

found:
	spin_lock(&ses->ses_lock);
	spin_lock(&ses->chan_lock);

	is_binding = (cifs_chan_needs_reconnect(ses, server) &&
		      ses->ses_status == SES_GOOD);
	if (is_binding) {
		/*
		 * If we are in the process of binding a new channel
		 * to an existing session, use the master connection
		 * session key
		 */
		memcpy(key, ses->smb3signingkey, SMB3_SIGN_KEY_SIZE);
		spin_unlock(&ses->chan_lock);
		spin_unlock(&ses->ses_lock);
		goto out;
	}

	/*
	 * Otherwise, use the channel key.
	 */

	for (i = 0; i < ses->chan_count; i++) {
		chan = ses->chans + i;
		if (chan->server == server) {
			memcpy(key, chan->signkey, SMB3_SIGN_KEY_SIZE);
			spin_unlock(&ses->chan_lock);
			spin_unlock(&ses->ses_lock);
			goto out;
		}
	}
	spin_unlock(&ses->chan_lock);
	spin_unlock(&ses->ses_lock);

	cifs_dbg(VFS,
		 "%s: Could not find channel signing key for session 0x%llx\n",
		 __func__, ses_id);
	rc = -ENOENT;

out:
	spin_unlock(&cifs_tcp_ses_lock);
	return rc;
}

static struct cifs_ses *
smb2_find_smb_ses_unlocked(struct TCP_Server_Info *server, __u64 ses_id)
{
	struct TCP_Server_Info *pserver;
	struct cifs_ses *ses;

	/* If server is a channel, select the primary channel */
	pserver = SERVER_IS_CHAN(server) ? server->primary_server : server;

	list_for_each_entry(ses, &pserver->smb_ses_list, smb_ses_list) {
		if (ses->Suid != ses_id)
			continue;

		spin_lock(&ses->ses_lock);
		if (ses->ses_status == SES_EXITING) {
			spin_unlock(&ses->ses_lock);
			continue;
		}
		cifs_smb_ses_inc_refcount(ses);
		spin_unlock(&ses->ses_lock);
		return ses;
	}

	return NULL;
}

static int smb2_get_sign_key(struct TCP_Server_Info *server,
			     __u64 ses_id, u8 *key)
{
	struct cifs_ses *ses;
	int rc = -ENOENT;

	if (SERVER_IS_CHAN(server))
		server = server->primary_server;

	spin_lock(&cifs_tcp_ses_lock);
	list_for_each_entry(ses, &server->smb_ses_list, smb_ses_list) {
		if (ses->Suid != ses_id)
			continue;

		rc = 0;
		spin_lock(&ses->ses_lock);
		switch (ses->ses_status) {
		case SES_EXITING: /* SMB2_LOGOFF */
		case SES_GOOD:
			if (likely(ses->auth_key.response)) {
				memcpy(key, ses->auth_key.response,
				       SMB2_NTLMV2_SESSKEY_SIZE);
			} else {
				rc = -EIO;
			}
			break;
		default:
			rc = -EAGAIN;
			break;
		}
		spin_unlock(&ses->ses_lock);
		break;
	}
	spin_unlock(&cifs_tcp_ses_lock);
	return rc;
}

static struct cifs_tcon *
smb2_find_smb_sess_tcon_unlocked(struct cifs_ses *ses, __u32  tid)
{
	struct cifs_tcon *tcon;

	list_for_each_entry(tcon, &ses->tcon_list, tcon_list) {
		if (tcon->tid != tid)
			continue;
		++tcon->tc_count;
		return tcon;
	}

	return NULL;
}

/*
 * Obtain tcon corresponding to the tid in the given
 * cifs_ses
 */

struct cifs_tcon *
smb2_find_smb_tcon(struct TCP_Server_Info *server, __u64 ses_id, __u32  tid)
{
	struct cifs_ses *ses;
	struct cifs_tcon *tcon;

	spin_lock(&cifs_tcp_ses_lock);
	ses = smb2_find_smb_ses_unlocked(server, ses_id);
	if (!ses) {
		spin_unlock(&cifs_tcp_ses_lock);
		return NULL;
	}
	tcon = smb2_find_smb_sess_tcon_unlocked(ses, tid);
	if (!tcon) {
		spin_unlock(&cifs_tcp_ses_lock);
		cifs_put_smb_ses(ses);
		return NULL;
	}
	spin_unlock(&cifs_tcp_ses_lock);
	/* tcon already has a ref to ses, so we don't need ses anymore */
	cifs_put_smb_ses(ses);

	return tcon;
}

int
smb2_calc_signature(struct smb_rqst *rqst, struct TCP_Server_Info *server,
			bool allocate_crypto)
{
	int rc;
	unsigned char smb2_signature[SMB2_HMACSHA256_SIZE];
	unsigned char *sigptr = smb2_signature;
	struct kvec *iov = rqst->rq_iov;
	struct smb2_hdr *shdr = (struct smb2_hdr *)iov[0].iov_base;
	struct shash_desc *shash = NULL;
	struct smb_rqst drqst;
	__u64 sid = le64_to_cpu(shdr->SessionId);
	u8 key[SMB2_NTLMV2_SESSKEY_SIZE];

	rc = smb2_get_sign_key(server, sid, key);
	if (unlikely(rc)) {
		cifs_server_dbg(FYI, "%s: [sesid=0x%llx] couldn't find signing key: %d\n",
				__func__, sid, rc);
		return rc;
	}

	memset(smb2_signature, 0x0, SMB2_HMACSHA256_SIZE);
	memset(shdr->Signature, 0x0, SMB2_SIGNATURE_SIZE);

	if (allocate_crypto) {
		rc = cifs_alloc_hash("hmac(sha256)", &shash);
		if (rc) {
			cifs_server_dbg(VFS,
					"%s: sha256 alloc failed\n", __func__);
			goto out;
		}
	} else {
		shash = server->secmech.hmacsha256;
	}

	rc = crypto_shash_setkey(shash->tfm, key, sizeof(key));
	if (rc) {
		cifs_server_dbg(VFS,
				"%s: Could not update with response\n",
				__func__);
		goto out;
	}

	rc = crypto_shash_init(shash);
	if (rc) {
		cifs_server_dbg(VFS, "%s: Could not init sha256", __func__);
		goto out;
	}

	/*
	 * For SMB2+, __cifs_calc_signature() expects to sign only the actual
	 * data, that is, iov[0] should not contain a rfc1002 length.
	 *
	 * Sign the rfc1002 length prior to passing the data (iov[1-N]) down to
	 * __cifs_calc_signature().
	 */
	drqst = *rqst;
	if (drqst.rq_nvec >= 2 && iov[0].iov_len == 4) {
		rc = crypto_shash_update(shash, iov[0].iov_base,
					 iov[0].iov_len);
		if (rc) {
			cifs_server_dbg(VFS,
					"%s: Could not update with payload\n",
					__func__);
			goto out;
		}
		drqst.rq_iov++;
		drqst.rq_nvec--;
	}

	rc = __cifs_calc_signature(&drqst, server, sigptr, shash);
	if (!rc)
		memcpy(shdr->Signature, sigptr, SMB2_SIGNATURE_SIZE);

out:
	if (allocate_crypto)
		cifs_free_hash(&shash);
	return rc;
}

static int generate_key(struct cifs_ses *ses, struct kvec label,
			struct kvec context, __u8 *key, unsigned int key_size)
{
	unsigned char zero = 0x0;
	__u8 i[4] = {0, 0, 0, 1};
	__u8 L128[4] = {0, 0, 0, 128};
	__u8 L256[4] = {0, 0, 1, 0};
	int rc = 0;
	unsigned char prfhash[SMB2_HMACSHA256_SIZE];
	unsigned char *hashptr = prfhash;
	struct TCP_Server_Info *server = ses->server;

	memset(prfhash, 0x0, SMB2_HMACSHA256_SIZE);
	memset(key, 0x0, key_size);

	rc = smb3_crypto_shash_allocate(server);
	if (rc) {
		cifs_server_dbg(VFS, "%s: crypto alloc failed\n", __func__);
		goto smb3signkey_ret;
	}

	rc = crypto_shash_setkey(server->secmech.hmacsha256->tfm,
		ses->auth_key.response, SMB2_NTLMV2_SESSKEY_SIZE);
	if (rc) {
		cifs_server_dbg(VFS, "%s: Could not set with session key\n", __func__);
		goto smb3signkey_ret;
	}

	rc = crypto_shash_init(server->secmech.hmacsha256);
	if (rc) {
		cifs_server_dbg(VFS, "%s: Could not init sign hmac\n", __func__);
		goto smb3signkey_ret;
	}

	rc = crypto_shash_update(server->secmech.hmacsha256, i, 4);
	if (rc) {
		cifs_server_dbg(VFS, "%s: Could not update with n\n", __func__);
		goto smb3signkey_ret;
	}

	rc = crypto_shash_update(server->secmech.hmacsha256, label.iov_base, label.iov_len);
	if (rc) {
		cifs_server_dbg(VFS, "%s: Could not update with label\n", __func__);
		goto smb3signkey_ret;
	}

	rc = crypto_shash_update(server->secmech.hmacsha256, &zero, 1);
	if (rc) {
		cifs_server_dbg(VFS, "%s: Could not update with zero\n", __func__);
		goto smb3signkey_ret;
	}

	rc = crypto_shash_update(server->secmech.hmacsha256, context.iov_base, context.iov_len);
	if (rc) {
		cifs_server_dbg(VFS, "%s: Could not update with context\n", __func__);
		goto smb3signkey_ret;
	}

	if ((server->cipher_type == SMB2_ENCRYPTION_AES256_CCM) ||
		(server->cipher_type == SMB2_ENCRYPTION_AES256_GCM)) {
		rc = crypto_shash_update(server->secmech.hmacsha256, L256, 4);
	} else {
		rc = crypto_shash_update(server->secmech.hmacsha256, L128, 4);
	}
	if (rc) {
		cifs_server_dbg(VFS, "%s: Could not update with L\n", __func__);
		goto smb3signkey_ret;
	}

	rc = crypto_shash_final(server->secmech.hmacsha256, hashptr);
	if (rc) {
		cifs_server_dbg(VFS, "%s: Could not generate sha256 hash\n", __func__);
		goto smb3signkey_ret;
	}

	memcpy(key, hashptr, key_size);

smb3signkey_ret:
	return rc;
}

struct derivation {
	struct kvec label;
	struct kvec context;
};

struct derivation_triplet {
	struct derivation signing;
	struct derivation encryption;
	struct derivation decryption;
};

static int
generate_smb3signingkey(struct cifs_ses *ses,
			struct TCP_Server_Info *server,
			const struct derivation_triplet *ptriplet)
{
	int rc;
	bool is_binding = false;
	int chan_index = 0;

	spin_lock(&ses->ses_lock);
	spin_lock(&ses->chan_lock);
	is_binding = (cifs_chan_needs_reconnect(ses, server) &&
		      ses->ses_status == SES_GOOD);

	chan_index = cifs_ses_get_chan_index(ses, server);
	if (chan_index == CIFS_INVAL_CHAN_INDEX) {
		spin_unlock(&ses->chan_lock);
		spin_unlock(&ses->ses_lock);

		return -EINVAL;
	}

	spin_unlock(&ses->chan_lock);
	spin_unlock(&ses->ses_lock);

	/*
	 * All channels use the same encryption/decryption keys but
	 * they have their own signing key.
	 *
	 * When we generate the keys, check if it is for a new channel
	 * (binding) in which case we only need to generate a signing
	 * key and store it in the channel as to not overwrite the
	 * master connection signing key stored in the session
	 */

	if (is_binding) {
		rc = generate_key(ses, ptriplet->signing.label,
				  ptriplet->signing.context,
				  ses->chans[chan_index].signkey,
				  SMB3_SIGN_KEY_SIZE);
		if (rc)
			return rc;
	} else {
		rc = generate_key(ses, ptriplet->signing.label,
				  ptriplet->signing.context,
				  ses->smb3signingkey,
				  SMB3_SIGN_KEY_SIZE);
		if (rc)
			return rc;

		/* safe to access primary channel, since it will never go away */
		spin_lock(&ses->chan_lock);
		memcpy(ses->chans[chan_index].signkey, ses->smb3signingkey,
		       SMB3_SIGN_KEY_SIZE);
		spin_unlock(&ses->chan_lock);

		rc = generate_key(ses, ptriplet->encryption.label,
				  ptriplet->encryption.context,
				  ses->smb3encryptionkey,
				  SMB3_ENC_DEC_KEY_SIZE);
		if (rc)
			return rc;
		rc = generate_key(ses, ptriplet->decryption.label,
				  ptriplet->decryption.context,
				  ses->smb3decryptionkey,
				  SMB3_ENC_DEC_KEY_SIZE);
		if (rc)
			return rc;
	}

#ifdef CONFIG_CIFS_DEBUG_DUMP_KEYS
	cifs_dbg(VFS, "%s: dumping generated AES session keys\n", __func__);
	/*
	 * The session id is opaque in terms of endianness, so we can't
	 * print it as a long long. we dump it as we got it on the wire
	 */
	cifs_dbg(VFS, "Session Id    %*ph\n", (int)sizeof(ses->Suid),
			&ses->Suid);
	cifs_dbg(VFS, "Cipher type   %d\n", server->cipher_type);
	cifs_dbg(VFS, "Session Key   %*ph\n",
		 SMB2_NTLMV2_SESSKEY_SIZE, ses->auth_key.response);
	cifs_dbg(VFS, "Signing Key   %*ph\n",
		 SMB3_SIGN_KEY_SIZE, ses->smb3signingkey);
	if ((server->cipher_type == SMB2_ENCRYPTION_AES256_CCM) ||
		(server->cipher_type == SMB2_ENCRYPTION_AES256_GCM)) {
		cifs_dbg(VFS, "ServerIn Key  %*ph\n",
				SMB3_GCM256_CRYPTKEY_SIZE, ses->smb3encryptionkey);
		cifs_dbg(VFS, "ServerOut Key %*ph\n",
				SMB3_GCM256_CRYPTKEY_SIZE, ses->smb3decryptionkey);
	} else {
		cifs_dbg(VFS, "ServerIn Key  %*ph\n",
				SMB3_GCM128_CRYPTKEY_SIZE, ses->smb3encryptionkey);
		cifs_dbg(VFS, "ServerOut Key %*ph\n",
				SMB3_GCM128_CRYPTKEY_SIZE, ses->smb3decryptionkey);
	}
#endif
	return rc;
}

int
generate_smb30signingkey(struct cifs_ses *ses,
			 struct TCP_Server_Info *server)

{
	struct derivation_triplet triplet;
	struct derivation *d;

	d = &triplet.signing;
	d->label.iov_base = "SMB2AESCMAC";
	d->label.iov_len = 12;
	d->context.iov_base = "SmbSign";
	d->context.iov_len = 8;

	d = &triplet.encryption;
	d->label.iov_base = "SMB2AESCCM";
	d->label.iov_len = 11;
	d->context.iov_base = "ServerIn ";
	d->context.iov_len = 10;

	d = &triplet.decryption;
	d->label.iov_base = "SMB2AESCCM";
	d->label.iov_len = 11;
	d->context.iov_base = "ServerOut";
	d->context.iov_len = 10;

	return generate_smb3signingkey(ses, server, &triplet);
}

int
generate_smb311signingkey(struct cifs_ses *ses,
			  struct TCP_Server_Info *server)

{
	struct derivation_triplet triplet;
	struct derivation *d;

	d = &triplet.signing;
	d->label.iov_base = "SMBSigningKey";
	d->label.iov_len = 14;
	d->context.iov_base = ses->preauth_sha_hash;
	d->context.iov_len = 64;

	d = &triplet.encryption;
	d->label.iov_base = "SMBC2SCipherKey";
	d->label.iov_len = 16;
	d->context.iov_base = ses->preauth_sha_hash;
	d->context.iov_len = 64;

	d = &triplet.decryption;
	d->label.iov_base = "SMBS2CCipherKey";
	d->label.iov_len = 16;
	d->context.iov_base = ses->preauth_sha_hash;
	d->context.iov_len = 64;

	return generate_smb3signingkey(ses, server, &triplet);
}

int
smb3_calc_signature(struct smb_rqst *rqst, struct TCP_Server_Info *server,
			bool allocate_crypto)
{
	int rc;
	unsigned char smb3_signature[SMB2_CMACAES_SIZE];
	unsigned char *sigptr = smb3_signature;
	struct kvec *iov = rqst->rq_iov;
	struct smb2_hdr *shdr = (struct smb2_hdr *)iov[0].iov_base;
	struct shash_desc *shash = NULL;
	struct smb_rqst drqst;
	u8 key[SMB3_SIGN_KEY_SIZE];

	rc = smb3_get_sign_key(le64_to_cpu(shdr->SessionId), server, key);
	if (unlikely(rc)) {
		cifs_server_dbg(FYI, "%s: Could not get signing key\n", __func__);
		return rc;
	}

	if (allocate_crypto) {
		rc = cifs_alloc_hash("cmac(aes)", &shash);
		if (rc)
			return rc;
	} else {
		shash = server->secmech.aes_cmac;
	}

	memset(smb3_signature, 0x0, SMB2_CMACAES_SIZE);
	memset(shdr->Signature, 0x0, SMB2_SIGNATURE_SIZE);

	rc = crypto_shash_setkey(shash->tfm, key, SMB2_CMACAES_SIZE);
	if (rc) {
		cifs_server_dbg(VFS, "%s: Could not set key for cmac aes\n", __func__);
		goto out;
	}

	/*
	 * we already allocate aes_cmac when we init smb3 signing key,
	 * so unlike smb2 case we do not have to check here if secmech are
	 * initialized
	 */
	rc = crypto_shash_init(shash);
	if (rc) {
		cifs_server_dbg(VFS, "%s: Could not init cmac aes\n", __func__);
		goto out;
	}

	/*
	 * For SMB2+, __cifs_calc_signature() expects to sign only the actual
	 * data, that is, iov[0] should not contain a rfc1002 length.
	 *
	 * Sign the rfc1002 length prior to passing the data (iov[1-N]) down to
	 * __cifs_calc_signature().
	 */
	drqst = *rqst;
	if (drqst.rq_nvec >= 2 && iov[0].iov_len == 4) {
		rc = crypto_shash_update(shash, iov[0].iov_base,
					 iov[0].iov_len);
		if (rc) {
			cifs_server_dbg(VFS, "%s: Could not update with payload\n",
				 __func__);
			goto out;
		}
		drqst.rq_iov++;
		drqst.rq_nvec--;
	}

	rc = __cifs_calc_signature(&drqst, server, sigptr, shash);
	if (!rc)
		memcpy(shdr->Signature, sigptr, SMB2_SIGNATURE_SIZE);

out:
	if (allocate_crypto)
		cifs_free_hash(&shash);
	return rc;
}

/* must be called with server->srv_mutex held */
static int
smb2_sign_rqst(struct smb_rqst *rqst, struct TCP_Server_Info *server)
{
	int rc = 0;
	struct smb2_hdr *shdr;
	struct smb2_sess_setup_req *ssr;
	bool is_binding;
	bool is_signed;

	shdr = (struct smb2_hdr *)rqst->rq_iov[0].iov_base;
	ssr = (struct smb2_sess_setup_req *)shdr;

	is_binding = shdr->Command == SMB2_SESSION_SETUP &&
		(ssr->Flags & SMB2_SESSION_REQ_FLAG_BINDING);
	is_signed = shdr->Flags & SMB2_FLAGS_SIGNED;

	if (!is_signed)
		return 0;
	spin_lock(&server->srv_lock);
	if (server->ops->need_neg &&
	    server->ops->need_neg(server)) {
		spin_unlock(&server->srv_lock);
		return 0;
	}
	spin_unlock(&server->srv_lock);
	if (!is_binding && !server->session_estab) {
		strncpy(shdr->Signature, "BSRSPYL", 8);
		return 0;
	}

	rc = server->ops->calc_signature(rqst, server, false);

	return rc;
}

int
smb2_verify_signature(struct smb_rqst *rqst, struct TCP_Server_Info *server)
{
	unsigned int rc;
	char server_response_sig[SMB2_SIGNATURE_SIZE];
	struct smb2_hdr *shdr =
			(struct smb2_hdr *)rqst->rq_iov[0].iov_base;

	if ((shdr->Command == SMB2_NEGOTIATE) ||
	    (shdr->Command == SMB2_SESSION_SETUP) ||
	    (shdr->Command == SMB2_OPLOCK_BREAK) ||
	    server->ignore_signature ||
	    (!server->session_estab))
		return 0;

	/*
	 * BB what if signatures are supposed to be on for session but
	 * server does not send one? BB
	 */

	/* Do not need to verify session setups with signature "BSRSPYL " */
	if (memcmp(shdr->Signature, "BSRSPYL ", 8) == 0)
		cifs_dbg(FYI, "dummy signature received for smb command 0x%x\n",
			 shdr->Command);

	/*
	 * Save off the origiginal signature so we can modify the smb and check
	 * our calculated signature against what the server sent.
	 */
	memcpy(server_response_sig, shdr->Signature, SMB2_SIGNATURE_SIZE);

	memset(shdr->Signature, 0, SMB2_SIGNATURE_SIZE);

	rc = server->ops->calc_signature(rqst, server, true);

	if (rc)
		return rc;

	if (memcmp(server_response_sig, shdr->Signature, SMB2_SIGNATURE_SIZE)) {
		cifs_dbg(VFS, "sign fail cmd 0x%x message id 0x%llx\n",
			shdr->Command, shdr->MessageId);
		return -EACCES;
	} else
		return 0;
}

/*
 * Set message id for the request. Should be called after wait_for_free_request
 * and when srv_mutex is held.
 */
static inline void
smb2_seq_num_into_buf(struct TCP_Server_Info *server,
		      struct smb2_hdr *shdr)
{
	unsigned int i, num = le16_to_cpu(shdr->CreditCharge);

	shdr->MessageId = get_next_mid64(server);
	/* skip message numbers according to CreditCharge field */
	for (i = 1; i < num; i++)
		get_next_mid(server);
}

static struct mid_q_entry *
smb2_mid_entry_alloc(const struct smb2_hdr *shdr,
		     struct TCP_Server_Info *server)
{
	struct mid_q_entry *temp;
	unsigned int credits = le16_to_cpu(shdr->CreditCharge);

	if (server == NULL) {
		cifs_dbg(VFS, "Null TCP session in smb2_mid_entry_alloc\n");
		return NULL;
	}

	temp = mempool_alloc(cifs_mid_poolp, GFP_NOFS);
	memset(temp, 0, sizeof(struct mid_q_entry));
	kref_init(&temp->refcount);
	temp->mid = le64_to_cpu(shdr->MessageId);
	temp->credits = credits > 0 ? credits : 1;
	temp->pid = current->pid;
	temp->command = shdr->Command; /* Always LE */
	temp->when_alloc = jiffies;
	temp->server = server;

	/*
	 * The default is for the mid to be synchronous, so the
	 * default callback just wakes up the current task.
	 */
	get_task_struct(current);
	temp->creator = current;
	temp->callback = cifs_wake_up_task;
	temp->callback_data = current;

	atomic_inc(&mid_count);
	temp->mid_state = MID_REQUEST_ALLOCATED;
	trace_smb3_cmd_enter(le32_to_cpu(shdr->Id.SyncId.TreeId),
			     le64_to_cpu(shdr->SessionId),
			     le16_to_cpu(shdr->Command), temp->mid);
	return temp;
}

static int
smb2_get_mid_entry(struct cifs_ses *ses, struct TCP_Server_Info *server,
		   struct smb2_hdr *shdr, struct mid_q_entry **mid)
{
	spin_lock(&server->srv_lock);
	if (server->tcpStatus == CifsExiting) {
		spin_unlock(&server->srv_lock);
		return -ENOENT;
	}

	if (server->tcpStatus == CifsNeedReconnect) {
		spin_unlock(&server->srv_lock);
		cifs_dbg(FYI, "tcp session dead - return to caller to retry\n");
		return -EAGAIN;
	}

	if (server->tcpStatus == CifsNeedNegotiate &&
	   shdr->Command != SMB2_NEGOTIATE) {
		spin_unlock(&server->srv_lock);
		return -EAGAIN;
	}
	spin_unlock(&server->srv_lock);

	spin_lock(&ses->ses_lock);
	if (ses->ses_status == SES_NEW) {
		if ((shdr->Command != SMB2_SESSION_SETUP) &&
		    (shdr->Command != SMB2_NEGOTIATE)) {
			spin_unlock(&ses->ses_lock);
			return -EAGAIN;
		}
		/* else ok - we are setting up session */
	}

	if (ses->ses_status == SES_EXITING) {
		if (shdr->Command != SMB2_LOGOFF) {
			spin_unlock(&ses->ses_lock);
			return -EAGAIN;
		}
		/* else ok - we are shutting down the session */
	}
	spin_unlock(&ses->ses_lock);

	*mid = smb2_mid_entry_alloc(shdr, server);
	if (*mid == NULL)
		return -ENOMEM;
	spin_lock(&server->mid_lock);
	list_add_tail(&(*mid)->qhead, &server->pending_mid_q);
	spin_unlock(&server->mid_lock);

	return 0;
}

int
smb2_check_receive(struct mid_q_entry *mid, struct TCP_Server_Info *server,
		   bool log_error)
{
	unsigned int len = mid->resp_buf_size;
	struct kvec iov[1];
	struct smb_rqst rqst = { .rq_iov = iov,
				 .rq_nvec = 1 };

	iov[0].iov_base = (char *)mid->resp_buf;
	iov[0].iov_len = len;

	dump_smb(mid->resp_buf, min_t(u32, 80, len));
	/* convert the length into a more usable form */
	if (len > 24 && server->sign && !mid->decrypted) {
		int rc;

		rc = smb2_verify_signature(&rqst, server);
		if (rc)
			cifs_server_dbg(VFS, "SMB signature verification returned error = %d\n",
				 rc);
	}

	return map_smb2_to_linux_error(mid->resp_buf, log_error);
}

struct mid_q_entry *
smb2_setup_request(struct cifs_ses *ses, struct TCP_Server_Info *server,
		   struct smb_rqst *rqst)
{
	int rc;
	struct smb2_hdr *shdr =
			(struct smb2_hdr *)rqst->rq_iov[0].iov_base;
	struct mid_q_entry *mid;

	smb2_seq_num_into_buf(server, shdr);

	rc = smb2_get_mid_entry(ses, server, shdr, &mid);
	if (rc) {
		revert_current_mid_from_hdr(server, shdr);
		return ERR_PTR(rc);
	}

	rc = smb2_sign_rqst(rqst, server);
	if (rc) {
		revert_current_mid_from_hdr(server, shdr);
		delete_mid(mid);
		return ERR_PTR(rc);
	}

	return mid;
}

struct mid_q_entry *
smb2_setup_async_request(struct TCP_Server_Info *server, struct smb_rqst *rqst)
{
	int rc;
	struct smb2_hdr *shdr =
			(struct smb2_hdr *)rqst->rq_iov[0].iov_base;
	struct mid_q_entry *mid;

	spin_lock(&server->srv_lock);
	if (server->tcpStatus == CifsNeedNegotiate &&
	   shdr->Command != SMB2_NEGOTIATE) {
		spin_unlock(&server->srv_lock);
		return ERR_PTR(-EAGAIN);
	}
	spin_unlock(&server->srv_lock);

	smb2_seq_num_into_buf(server, shdr);

	mid = smb2_mid_entry_alloc(shdr, server);
	if (mid == NULL) {
		revert_current_mid_from_hdr(server, shdr);
		return ERR_PTR(-ENOMEM);
	}

	rc = smb2_sign_rqst(rqst, server);
	if (rc) {
		revert_current_mid_from_hdr(server, shdr);
		release_mid(mid);
		return ERR_PTR(rc);
	}

	return mid;
}

int
smb3_crypto_aead_allocate(struct TCP_Server_Info *server)
{
	struct crypto_aead *tfm;

	if (!server->secmech.enc) {
		if ((server->cipher_type == SMB2_ENCRYPTION_AES128_GCM) ||
		    (server->cipher_type == SMB2_ENCRYPTION_AES256_GCM))
			tfm = crypto_alloc_aead("gcm(aes)", 0, 0);
		else
			tfm = crypto_alloc_aead("ccm(aes)", 0, 0);
		if (IS_ERR(tfm)) {
			cifs_server_dbg(VFS, "%s: Failed alloc encrypt aead\n",
				 __func__);
			return PTR_ERR(tfm);
		}
		server->secmech.enc = tfm;
	}

	if (!server->secmech.dec) {
		if ((server->cipher_type == SMB2_ENCRYPTION_AES128_GCM) ||
		    (server->cipher_type == SMB2_ENCRYPTION_AES256_GCM))
			tfm = crypto_alloc_aead("gcm(aes)", 0, 0);
		else
			tfm = crypto_alloc_aead("ccm(aes)", 0, 0);
		if (IS_ERR(tfm)) {
			crypto_free_aead(server->secmech.enc);
			server->secmech.enc = NULL;
			cifs_server_dbg(VFS, "%s: Failed to alloc decrypt aead\n",
				 __func__);
			return PTR_ERR(tfm);
		}
		server->secmech.dec = tfm;
	}

	return 0;
}

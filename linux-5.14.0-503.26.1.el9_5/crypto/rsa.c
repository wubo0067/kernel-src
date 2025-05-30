// SPDX-License-Identifier: GPL-2.0-or-later
/* RSA asymmetric public-key algorithm [RFC3447]
 *
 * Copyright (c) 2015, Intel Corporation
 * Authors: Tadeusz Struk <tadeusz.struk@intel.com>
 */

#include <linux/fips.h>
#include <linux/module.h>
#include <linux/mpi.h>
#include <crypto/internal/rsa.h>
#include <crypto/internal/akcipher.h>
#include <crypto/akcipher.h>
#include <crypto/algapi.h>

struct rsa_mpi_key {
	MPI n;
	MPI e;
	MPI d;
};

static int rsa_check_payload(MPI x, MPI n)
{
	MPI n1;

	if (mpi_cmp_ui(x, 1) <= 0)
		return -EINVAL;

	n1 = mpi_alloc(0);
	if (!n1)
		return -ENOMEM;

	if (mpi_sub_ui(n1, n, 1) || mpi_cmp(x, n1) >= 0) {
		mpi_free(n1);
		return -EINVAL;
	}

	mpi_free(n1);
	return 0;
}

/*
 * RSAEP function [RFC3447 sec 5.1.1]
 * c = m^e mod n;
 */
static int _rsa_enc(const struct rsa_mpi_key *key, MPI c, MPI m)
{
	/*
	 * Even though (1) in RFC3447 only requires 0 <= m <= n - 1, we are
	 * slightly more conservative and require 1 < m < n - 1. This is in line
	 * with SP 800-56Br2, Section 7.1.1.
	 */
	if (rsa_check_payload(m, key->n))
		return -EINVAL;

	/* (2) c = m^e mod n */
	return mpi_powm(c, m, key->e, key->n);
}

/*
 * RSADP function [RFC3447 sec 5.1.2]
 * m = c^d mod n;
 */
static int _rsa_dec(const struct rsa_mpi_key *key, MPI m, MPI c)
{
	/*
	 * Even though (1) in RFC3447 only requires 0 <= c <= n - 1, we are
	 * slightly more conservative and require 1 < c < n - 1. This is in line
	 * with SP 800-56Br2, Section 7.1.2.
	 */
	if (rsa_check_payload(c, key->n))
		return -EINVAL;

	/* (2) m = c^d mod n */
	return mpi_powm(m, c, key->d, key->n);
}

static inline struct rsa_mpi_key *rsa_get_key(struct crypto_akcipher *tfm)
{
	return akcipher_tfm_ctx(tfm);
}

static int rsa_enc(struct akcipher_request *req)
{
	struct crypto_akcipher *tfm = crypto_akcipher_reqtfm(req);
	const struct rsa_mpi_key *pkey = rsa_get_key(tfm);
	MPI m, c = mpi_alloc(0);
	int ret = 0;
	int sign;

	if (!c)
		return -ENOMEM;

	if (unlikely(!pkey->n || !pkey->e)) {
		ret = -EINVAL;
		goto err_free_c;
	}

	ret = -ENOMEM;
	m = mpi_read_raw_from_sgl(req->src, req->src_len);
	if (!m)
		goto err_free_c;

	ret = _rsa_enc(pkey, c, m);
	if (ret)
		goto err_free_m;

	ret = mpi_write_to_sgl(c, req->dst, req->dst_len, &sign);
	if (ret)
		goto err_free_m;

	if (sign < 0)
		ret = -EBADMSG;

err_free_m:
	mpi_free(m);
err_free_c:
	mpi_free(c);
	return ret;
}

static int rsa_dec(struct akcipher_request *req)
{
	struct crypto_akcipher *tfm = crypto_akcipher_reqtfm(req);
	const struct rsa_mpi_key *pkey = rsa_get_key(tfm);
	MPI c, m = mpi_alloc(0);
	int ret = 0;
	int sign;

	if (!m)
		return -ENOMEM;

	if (unlikely(!pkey->n || !pkey->d)) {
		ret = -EINVAL;
		goto err_free_m;
	}

	ret = -ENOMEM;
	c = mpi_read_raw_from_sgl(req->src, req->src_len);
	if (!c)
		goto err_free_m;

	ret = _rsa_dec(pkey, m, c);
	if (ret)
		goto err_free_c;

	ret = mpi_write_to_sgl(m, req->dst, req->dst_len, &sign);
	if (ret)
		goto err_free_c;

	if (sign < 0)
		ret = -EBADMSG;
err_free_c:
	mpi_free(c);
err_free_m:
	mpi_free(m);
	return ret;
}

static void rsa_free_mpi_key(struct rsa_mpi_key *key)
{
	mpi_free(key->d);
	mpi_free(key->e);
	mpi_free(key->n);
	key->d = NULL;
	key->e = NULL;
	key->n = NULL;
}

static int rsa_check_key_length(unsigned int len)
{
	switch (len) {
	case 512:
	case 1024:
	case 1536:
		if (fips_enabled)
			return -EINVAL;
		fallthrough;
	case 2048:
	case 3072:
	case 4096:
		return 0;
	}

	return -EINVAL;
}

static int rsa_check_exponent_fips(MPI e)
{
	MPI e_max = NULL;

	/* check if odd */
	if (!mpi_test_bit(e, 0)) {
		return -EINVAL;
	}

	/* check if 2^16 < e < 2^256. */
	if (mpi_cmp_ui(e, 65536) <= 0) {
		return -EINVAL;
	}

	e_max = mpi_alloc(0);
	if (!e_max)
		return -ENOMEM;
	mpi_set_bit(e_max, 256);

	if (mpi_cmp(e, e_max) >= 0) {
		mpi_free(e_max);
		return -EINVAL;
	}

	mpi_free(e_max);
	return 0;
}

static int rsa_set_pub_key(struct crypto_akcipher *tfm, const void *key,
			   unsigned int keylen)
{
	struct rsa_mpi_key *mpi_key = akcipher_tfm_ctx(tfm);
	struct rsa_key raw_key = {0};
	int ret;

	/* Free the old MPI key if any */
	rsa_free_mpi_key(mpi_key);

	ret = rsa_parse_pub_key(&raw_key, key, keylen);
	if (ret)
		return ret;

	mpi_key->e = mpi_read_raw_data(raw_key.e, raw_key.e_sz);
	if (!mpi_key->e)
		goto err;

	mpi_key->n = mpi_read_raw_data(raw_key.n, raw_key.n_sz);
	if (!mpi_key->n)
		goto err;

	if (rsa_check_key_length(mpi_get_size(mpi_key->n) << 3)) {
		rsa_free_mpi_key(mpi_key);
		return -EINVAL;
	}

	if (fips_enabled && rsa_check_exponent_fips(mpi_key->e)) {
		rsa_free_mpi_key(mpi_key);
		return -EINVAL;
	}

	return 0;

err:
	rsa_free_mpi_key(mpi_key);
	return -ENOMEM;
}

static int rsa_set_priv_key(struct crypto_akcipher *tfm, const void *key,
			    unsigned int keylen)
{
	struct rsa_mpi_key *mpi_key = akcipher_tfm_ctx(tfm);
	struct rsa_key raw_key = {0};
	int ret;

	/* Free the old MPI key if any */
	rsa_free_mpi_key(mpi_key);

	ret = rsa_parse_priv_key(&raw_key, key, keylen);
	if (ret)
		return ret;

	mpi_key->d = mpi_read_raw_data(raw_key.d, raw_key.d_sz);
	if (!mpi_key->d)
		goto err;

	mpi_key->e = mpi_read_raw_data(raw_key.e, raw_key.e_sz);
	if (!mpi_key->e)
		goto err;

	mpi_key->n = mpi_read_raw_data(raw_key.n, raw_key.n_sz);
	if (!mpi_key->n)
		goto err;

	if (rsa_check_key_length(mpi_get_size(mpi_key->n) << 3)) {
		rsa_free_mpi_key(mpi_key);
		return -EINVAL;
	}

	if (fips_enabled && rsa_check_exponent_fips(mpi_key->e)) {
		rsa_free_mpi_key(mpi_key);
		return -EINVAL;
	}

	return 0;

err:
	rsa_free_mpi_key(mpi_key);
	return -ENOMEM;
}

static unsigned int rsa_max_size(struct crypto_akcipher *tfm)
{
	struct rsa_mpi_key *pkey = akcipher_tfm_ctx(tfm);

	return mpi_get_size(pkey->n);
}

static void rsa_exit_tfm(struct crypto_akcipher *tfm)
{
	struct rsa_mpi_key *pkey = akcipher_tfm_ctx(tfm);

	rsa_free_mpi_key(pkey);
}

static struct akcipher_alg rsa = {
	.encrypt = rsa_enc,
	.decrypt = rsa_dec,
	.set_priv_key = rsa_set_priv_key,
	.set_pub_key = rsa_set_pub_key,
	.max_size = rsa_max_size,
	.exit = rsa_exit_tfm,
	.base = {
		.cra_name = "rsa",
		.cra_driver_name = "rsa-generic",
		.cra_priority = 100,
		.cra_module = THIS_MODULE,
		.cra_ctxsize = sizeof(struct rsa_mpi_key),
	},
};

static int __init rsa_init(void)
{
	int err;

	err = crypto_register_akcipher(&rsa);
	if (err)
		return err;

	err = crypto_register_template(&rsa_pkcs1pad_tmpl);
	if (err) {
		crypto_unregister_akcipher(&rsa);
		return err;
	}

	return 0;
}

static void __exit rsa_exit(void)
{
	crypto_unregister_template(&rsa_pkcs1pad_tmpl);
	crypto_unregister_akcipher(&rsa);
}

subsys_initcall(rsa_init);
module_exit(rsa_exit);
MODULE_ALIAS_CRYPTO("rsa");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RSA generic algorithm");

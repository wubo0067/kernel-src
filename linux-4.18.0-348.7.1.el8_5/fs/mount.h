/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/mount.h>
#include <linux/seq_file.h>
#include <linux/poll.h>
#include <linux/ns_common.h>
#include <linux/fs_pin.h>

struct mnt_namespace {
	atomic_t count;
	struct ns_common ns;
	struct mount *root;
	/*
	 * Traversal and modification of .list is protected by either
	 * - taking namespace_sem for write, OR
	 * - taking namespace_sem for read AND taking .ns_lock.
	 */
	struct list_head list;
	spinlock_t ns_lock;
	struct user_namespace *user_ns;
	struct ucounts *ucounts;
	u64 seq; /* Sequence number to prevent loops */
	wait_queue_head_t poll;
	u64 event;
	unsigned int mounts; /* # of mounts in the namespace */
	unsigned int pending_mounts;
} __randomize_layout;

struct mnt_pcp {
	int mnt_count;
	int mnt_writers;
};

struct mountpoint {
	struct hlist_node m_hash;
	struct dentry *m_dentry;
	struct hlist_head m_list;
	int m_count;
};

struct mount {
	struct hlist_node mnt_hash;
	struct mount *mnt_parent;
	struct dentry *mnt_mountpoint;
	struct vfsmount mnt;
	union {
		struct rcu_head mnt_rcu;
		struct llist_node mnt_llist;
	};
#ifdef CONFIG_SMP
	struct mnt_pcp __percpu *mnt_pcp;
#else
	int mnt_count;
	int mnt_writers;
#endif
	// 子挂载点怎么来的，
	// mount /dev/sda1 /home        // 创建 /home 子挂载点
	// mount /dev/sdb1 /home/data   // 创建 /home/data 子挂载点
	// 使用命令 findmnt -R /
	struct list_head mnt_mounts; /* list of children, anchored here */
	struct list_head mnt_child; /* and going through their mnt_child */
	struct list_head mnt_instance; /* mount instance on sb->s_mounts */
	const char *mnt_devname; /* Name of device e.g. /dev/dsk/hda1 */
	struct list_head mnt_list;
	struct list_head mnt_expire; /* link in fs-specific expiry list */

	/*
	共享挂载指的是多个不同的挂载点可以共享同一个文件系统
	容器化环境: 容器之间可以共享同一个数据卷，从而实现数据共享。
	NFS挂载: 多个主机可以同时挂载同一个NFS服务器上的文件系统。
	mnt_share 链表将所有共享同一个文件系统的挂载点连接起来，形成一个循环链表。
	共享属性管理: 通过这个链表，内核可以方便地跟踪和管理共享挂载的各种属性，比如传播属性（propagation）、共享类型（share type）等
	传播属性: 共享挂载的传播属性决定了对共享文件系统的修改如何传播到其他共享挂载点。常见的传播属性有：
	*/
	struct list_head mnt_share; /* circular list of shared mounts */
	struct list_head mnt_slave_list; /* list of slave mounts */
	struct list_head mnt_slave; /* slave list entry */
	struct mount *mnt_master; /* slave is on master->mnt_slave_list */
	struct mnt_namespace *mnt_ns; /* containing namespace */
	struct mountpoint *mnt_mp; /* where is it mounted */
	struct hlist_node mnt_mp_list; /* list mounts with the same mountpoint */
	struct list_head mnt_umounting; /* list entry for umount propagation */
#ifdef CONFIG_FSNOTIFY
	struct fsnotify_mark_connector __rcu *mnt_fsnotify_marks;
	__u32 mnt_fsnotify_mask;
#endif
	int mnt_id; /* mount identifier */
	int mnt_group_id; /* peer group identifier */
	int mnt_expiry_mark; /* true if marked for expiry */
	struct hlist_head mnt_pins;
	struct fs_pin mnt_umount;
	struct dentry *mnt_ex_mountpoint;
} __randomize_layout;

#define MNT_NS_INTERNAL ERR_PTR(-EINVAL) /* distinct from any mnt_namespace */

static inline struct mount *real_mount(struct vfsmount *mnt)
{
	return container_of(mnt, struct mount, mnt);
}

static inline int mnt_has_parent(struct mount *mnt)
{
	return mnt != mnt->mnt_parent;
}

static inline int is_mounted(struct vfsmount *mnt)
{
	/* neither detached nor internal? */
	return !IS_ERR_OR_NULL(real_mount(mnt)->mnt_ns);
}

extern struct mount *__lookup_mnt(struct vfsmount *, struct dentry *);

extern int __legitimize_mnt(struct vfsmount *, unsigned);
extern bool legitimize_mnt(struct vfsmount *, unsigned);

static inline bool __path_is_mountpoint(const struct path *path)
{
	struct mount *m = __lookup_mnt(path->mnt, path->dentry);
	return m && likely(!(m->mnt.mnt_flags & MNT_SYNC_UMOUNT));
}

extern void __detach_mounts(struct dentry *dentry);

static inline void detach_mounts(struct dentry *dentry)
{
	if (!d_mountpoint(dentry))
		return;
	__detach_mounts(dentry);
}

static inline void get_mnt_ns(struct mnt_namespace *ns)
{
	atomic_inc(&ns->count);
}

extern seqlock_t mount_lock;

static inline void lock_mount_hash(void)
{
	write_seqlock(&mount_lock);
}

static inline void unlock_mount_hash(void)
{
	write_sequnlock(&mount_lock);
}

struct proc_mounts {
	struct mnt_namespace *ns;
	struct path root;
	int (*show)(struct seq_file *, struct vfsmount *);
	struct mount cursor;
};

extern const struct seq_operations mounts_op;

extern bool __is_local_mountpoint(struct dentry *dentry);
static inline bool is_local_mountpoint(struct dentry *dentry)
{
	if (!d_mountpoint(dentry))
		return false;

	return __is_local_mountpoint(dentry);
}

static inline bool is_anon_ns(struct mnt_namespace *ns)
{
	return ns->seq == 0;
}

extern void mnt_cursor_del(struct mnt_namespace *ns, struct mount *cursor);

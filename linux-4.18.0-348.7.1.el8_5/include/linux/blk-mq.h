/* SPDX-License-Identifier: GPL-2.0 */
#ifndef BLK_MQ_H
#define BLK_MQ_H

#include <linux/blkdev.h>
#include <linux/sbitmap.h>
#include <linux/srcu.h>
#include <linux/lockdep.h>

struct blk_mq_tags;
struct blk_flush_queue;

/**
 * struct blk_mq_hw_ctx - State for a hardware queue facing the hardware block device
 */
struct blk_mq_hw_ctx {
	struct {
		spinlock_t lock;
		struct list_head
			dispatch; // 双向链表头节点，用于保存派发到硬件队列的所有 request；
		unsigned long state; /* BLK_MQ_S_* flags */
	} ____cacheline_aligned_in_smp;

	struct delayed_work run_work;
	cpumask_var_t cpumask;
	int next_cpu;
	int next_cpu_batch;

	unsigned long flags; /* BLK_MQ_F_* flags */

	void *sched_data;
	struct request_queue *queue;
	struct blk_flush_queue *fq;

	void *driver_data;

	struct sbitmap ctx_map;

	struct blk_mq_ctx *dispatch_from;
	unsigned int dispatch_busy;

	unsigned short type;
	unsigned short nr_ctx;
	struct blk_mq_ctx **ctxs;

	spinlock_t dispatch_wait_lock;
	wait_queue_entry_t dispatch_wait;
	atomic_t wait_index;

	struct blk_mq_tags *
		tags; // 这应该是 blk_mq_tag_set 中的一个，每一个硬件队列对应一个 blk_mq_tags
	struct blk_mq_tags *
		sched_tags; // 针对有调度的算法的硬件队列，用来保存硬队列对应的 blk_mq_tags
	// blk_mq_tags 主要是管理 struct request 的分配，blk_mq_tags 与硬件队列 blk_mq_hw_ctx 一一对应

	unsigned long queued;
	unsigned long run;
#define BLK_MQ_MAX_DISPATCH_ORDER 7
	unsigned long dispatched[BLK_MQ_MAX_DISPATCH_ORDER];

	unsigned int numa_node;
	unsigned int queue_num;

	atomic_t nr_active;
	RH_KABI_DEPRECATE(unsigned int, nr_expired)

	struct hlist_node cpuhp_dead;
	struct kobject kobj;

	unsigned long poll_considered;
	unsigned long poll_invoked;
	unsigned long poll_success;

#ifdef CONFIG_BLK_DEBUG_FS
	struct dentry *debugfs_dir;
	struct dentry *sched_debugfs_dir;
#endif

	RH_KABI_USE(1, 2, struct list_head hctx_list)

	/** @cpuhp_online: List to store request if CPU is going to die */
	RH_KABI_USE(3, 4, struct hlist_node cpuhp_online)

	RH_KABI_RESERVE(5)

	RH_KABI_RESERVE(6)
	RH_KABI_RESERVE(7)
	RH_KABI_RESERVE(8)

	/* Must be the last member - see also blk_mq_hw_ctx_size(). */
	struct srcu_struct srcu[0];
};

/**
 * struct blk_mq_queue_map - ctx -> hctx mapping
 * @mq_map:       CPU ID to hardware queue index map. This is an array
 *	with nr_cpu_ids elements. Each element has a value in the range
 *	[@queue_offset, @queue_offset + @nr_queues).
 * @nr_queues:    Number of hardware queues to map CPU IDs onto.
 * @queue_offset: First hardware queue to map onto. Used by the PCIe NVMe
 *	driver to map each hardware queue type (enum hctx_type) onto a distinct
 *	set of hardware queues.
 */
struct blk_mq_queue_map {
	unsigned int *mq_map;
	unsigned int nr_queues;
	unsigned int queue_offset;
};

enum hctx_type {
	HCTX_TYPE_DEFAULT, /* all I/O not otherwise accounted for */
	HCTX_TYPE_READ, /* just for READ I/O */
	HCTX_TYPE_POLL, /* polled I/O of any kind */

	HCTX_MAX_TYPES,
	RH_HCTX_MAX_TYPES = 6, /* RH extend for reserving space*/
};

struct blk_mq_tag_set_aux {
	struct sbitmap_queue __bitmap_tags;
	struct sbitmap_queue __breserved_tags;
};

/**
 * struct blk_mq_tag_set - tag set that can be shared between request queues
 * @map:	   One or more ctx -> hctx mappings. One map exists for each
 *		   hardware queue type (enum hctx_type) that the driver wishes
 *		   to support. There are no restrictions on maps being of the
 *		   same size, and it's perfectly legal to share maps between
 *		   types.
 * @nr_maps:	   Number of elements in the @map array. A number in the range
 *		   [1, HCTX_MAX_TYPES].
 * @ops:	   Pointers to functions that implement block driver behavior.
 * @nr_hw_queues:  Number of hardware queues supported by the block driver that
 *		   owns this data structure.
 * @queue_depth:   Number of tags per hardware queue, reserved tags included.
 * @reserved_tags: Number of tags to set aside for BLK_MQ_REQ_RESERVED tag
 *		   allocations.
 * @cmd_size:	   Number of additional bytes to allocate per request. The block
 *		   driver owns these additional bytes.
 * @numa_node:	   NUMA node the storage adapter has been connected to.
 * @timeout:	   Request processing timeout in jiffies.
 * @flags:	   Zero or more BLK_MQ_F_* flags.
 * @driver_data:   Pointer to data owned by the block driver that created this
 *		   tag set.
 * @__bitmap_tags: A shared tags sbitmap, used over all hctx's
 * @__breserved_tags:
 *		   A shared reserved tags sbitmap, used over all hctx's
 * @tags:	   Tag sets. One tag set per hardware queue. Has @nr_hw_queues
 *		   elements.
 * @tag_list_lock: Serializes tag_list accesses.
 * @tag_list:	   List of the request queues that use this tag set. See also
 *		   request_queue.tag_set_list.
 * blk_mq_alloc_tag_set 这是个全局对象
 */
struct blk_mq_tag_set {
	struct blk_mq_queue_map
		map[RH_HCTX_MAX_TYPES]; // 每个数组元素代表一种硬件队列类型，主要的硬件列类型包括三种，默认模式，读模式，轮询模式
	unsigned int nr_maps;
	const struct blk_mq_ops *
		ops; // 定义块设备驱动 mq 的操作集合，如果是 scsi 设备该变量会被初始化为 scsi_mq_ops/scsi_mq_ops_no_commit
	// struct request_queue 中也有这个对象
	unsigned int nr_hw_queues; // 硬件队列的数量
	unsigned int queue_depth; // 硬件队列的深度，包含预留的个数 reserved_tags
	unsigned int reserved_tags; // 每个硬件队列预留的元素个数
	unsigned int
		cmd_size; // 块设备驱动为每个 request 分配的额外的空间大小，一般用于存放设备驱动 payload 数据
	int numa_node; // 块设备连接的 NUMA（Non Uniform Memory Access Architecture）节点，分配 request 内存时使用，避免远程内存访问问题
	unsigned int
		timeout; // 请求处理的超时时间，单位是 jiffies，例如 ufs 默认是 30s
	unsigned int flags;
	void *driver_data;

	struct blk_mq_tags **
		tags; // 每个硬件队列都有一个 blk_mq_tags 结构体，一共具有 nr_hw_queues 个元素
	// blk_mq_realloc_tag_set_tags 这个函数分配的

	struct mutex tag_list_lock; // 互斥锁，用于同步访问 tag_list
	struct list_head tag_list;

	RH_KABI_USE(1, struct blk_mq_tag_set_aux *aux)
	RH_KABI_USE(2, atomic_t active_queues_shared_sbitmap)

	RH_KABI_RESERVE(3)
	RH_KABI_RESERVE(4)
	RH_KABI_RESERVE(5)
	RH_KABI_RESERVE(6)
	RH_KABI_RESERVE(7)
	RH_KABI_RESERVE(8)
};

struct blk_mq_queue_data {
	struct request *rq;
	bool last;

	RH_KABI_RESERVE(1)
};

typedef blk_status_t(queue_rq_fn)(struct blk_mq_hw_ctx *,
				  const struct blk_mq_queue_data *);
typedef void(commit_rqs_fn)(struct blk_mq_hw_ctx *);
#ifndef __GENKSYMS__
typedef int(get_budget_fn)(struct request_queue *);
typedef void(put_budget_fn)(struct request_queue *, int);
#else
typedef bool(get_budget_fn)(struct blk_mq_hw_ctx *);
typedef void(put_budget_fn)(struct blk_mq_hw_ctx *);
#endif
typedef enum blk_eh_timer_return(timeout_fn)(struct request *, bool);
typedef int(init_hctx_fn)(struct blk_mq_hw_ctx *, void *, unsigned int);
typedef void(exit_hctx_fn)(struct blk_mq_hw_ctx *, unsigned int);
typedef int(init_request_fn)(struct blk_mq_tag_set *set, struct request *,
			     unsigned int, unsigned int);
typedef void(exit_request_fn)(struct blk_mq_tag_set *set, struct request *,
			      unsigned int);

typedef bool(busy_iter_fn)(struct blk_mq_hw_ctx *, struct request *, void *,
			   bool);
typedef bool(busy_tag_iter_fn)(struct request *, void *, bool);
typedef int(poll_fn)(struct blk_mq_hw_ctx *);
typedef int(map_queues_fn)(struct blk_mq_tag_set *set);
typedef bool(busy_fn)(struct request_queue *);
typedef void(complete_fn)(struct request *);
typedef void(cleanup_rq_fn)(struct request *);

struct blk_mq_ops {
	/*
	 * Queue request
	 */
	queue_rq_fn *queue_rq;

	/*
	 * If a driver uses bd->last to judge when to submit requests to
	 * hardware, it must define this function. In case of errors that
	 * make us stop issuing further requests, this hook serves the
	 * purpose of kicking the hardware (which the last request otherwise
	 * would have done).
	 */
	commit_rqs_fn *commit_rqs;

	/*
	 * Reserve budget before queue request, once .queue_rq is
	 * run, it is driver's responsibility to release the
	 * reserved budget. Also we have to handle failure case
	 * of .get_budget for avoiding I/O deadlock.
	 */
	get_budget_fn *get_budget;
	put_budget_fn *put_budget;

	/*
	 * Called on request timeout
	 */
	timeout_fn *timeout;

	/*
	 * Called to poll for completion of a specific tag.
	 */
	poll_fn *poll;

	complete_fn *complete;

	/*
	 * Called when the block layer side of a hardware queue has been
	 * set up, allowing the driver to allocate/init matching structures.
	 * Ditto for exit/teardown.
	 */
	init_hctx_fn *init_hctx;
	exit_hctx_fn *exit_hctx;

	/*
	 * Called for every command allocated by the block layer to allow
	 * the driver to set up driver specific data.
	 *
	 * Tag greater than or equal to queue_depth is for setting up
	 * flush request.
	 *
	 * Ditto for exit/teardown.
	 */
	init_request_fn *init_request;
	exit_request_fn *exit_request;
	/* Called from inside blk_get_request() */
	void (*initialize_rq_fn)(struct request *rq);

	/*
	 * If set, returns whether or not this queue currently is busy
	 */
	busy_fn *busy;

	map_queues_fn *map_queues;

#ifdef CONFIG_BLK_DEBUG_FS
	/*
	 * Used by the debugfs implementation to show driver-specific
	 * information about a request.
	 */
	void (*show_rq)(struct seq_file *m, struct request *rq);
#endif

	/*
	 * Called before freeing one request which isn't completed yet,
	 * and usually for freeing the driver private data
	 */
	RH_KABI_USE(1, cleanup_rq_fn *cleanup_rq)

	/*
	 * Store rq's budget token
	 */
	RH_KABI_USE(2, void (*set_rq_budget_token)(struct request *, int))

	/*
	 * Retrieve rq's budget token
	 */
	RH_KABI_USE(3, int (*get_rq_budget_token)(struct request *))

	RH_KABI_RESERVE(4)
	RH_KABI_RESERVE(5)
	RH_KABI_RESERVE(6)
	RH_KABI_RESERVE(7)
	RH_KABI_RESERVE(8)
};

enum {
	BLK_MQ_F_SHOULD_MERGE = 1 << 0,
	BLK_MQ_F_TAG_SHARED = 1 << 1,
	BLK_MQ_F_TAG_QUEUE_SHARED = BLK_MQ_F_TAG_SHARED,
	BLK_MQ_F_SG_MERGE = 1 << 2, /* obsolete */
	/*
	 * Set when this device requires underlying blk-mq device for
	 * completing IO:
	 */
	BLK_MQ_F_STACKING = 1 << 3,
	BLK_MQ_F_TAG_HCTX_SHARED = 1 << 4,
	BLK_MQ_F_BLOCKING = 1 << 5,
	BLK_MQ_F_NO_SCHED = 1 << 6,
	BLK_MQ_F_ALLOC_POLICY_START_BIT = 8,
	BLK_MQ_F_ALLOC_POLICY_BITS = 1,

	BLK_MQ_S_STOPPED = 0,
	BLK_MQ_S_TAG_ACTIVE = 1,
	BLK_MQ_S_SCHED_RESTART = 2,

	/* hw queue is inactive after all its CPUs become offline */
	BLK_MQ_S_INACTIVE = 3,

	BLK_MQ_MAX_DEPTH = 10240,

	BLK_MQ_CPU_WORK_BATCH = 8,
};
#define BLK_MQ_FLAG_TO_ALLOC_POLICY(flags)                                     \
	((flags >> BLK_MQ_F_ALLOC_POLICY_START_BIT) &                          \
	 ((1 << BLK_MQ_F_ALLOC_POLICY_BITS) - 1))
#define BLK_ALLOC_POLICY_TO_MQ_FLAG(policy)                                    \
	((policy & ((1 << BLK_MQ_F_ALLOC_POLICY_BITS) - 1))                    \
	 << BLK_MQ_F_ALLOC_POLICY_START_BIT)

struct request_queue *blk_mq_init_queue(struct blk_mq_tag_set *);
struct request_queue *blk_mq_init_queue_data(struct blk_mq_tag_set *set,
					     void *queuedata);
struct request_queue *blk_mq_init_allocated_queue(struct blk_mq_tag_set *set,
						  struct request_queue *q,
						  bool elevator_init);
struct request_queue *blk_mq_init_sq_queue(struct blk_mq_tag_set *set,
					   const struct blk_mq_ops *ops,
					   unsigned int queue_depth,
					   unsigned int set_flags);
void blk_mq_unregister_dev(struct device *, struct request_queue *);

int blk_mq_alloc_tag_set(struct blk_mq_tag_set *set);
void blk_mq_free_tag_set(struct blk_mq_tag_set *set);

void blk_mq_flush_plug_list(struct blk_plug *plug, bool from_schedule);

void blk_mq_free_request(struct request *rq);

bool blk_mq_queue_inflight(struct request_queue *q);

enum {
	/* return when out of requests */
	BLK_MQ_REQ_NOWAIT = (__force blk_mq_req_flags_t)(1 << 0),
	/* allocate from reserved pool */
	BLK_MQ_REQ_RESERVED = (__force blk_mq_req_flags_t)(1 << 1),
	/* set RQF_PREEMPT */
	BLK_MQ_REQ_PREEMPT = (__force blk_mq_req_flags_t)(1 << 3),
};

struct request *blk_mq_alloc_request(struct request_queue *q, unsigned int op,
				     blk_mq_req_flags_t flags);
struct request *blk_mq_alloc_request_hctx(struct request_queue *q,
					  unsigned int op,
					  blk_mq_req_flags_t flags,
					  unsigned int hctx_idx);
struct request *blk_mq_tag_to_rq(struct blk_mq_tags *tags, unsigned int tag);

enum {
	BLK_MQ_UNIQUE_TAG_BITS = 16,
	BLK_MQ_UNIQUE_TAG_MASK = (1 << BLK_MQ_UNIQUE_TAG_BITS) - 1,
};

u32 blk_mq_unique_tag(struct request *rq);

static inline u16 blk_mq_unique_tag_to_hwq(u32 unique_tag)
{
	return unique_tag >> BLK_MQ_UNIQUE_TAG_BITS;
}

static inline u16 blk_mq_unique_tag_to_tag(u32 unique_tag)
{
	return unique_tag & BLK_MQ_UNIQUE_TAG_MASK;
}

/**
 * blk_mq_rq_state() - read the current MQ_RQ_* state of a request
 * @rq: target request.
 */
static inline enum mq_rq_state blk_mq_rq_state(struct request *rq)
{
	return READ_ONCE(rq->state);
}

static inline int blk_mq_request_started(struct request *rq)
{
	return blk_mq_rq_state(rq) != MQ_RQ_IDLE;
}

static inline int blk_mq_request_completed(struct request *rq)
{
	return blk_mq_rq_state(rq) == MQ_RQ_COMPLETE;
}

/*
 *
 * Set the state to complete when completing a request from inside ->queue_rq.
 * This is used by drivers that want to ensure special complete actions that
 * need access to the request are called on failure, e.g. by nvme for
 * multipathing.
 */
static inline void blk_mq_set_request_complete(struct request *rq)
{
	WRITE_ONCE(rq->state, MQ_RQ_COMPLETE);
}

void blk_mq_start_request(struct request *rq);
void blk_mq_end_request(struct request *rq, blk_status_t error);
void __blk_mq_end_request(struct request *rq, blk_status_t error);

void blk_mq_requeue_request(struct request *rq, bool kick_requeue_list);
void blk_mq_kick_requeue_list(struct request_queue *q);
void blk_mq_delay_kick_requeue_list(struct request_queue *q,
				    unsigned long msecs);
void blk_mq_complete_request(struct request *rq);
bool blk_mq_complete_request_remote(struct request *rq);
bool blk_mq_queue_stopped(struct request_queue *q);
void blk_mq_stop_hw_queue(struct blk_mq_hw_ctx *hctx);
void blk_mq_start_hw_queue(struct blk_mq_hw_ctx *hctx);
void blk_mq_stop_hw_queues(struct request_queue *q);
void blk_mq_start_hw_queues(struct request_queue *q);
void blk_mq_start_stopped_hw_queue(struct blk_mq_hw_ctx *hctx, bool async);
void blk_mq_start_stopped_hw_queues(struct request_queue *q, bool async);
void blk_mq_quiesce_queue(struct request_queue *q);
void blk_mq_unquiesce_queue(struct request_queue *q);
void blk_mq_delay_run_hw_queue(struct blk_mq_hw_ctx *hctx, unsigned long msecs);
void blk_mq_run_hw_queue(struct blk_mq_hw_ctx *hctx, bool async);
void blk_mq_run_hw_queues(struct request_queue *q, bool async);
void blk_mq_delay_run_hw_queues(struct request_queue *q, unsigned long msecs);
void blk_mq_tagset_busy_iter(struct blk_mq_tag_set *tagset,
			     busy_tag_iter_fn *fn, void *priv);
void blk_mq_tagset_wait_completed_request(struct blk_mq_tag_set *tagset);
void blk_mq_freeze_queue(struct request_queue *q);
void blk_mq_unfreeze_queue(struct request_queue *q);
void blk_freeze_queue_start(struct request_queue *q);
void blk_mq_freeze_queue_wait(struct request_queue *q);
int blk_mq_freeze_queue_wait_timeout(struct request_queue *q,
				     unsigned long timeout);

int blk_mq_map_queues(struct blk_mq_queue_map *qmap);
void blk_mq_update_nr_hw_queues(struct blk_mq_tag_set *set, int nr_hw_queues);

void blk_mq_quiesce_queue_nowait(struct request_queue *q);

unsigned int blk_mq_rq_cpu(struct request *rq);

bool __blk_should_fake_timeout(struct request_queue *q);
static inline bool blk_should_fake_timeout(struct request_queue *q)
{
	if (IS_ENABLED(CONFIG_FAIL_IO_TIMEOUT) &&
	    test_bit(QUEUE_FLAG_FAIL_IO, &q->queue_flags))
		return __blk_should_fake_timeout(q);
	return false;
}

/*
 * Driver command data is immediately after the request. So subtract request
 * size to get back to the original request, add request size to get the PDU.
 */
static inline struct request *blk_mq_rq_from_pdu(void *pdu)
{
	return pdu - sizeof(struct request);
}
static inline void *blk_mq_rq_to_pdu(struct request *rq)
{
	return rq + 1;
}

#define queue_for_each_hw_ctx(q, hctx, i)                                      \
	for ((i) = 0; (i) < (q)->nr_hw_queues && ({                            \
			      hctx = (q)->queue_hw_ctx[i];                     \
			      1;                                               \
		      });                                                      \
	     (i)++)

#define hctx_for_each_ctx(hctx, ctx, i)                                        \
	for ((i) = 0; (i) < (hctx)->nr_ctx && ({                               \
			      ctx = (hctx)->ctxs[(i)];                         \
			      1;                                               \
		      });                                                      \
	     (i)++)

static inline blk_qc_t request_to_qc_t(struct blk_mq_hw_ctx *hctx,
				       struct request *rq)
{
	if (rq->tag != -1)
		return rq->tag | (hctx->queue_num << BLK_QC_T_SHIFT);

	return rq->internal_tag | (hctx->queue_num << BLK_QC_T_SHIFT) |
	       BLK_QC_T_INTERNAL;
}

static inline void blk_mq_cleanup_rq(struct request *rq)
{
	if (rq->q->mq_ops->cleanup_rq)
		rq->q->mq_ops->cleanup_rq(rq);
}

blk_qc_t blk_mq_make_request(struct request_queue *q, struct bio *bio);
void blk_mq_hctx_set_fq_lock_class(struct blk_mq_hw_ctx *hctx,
				   struct lock_class_key *key);

#endif

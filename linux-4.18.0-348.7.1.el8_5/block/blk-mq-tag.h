/* SPDX-License-Identifier: GPL-2.0 */
#ifndef INT_BLK_MQ_TAG_H
#define INT_BLK_MQ_TAG_H

/*
 * Tag address space map. 管理struct request的分配，与硬件队列blk_mq_hw_ctx对应。
 */
struct blk_mq_tags {
	unsigned int
		nr_tags; // 每个硬件队列的深度（包含预留的个数 reserved_tags）；
		// 这个硬件队列最多包含 nr_tags 个 request，这些 request 是通过 blk_mq_alloc_rq() 分配的；保存在 static_rqs 数组中
	unsigned int nr_reserved_tags; // 每个硬件队列预留的 tag 个数，static_rqs[0~ (nr_reserved_tags-1]] 这 nr_reserved_tags 个 request

	atomic_t active_queues;

	struct sbitmap_queue RH_KABI_RENAME(bitmap_tags, __bitmap_tags);
	struct sbitmap_queue RH_KABI_RENAME(breserved_tags, __breserved_tags);

	struct request **rqs; // struct request *类型数组，数组长度为 nr_tags；
	struct request **
		static_rqs; // struct request *类型数组，数组长度为 nr_tags；数组元素在 blk_mq_alloc_rqs() 中根据硬队列深度真实分配了队列的 request；
	struct list_head page_list; // 用于链接分配出的 page

	// tag 的位图；每个 bit 代表一个 tag 标记，用于标示硬件队列中的 request；1 位已分配，0 为为分配；
	// bitmap_tags 管理 static_rqs[nr_reserved_tags ~ nr_tags] 这 nr_tags- nr_reserved_tags 个 request；
	RH_KABI_EXTEND(struct sbitmap_queue *bitmap_tags)
	// 保留 tag 的位图，每个 bit 代表一个 tag 标记，用于标示硬件队列中的 request；1 位已分配，0 为为分配；
	// breserved_tags 管理 static_rqs[0~ (nr_reserved_tags-1]] 这 nr_reserved_tags 个 request
	RH_KABI_EXTEND(struct sbitmap_queue *breserved_tags)

	/*
	 * used to clear request reference in rqs[] before freeing one
	 * request pool
	 */
	RH_KABI_EXTEND(spinlock_t lock)
};

extern struct blk_mq_tags *blk_mq_init_tags(unsigned int nr_tags,
					    unsigned int reserved_tags,
					    int node, unsigned int flags);
extern void blk_mq_free_tags(struct blk_mq_tags *tags, unsigned int flags);
extern int blk_mq_init_bitmaps(struct sbitmap_queue *bitmap_tags,
			       struct sbitmap_queue *breserved_tags,
			       unsigned int queue_depth, unsigned int reserved,
			       int node, int alloc_policy);

extern int blk_mq_init_shared_sbitmap(struct blk_mq_tag_set *set);
extern void blk_mq_exit_shared_sbitmap(struct blk_mq_tag_set *set);
extern unsigned int blk_mq_get_tag(struct blk_mq_alloc_data *data);
extern void blk_mq_put_tag(struct blk_mq_tags *tags, struct blk_mq_ctx *ctx,
			   unsigned int tag);
extern int blk_mq_tag_update_depth(struct blk_mq_hw_ctx *hctx,
				   struct blk_mq_tags **tags,
				   unsigned int depth, bool can_grow);
extern void blk_mq_tag_resize_shared_sbitmap(struct blk_mq_tag_set *set,
					     unsigned int size);

extern void blk_mq_tag_wakeup_all(struct blk_mq_tags *tags, bool);
void blk_mq_queue_tag_busy_iter(struct request_queue *q, busy_iter_fn *fn,
				void *priv);
void blk_mq_all_tag_iter(struct blk_mq_tags *tags, busy_tag_iter_fn *fn,
			 void *priv);

static inline struct sbq_wait_state *bt_wait_ptr(struct sbitmap_queue *bt,
						 struct blk_mq_hw_ctx *hctx)
{
	if (!hctx)
		return &bt->ws[0];
	return sbq_wait_ptr(bt, &hctx->wait_index);
}

enum {
	BLK_MQ_NO_TAG = -1U,
	BLK_MQ_TAG_MIN = 1,
	BLK_MQ_TAG_MAX = BLK_MQ_NO_TAG - 1,
};

extern bool __blk_mq_tag_busy(struct blk_mq_hw_ctx *);
extern void __blk_mq_tag_idle(struct blk_mq_hw_ctx *);

static inline bool blk_mq_tag_busy(struct blk_mq_hw_ctx *hctx)
{
	if (!(hctx->flags & BLK_MQ_F_TAG_QUEUE_SHARED))
		return false;

	return __blk_mq_tag_busy(hctx);
}

static inline void blk_mq_tag_idle(struct blk_mq_hw_ctx *hctx)
{
	if (!(hctx->flags & BLK_MQ_F_TAG_QUEUE_SHARED))
		return;

	__blk_mq_tag_idle(hctx);
}

static inline bool blk_mq_tag_is_reserved(struct blk_mq_tags *tags,
					  unsigned int tag)
{
	return tag < tags->nr_reserved_tags;
}

#endif

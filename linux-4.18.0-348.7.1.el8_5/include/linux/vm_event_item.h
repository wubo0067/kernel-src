/* SPDX-License-Identifier: GPL-2.0 */
#ifndef VM_EVENT_ITEM_H_INCLUDED
#define VM_EVENT_ITEM_H_INCLUDED

#ifdef CONFIG_ZONE_DMA
#define DMA_ZONE(xx) xx##_DMA,
#else
#define DMA_ZONE(xx)
#endif

#ifdef CONFIG_ZONE_DMA32
#define DMA32_ZONE(xx) xx##_DMA32,
#else
#define DMA32_ZONE(xx)
#endif

#ifdef CONFIG_HIGHMEM
#define HIGHMEM_ZONE(xx) xx##_HIGH,
#else
#define HIGHMEM_ZONE(xx)
#endif

#define FOR_ALL_ZONES(xx)                                                      \
	DMA_ZONE(xx) DMA32_ZONE(xx) xx##_NORMAL, HIGHMEM_ZONE(xx) xx##_MOVABLE

enum vm_event_item {
	PGPGIN,
	PGPGOUT,
	PSWPIN,
	PSWPOUT,
	FOR_ALL_ZONES(PGALLOC),
	FOR_ALL_ZONES(ALLOCSTALL),
	FOR_ALL_ZONES(PGSCAN_SKIP),
	PGFREE,
	PGACTIVATE,
	PGDEACTIVATE,
	PGLAZYFREE,
	PGFAULT,
	PGMAJFAULT,
	PGLAZYFREED,
	PGREFILL,
	PGSTEAL_KSWAPD,
	PGSTEAL_DIRECT,
	PGSCAN_KSWAPD,
	PGSCAN_DIRECT,
	PGSCAN_DIRECT_THROTTLE,
#ifndef __GENKSYMS__
	PGSCAN_ANON,
	PGSCAN_FILE,
	PGSTEAL_ANON,
	PGSTEAL_FILE,
#endif
#ifdef CONFIG_NUMA
	PGSCAN_ZONE_RECLAIM_FAILED,
#endif
	PGINODESTEAL,
	SLABS_SCANNED,
	KSWAPD_INODESTEAL,
	KSWAPD_LOW_WMARK_HIT_QUICKLY,
	KSWAPD_HIGH_WMARK_HIT_QUICKLY,
	PAGEOUTRUN,
	PGROTATED,
	DROP_PAGECACHE,
	DROP_SLAB,
	OOM_KILL,
#ifdef CONFIG_NUMA_BALANCING
	NUMA_PTE_UPDATES,
	NUMA_HUGE_PTE_UPDATES,
	NUMA_HINT_FAULTS, // 记录了系统中发生的 NUMA 暗示错误的总次数，每次进程访问远程 NUMA 节点上的页面时，都会触发一次暗示错误，并增加计数
	NUMA_HINT_FAULTS_LOCAL, // 记录发生在本地 NUMA 节点的暗示错误次数
	NUMA_PAGE_MIGRATE, // 记录因 NUMA 优化而迁移的页面数量
#endif
#ifdef CONFIG_MIGRATION
	PGMIGRATE_SUCCESS,
	PGMIGRATE_FAIL,
#endif
#ifdef CONFIG_COMPACTION
	COMPACTMIGRATE_SCANNED,
	COMPACTFREE_SCANNED,
	COMPACTISOLATED,
	COMPACTSTALL,
	COMPACTFAIL,
	COMPACTSUCCESS,
	KCOMPACTD_WAKE,
	KCOMPACTD_MIGRATE_SCANNED,
	KCOMPACTD_FREE_SCANNED,
#endif
#ifdef CONFIG_HUGETLB_PAGE
	HTLB_BUDDY_PGALLOC,
	HTLB_BUDDY_PGALLOC_FAIL,
#endif
	UNEVICTABLE_PGCULLED, /* culled to noreclaim list */
	UNEVICTABLE_PGSCANNED, /* scanned for reclaimability */
	UNEVICTABLE_PGRESCUED, /* rescued from noreclaim list */
	UNEVICTABLE_PGMLOCKED,
	UNEVICTABLE_PGMUNLOCKED,
	UNEVICTABLE_PGCLEARED, /* on COW, page truncate */
	UNEVICTABLE_PGSTRANDED, /* unable to isolate on unlock */
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	THP_FAULT_ALLOC,
	THP_FAULT_FALLBACK,
	RH_KABI_BROKEN_INSERT_ENUM(THP_FAULT_FALLBACK_CHARGE)
		THP_COLLAPSE_ALLOC,
	THP_COLLAPSE_ALLOC_FAILED,
	THP_FILE_ALLOC,
	RH_KABI_BROKEN_INSERT_ENUM(THP_FILE_FALLBACK)
		RH_KABI_BROKEN_INSERT_ENUM(THP_FILE_FALLBACK_CHARGE)
			THP_FILE_MAPPED,
	THP_SPLIT_PAGE,
	THP_SPLIT_PAGE_FAILED,
	THP_DEFERRED_SPLIT_PAGE,
	THP_SPLIT_PMD,
#ifdef CONFIG_HAVE_ARCH_TRANSPARENT_HUGEPAGE_PUD
	THP_SPLIT_PUD,
#endif
	THP_ZERO_PAGE_ALLOC,
	THP_ZERO_PAGE_ALLOC_FAILED,
	THP_SWPOUT,
	THP_SWPOUT_FALLBACK,
#endif
#ifdef CONFIG_MEMORY_BALLOON
	BALLOON_INFLATE,
	BALLOON_DEFLATE,
#ifdef CONFIG_BALLOON_COMPACTION
	BALLOON_MIGRATE,
#endif
#endif
#ifdef CONFIG_DEBUG_TLBFLUSH
	NR_TLB_REMOTE_FLUSH, /* cpu tried to flush others' tlbs */
	NR_TLB_REMOTE_FLUSH_RECEIVED, /* cpu received ipi for flush */
	NR_TLB_LOCAL_FLUSH_ALL,
	NR_TLB_LOCAL_FLUSH_ONE,
#endif /* CONFIG_DEBUG_TLBFLUSH */
#ifdef CONFIG_DEBUG_VM_VMACACHE
	VMACACHE_FIND_CALLS,
	VMACACHE_FIND_HITS,
#endif
#ifdef CONFIG_SWAP
	SWAP_RA,
	SWAP_RA_HIT,
#endif
	NR_VM_EVENT_ITEMS
};

#ifndef CONFIG_TRANSPARENT_HUGEPAGE
#define THP_FILE_ALLOC                                                         \
	({                                                                     \
		BUILD_BUG();                                                   \
		0;                                                             \
	})
#define THP_FILE_FALLBACK                                                      \
	({                                                                     \
		BUILD_BUG();                                                   \
		0;                                                             \
	})
#define THP_FILE_FALLBACK_CHARGE                                               \
	({                                                                     \
		BUILD_BUG();                                                   \
		0;                                                             \
	})
#define THP_FILE_MAPPED                                                        \
	({                                                                     \
		BUILD_BUG();                                                   \
		0;                                                             \
	})
#endif

#endif /* VM_EVENT_ITEM_H_INCLUDED */

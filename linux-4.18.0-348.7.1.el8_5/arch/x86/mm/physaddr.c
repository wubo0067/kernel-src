// SPDX-License-Identifier: GPL-2.0
#include <linux/memblock.h>
#include <linux/mmdebug.h>
#include <linux/export.h>
#include <linux/mm.h>

#include <asm/page.h>

#include "physaddr.h"

#ifdef CONFIG_X86_64

#ifdef CONFIG_DEBUG_VIRTUAL
unsigned long __phys_addr(unsigned long x)
{
	unsigned long y = x - __START_KERNEL_map;

	/* use the carry flag to determine if x was < __START_KERNEL_map */
	if (unlikely(x > y)) {
		x = y + phys_base;

		VIRTUAL_BUG_ON(y >= KERNEL_IMAGE_SIZE);
	} else {
		x = y + (__START_KERNEL_map - PAGE_OFFSET);

		/* carry flag will be set if starting x was >= PAGE_OFFSET */
		VIRTUAL_BUG_ON((x > y) || !phys_addr_valid(x));
	}

	return x;
}
EXPORT_SYMBOL(__phys_addr);

unsigned long __phys_addr_symbol(unsigned long x)
{
	unsigned long y = x - __START_KERNEL_map;

	/* only check upper bounds since lower bounds will trigger carry */
	VIRTUAL_BUG_ON(y >= KERNEL_IMAGE_SIZE);

	return y + phys_base;
}
EXPORT_SYMBOL(__phys_addr_symbol);
#endif

/*
	检查一个给定的虚拟地址 x 是否映射到了一个有效的物理内存页
	在Linux内核中，并非所有的虚拟地址都是有效的。有些地址可能尚未被映射，
	有些可能指向I/O空间，还有些可能在内核地址空间的“空洞”（gap）中。
	这个函数就是用来快速判断一个地址是否最终能对应到实际的RAM
*/
bool __virt_addr_valid(unsigned long x)
{
    // 将输入的虚拟地址 x 减去内核虚拟地址空间的起始地址 __START_KERNEL_map。
    // 这里的 y 实质上是地址 x 相对于内核起始地址的偏移量。
	// !!这个偏移值用来判断地址属于哪个内存区域，内核虚拟地址区域：内核代码映射区和直接映射区。
	unsigned long y = x - __START_KERNEL_map;

	/* use the carry flag to determine if x was < __START_KERNEL_map */
	// 利用无符号整数溢出的特性来判断 x 和 __START_KERNEL_map 的大小关系。
	// 如果 x < __START_KERNEL_map，那么 x - __START_KERNEL_map 会发生下溢（underflow），
	// 结果 y 将会是一个非常大的数，此时 y 会大于 x。
	// 这种情况对应于处理内核虚拟地址空间“下半部分”（用户空间和可能的地址空洞）的地址。

	if (unlikely(x > y)) {
		// 当x > y，说明 x >= __START_KERNEL_map，地址在内核代码映射区域
		// 将内核虚拟地址转换为物理地址, 转换公式：物理地址 = (虚拟地址 - __START_KERNEL_map) + phys_base
		// phys_base 是内核实际加载的物理基地址
		x = y + phys_base;

		if (y >= KERNEL_IMAGE_SIZE)
			// 检查偏移量是否超出内核镜像大小
			return false;
	} else {
		// 说明 x < __START_KERNEL_map，地址在直接映射区域
		// 将直接映射区的虚拟地址转换为物理地址,物理地址 = 虚拟地址 - PAGE_OFFSET
		x = y + (__START_KERNEL_map - PAGE_OFFSET);

		/* carry flag will be set if starting x was >= PAGE_OFFSET */
		// 检查转换过程中是否发生了意外的溢出,
		// 检查计算出的物理地址是否在有效的物理地址范围内
		if ((x > y) || !phys_addr_valid(x))
			// 任一条件成立都返回 false
			return false;
	}

	return pfn_valid(x >> PAGE_SHIFT);
}
EXPORT_SYMBOL(__virt_addr_valid);

#else

#ifdef CONFIG_DEBUG_VIRTUAL
unsigned long __phys_addr(unsigned long x)
{
	unsigned long phys_addr = x - PAGE_OFFSET;
	/* VMALLOC_* aren't constants  */
	VIRTUAL_BUG_ON(x < PAGE_OFFSET);
	VIRTUAL_BUG_ON(__vmalloc_start_set && is_vmalloc_addr((void *) x));
	/* max_low_pfn is set early, but not _that_ early */
	if (max_low_pfn) {
		VIRTUAL_BUG_ON((phys_addr >> PAGE_SHIFT) > max_low_pfn);
		BUG_ON(slow_virt_to_phys((void *)x) != phys_addr);
	}
	return phys_addr;
}
EXPORT_SYMBOL(__phys_addr);
#endif

bool __virt_addr_valid(unsigned long x)
{
	if (x < PAGE_OFFSET)
		return false;
	if (__vmalloc_start_set && is_vmalloc_addr((void *) x))
		return false;
	if (x >= FIXADDR_START)
		return false;
	return pfn_valid((x - PAGE_OFFSET) >> PAGE_SHIFT);
}
EXPORT_SYMBOL(__virt_addr_valid);

#endif	/* CONFIG_X86_64 */

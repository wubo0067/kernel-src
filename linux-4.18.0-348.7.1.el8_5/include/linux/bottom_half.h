/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_BH_H
#define _LINUX_BH_H

#include <linux/preempt.h>

#ifdef CONFIG_TRACE_IRQFLAGS
extern void __local_bh_disable_ip(unsigned long ip, unsigned int cnt);
#else
static __always_inline void __local_bh_disable_ip(unsigned long ip,
						  unsigned int cnt)
{
	preempt_count_add(cnt);
	// 这是一个编译器屏障，防止编译器重排序后续的代码到这个函数调用之前。
	barrier();
}
#endif

static inline void local_bh_disable(void)
{
	// _THIS_IP_:用于获取当前代码的指令指针，在某些体系结构上，它可能被定义为返回当前函数的地址
	// SOFTIRQ_DISABLE_OFFSET: 这个宏定义了禁用软中断时要增加的抢占计数器的值，0x200
	// 为什么抢占计数器的软中断域加 2
	__local_bh_disable_ip(_THIS_IP_, SOFTIRQ_DISABLE_OFFSET);
}

extern void _local_bh_enable(void);
extern void __local_bh_enable_ip(unsigned long ip, unsigned int cnt);

static inline void local_bh_enable_ip(unsigned long ip)
{
	__local_bh_enable_ip(ip, SOFTIRQ_DISABLE_OFFSET);
}

static inline void local_bh_enable(void)
{
	__local_bh_enable_ip(_THIS_IP_, SOFTIRQ_DISABLE_OFFSET);
}

#endif /* _LINUX_BH_H */

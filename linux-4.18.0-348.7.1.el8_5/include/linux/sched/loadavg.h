/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SCHED_LOADAVG_H
#define _LINUX_SCHED_LOADAVG_H

/*
 * These are the constant used to fake the fixed-point load-average
 * counting. Some notes:
 *  - 11 bit fractions expand to 22 bits by the multiplies: this gives
 *    a load-average precision of 10 bits integer + 11 bits fractional
 *  - if you want to count load-averages more often, you need more
 *    precision, or rounding will get you. With 2-second counting freq,
 *    the EXP_n values would be 1981, 2034 and 2043 if still using only
 *    11 bit fractions.
 */
extern unsigned long avenrun[]; /* Load averages */
extern void get_avenrun(unsigned long *loads, unsigned long offset, int shift);

#define FSHIFT 11 /* nr of bits of precision */
#define FIXED_1 (1 << FSHIFT) /* 1.0 as fixed-point */
#define LOAD_FREQ (5 * HZ + 1) /* 5 sec intervals */
#define EXP_1 1884 /* 1/exp(5sec/1min) as fixed-point */
#define EXP_5 2014 /* 1/exp(5sec/5min) */
#define EXP_15 2037 /* 1/exp(5sec/15min) */

/*
 * a1 = a0 * e + a * (1 - e)
 */
static inline unsigned long calc_load(unsigned long load, unsigned long exp,
				      unsigned long active)
{
	unsigned long newload;

	newload = load * exp + active * (FIXED_1 - exp);
	if (active >= load)
		newload += FIXED_1 - 1;

	return newload / FIXED_1;
}

extern unsigned long calc_load_n(unsigned long load, unsigned long exp,
				 unsigned long active, unsigned int n);

/*
这个宏用于提取并格式化定点数的小数部分。
(x) & (FIXED_1-1):
FIXED_1-1 是 2047（二进制 11111111111），它是一个掩码，用于提取 x 的低 11 位（即小数部分）。
例如，对于定点数 6656（二进制 1101000000000）：
6656 & 2047 = 6656 & 11111111111 = 10000000000 (二进制) = 1024 (十进制)。
这 1024 代表小数部分 1024/2048 = 0.5。

(...) * 100:
将提取出的小数部分乘以 100。这是为了将小数转换为百分比形式，便于显示。
继续上面的例子：1024 * 100 = 102400。

LOAD_INT(...):
最后，再次使用 LOAD_INT 宏（右移 11 位）来获取最终的“小数部分”显示值。
LOAD_INT(102400) = 102400 >> 11 = 50。


举例说明
假设有一个定点数 x = 5376，它代表的浮点数是 5376 / 2048 = 2.625。

提取整数部分：

LOAD_INT(5376) = 5376 >> 11 = 2。
提取小数部分：

提取低 11 位: 5376 & 2047 = 1280 (这代表 1280/2048 = 0.625)。
乘以 100: 1280 * 100 = 128000。
右移 11 位: LOAD_INT(128000) = 128000 >> 11 = 62。
所以，定点数 5376 会被格式化显示为 2.62 (通常会四舍五入或截断)。
*/

#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1 - 1)) * 100)

extern void calc_global_load(void);

#endif /* _LINUX_SCHED_LOADAVG_H */

#include "dwt_delay.h"
#include "system_stm32f10x.h"

/* 绝对地址定义，不依赖 CMSIS 头文件版本 */
#define DLY_DEMCR        (*(volatile uint32_t *)0xE000EDFCu)
#define DLY_DWT_CTRL     (*(volatile uint32_t *)0xE0001000u)
#define DLY_DWT_CYCCNT   (*(volatile uint32_t *)0xE0001004u)
#define DLY_DEMCR_TRCENA (1uL << 24)
#define DLY_CYCCNT_EN    (1uL << 0)

void DWT_Delay_Init(void)
{
	DLY_DEMCR |= DLY_DEMCR_TRCENA;   /* 打开 DWT */
	DLY_DWT_CYCCNT = 0u;
	DLY_DWT_CTRL |= DLY_CYCCNT_EN;   /* 使能周期计数 */
}

void DWT_Delay_us(uint32_t us)
{
	uint32_t start = DLY_DWT_CYCCNT;
	uint32_t ticks = us * (SystemCoreClock / 1000000u);   /* 72MHz → 72 周期/µs */

	while ((DLY_DWT_CYCCNT - start) < ticks)
	{
	}
}

#ifndef __DWT_DELAY_H
#define __DWT_DELAY_H
#include "stm32f10x.h"

/*
 * 基于内核 DWT 周期计数器的微秒延时。
 * 为什么不用 Delay.c 里的 Delay_us：它直接改写 SysTick 寄存器，
 * 调度器启动后一用就会打乱 FreeRTOS 心跳（1ms tick 被关掉/篡改）。
 * DWT 是独立的核内调试计数器，不碰 SysTick，可在任务里随时使用。
 * 须先调用一次 DWT_Delay_Init()（在调度器启动前）。
 */
void DWT_Delay_Init(void);
void DWT_Delay_us(uint32_t us);

#endif

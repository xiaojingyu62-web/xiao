#ifndef __BH1750_H
#define __BH1750_H
#include "stm32f10x.h"
#include <stdbool.h>

/*
 * BH1750 光照传感器驱动，软件 I²C 挂 PB8(SCL)/PB9(SDA)
 * 与 OLED(0x3C) 共总线，本器件地址 0x23（写 0x46 / 读 0x47）。
 * 总线为共享资源：任务中调用 BH1750_Read 前后须用互斥量与 OLED 操作互斥。
 */

void BH1750_GPIO_Init(void);   /* PB8/PB9 开漏输出（在 OLED_Init 之后调用） */
bool BH1750_Init(void);        /* 上电 + 清除状态 + 高分辨率模式(1lx,120ms) */
bool BH1750_Read(uint16_t *lux);

#endif

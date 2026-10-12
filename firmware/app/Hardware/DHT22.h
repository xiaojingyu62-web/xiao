#ifndef __DHT22_H
#define __DHT22_H
#include "stm32f10x.h"
#include <stdbool.h>

/*
 * DHT22 (AM2302) 单总线驱动，数据脚 = PA1
 * 注意：手册规定最小采样间隔 2s（0.5Hz），调用方负责节流！
 * DHT22_Read 内部使用临界区保护 ~3ms 的时序敏感段，仅可在 FreeRTOS 任务中调用。
 */

void DHT22_GPIO_Init(void);

/* 读取成功返回 true；temp 单位 0.1°C，hum 单位 0.1%RH */
bool DHT22_Read(int16_t *temp_tenths, uint16_t *hum_tenths);

#endif

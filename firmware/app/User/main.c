/*
 * 单节点传感器检测版 v1 —— STM32F103C8T6 + FreeRTOS + 标准外设库
 *
 * 架构：2 任务 + 1 队列 + 1 互斥量
 *   [task_sample  prio2]  500ms 节拍：BH1750 每拍读（I2C 互斥保护），
 *                         DHT22 每 4 拍读（手册最小间隔 2s）
 *        │  xFrameQueue（sensor_frame_t）
 *        ▼
 *   [task_display prio1]  OLED 分行显示 + LED1 心跳翻转（I2C 互斥保护）
 *
 * I²C(PB8/PB9) 为 OLED 与 BH1750 共享总线 → 互斥量保护（任务间同步实例）
 * DHT22 = PA1 单总线；上位机/RS485 通信任务为下一阶段加入。
 */
#include "stm32f10x.h"
#include "LED.h"
#include "OLED.h"
#include "DHT22.h"
#include "BH1750.h"
#include "dwt_delay.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <stdio.h>

/*================ 任务配置 ================*/
#define PRIO_SAMPLE        2
#define PRIO_DISPLAY       1
#define STK_SAMPLE         128          /* 单位：字 */
#define STK_DISPLAY        192          /* OLED+sprintf，给宽裕些 */
#define SAMPLE_PERIOD_MS   500
#define DHT_PERIOD_TICKS   4            /* 500ms × 4 = 2s，满足 DHT22 最小采样间隔 */

/*================ 数据帧 ================*/
typedef struct
{
	uint32_t seq;
	int16_t  temp10;        /* 0.1 °C  */
	uint16_t hum10;         /* 0.1 %RH */
	uint16_t lux;
	uint16_t dht_fail;      /* 累计读失败（自检素材） */
	uint16_t bh_fail;
	uint8_t  dht_valid;     /* 尚未成功读过 DHT22 时为 0 */
} sensor_frame_t;

TaskHandle_t xSampleHandle;
TaskHandle_t xDisplayHandle;
QueueHandle_t xFrameQueue;
SemaphoreHandle_t xI2CMutex;   /* 保护 PB8/PB9 软件 I²C 总线 */

/*---------------- 采集任务 ----------------*/
void vSampleTask(void *pvParameters)
{
	sensor_frame_t f;
	uint32_t tick = 0u;
	int16_t t = 0;
	uint16_t h = 0u;
	uint16_t lux = 0u;

	(void)pvParameters;

	f.seq = 0u; f.temp10 = 0; f.hum10 = 0u; f.lux = 0u;
	f.dht_fail = 0u; f.bh_fail = 0u; f.dht_valid = 0u;

	while (1)
	{
		f.seq = tick;

		/* BH1750：每拍（与 OLED 共总线 → 互斥量） */
		xSemaphoreTake(xI2CMutex, portMAX_DELAY);
		if (BH1750_Read(&lux))
		{
			f.lux = lux;
		}
		else
		{
			f.bh_fail++;
		}
		xSemaphoreGive(xI2CMutex);

		/* DHT22：每 4 拍一次（2s） */
		if ((tick % DHT_PERIOD_TICKS) == 0u)
		{
			if (DHT22_Read(&t, &h))
			{
				f.temp10 = t;
				f.hum10 = h;
				f.dht_valid = 1u;
			}
			else
			{
				f.dht_fail++;
			}
		}

		xQueueSend(xFrameQueue, &f, portMAX_DELAY);
		tick++;
		vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
	}
}

/*---------------- 显示任务 ----------------*/
void vDisplayTask(void *pvParameters)
{
	sensor_frame_t f;
	char s[18];

	(void)pvParameters;

	while (1)
	{
		if (xQueueReceive(xFrameQueue, &f, portMAX_DELAY) == pdTRUE)
		{
			/* 组帧（不碰总线） */
			if (f.dht_valid)
			{
				sprintf(s, "T:%2d.%1dC H:%2d.%1d%%",
				        (int)(f.temp10 / 10),
				        (int)((f.temp10 < 0 ? -f.temp10 : f.temp10) % 10),
				        (int)(f.hum10 / 10),
				        (int)(f.hum10 % 10));
			}
			else
			{
				sprintf(s, "T:--.-C H:--.-%");
			}

			/* 总线访问统一加锁 */
			xSemaphoreTake(xI2CMutex, portMAX_DELAY);
			OLED_ShowString(1, 1, s);

			sprintf(s, "Lux:%4u lx", (unsigned int)f.lux);
			OLED_ShowString(2, 1, s);

			sprintf(s, "Seq:%6lu", (unsigned long)f.seq);
			OLED_ShowString(3, 1, s);

			sprintf(s, "F:%u/%u", (unsigned int)f.dht_fail, (unsigned int)f.bh_fail);
			OLED_ShowString(4, 1, s);
			xSemaphoreGive(xI2CMutex);

			LED1_Turn();               /* 500ms 心跳 */
		}
	}
}

/*---------------- 主函数 ----------------*/
int main(void)
{
	/* 裸机初始化（调度器启动前，I²C 无并发故不加锁） */
	LED_Init();
	OLED_Init();
	DWT_Delay_Init();
	DHT22_GPIO_Init();
	BH1750_GPIO_Init();               /* 必须在 OLED_Init 之后：统一为开漏 */
	BH1750_Init();                    /* 上电+清状态+高分辨率模式 */

	/* 内核对象 */
	xFrameQueue = xQueueCreate(4u, sizeof(sensor_frame_t));
	xI2CMutex = xSemaphoreCreateMutex();
	configASSERT(xFrameQueue != NULL);
	configASSERT(xI2CMutex != NULL);

	/* 任务 */
	xTaskCreate(vSampleTask,  "sample",  STK_SAMPLE,  NULL, PRIO_SAMPLE,  &xSampleHandle);
	xTaskCreate(vDisplayTask, "display", STK_DISPLAY, NULL, PRIO_DISPLAY, &xDisplayHandle);

	vTaskStartScheduler();

	/* 能走到这里 = 堆不足，无法创建空闲任务 */
	while (1)
	{
	}
}

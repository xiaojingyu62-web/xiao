/*
 * 毕设节点固件起步模板 —— STM32F103C8T6 + FreeRTOS + 标准外设库
 * 结构：生产者任务(模拟采集) --队列--> 消费者任务(显示)
 * 后续在此骨架上叠加：RS485通信任务 / 自检任务 / 三类故障判据
 */
#include "stm32f10x.h"                  // Device header

#include "LED.h"
#include "OLED.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*================ 任务配置 ================*/
#define PRIO_PROD        2              // 生产者优先级（数值大者优先）
#define PRIO_CONS        1              // 消费者优先级
#define STK_PROD         128            // 栈大小，单位：字（128字=512字节）
#define STK_CONS         128

#define SAMPLE_PERIOD_MS 500            // 采样周期 500ms

/*================ 全局句柄 ================*/
TaskHandle_t xProdTaskHandle;
TaskHandle_t xConsTaskHandle;
QueueHandle_t xDataQueue;               // 数据队列：采集值从生产者流向消费者

void vProducerTask(void *pvParameters);
void vConsumerTask(void *pvParameters);

int main(void)
{
	/* 1. 硬件初始化（调度器启动前，裸机阶段） */
	LED_Init();
	OLED_Init();
	OLED_ShowString(1, 1, "FreeRTOS Template");
	OLED_ShowString(2, 1, "CNT:");

	/* 2. 内核对象：队列深度4，每项4字节(uint32_t) */
	xDataQueue = xQueueCreate(4, sizeof(uint32_t));
	configASSERT(xDataQueue != NULL);   // 创建失败直接停在这里，方便排查

	/* 3. 创建任务 */
	xTaskCreate(vProducerTask,          // 任务函数
	            "prod",                  // 任务名（调试用）
	            STK_PROD,                // 栈大小（字）
	            NULL,                    // 传给任务的参数
	            PRIO_PROD,               // 优先级
	            &xProdTaskHandle);       // 任务句柄（可不取）

	xTaskCreate(vConsumerTask, "cons", STK_CONS, NULL, PRIO_CONS, &xConsTaskHandle);

	/* 4. 启动调度器——之后 CPU 归 FreeRTOS 管，正常永不返回 */
	vTaskStartScheduler();

	/* 能走到这里说明：堆内存不足，无法创建空闲任务 */
	while (1)
	{
	}
}

/*
 * 生产者：每 500ms 产生一个采样值入队。
 * TODO: 后续把 ulSample++ 换成真实传感器读数（ADC/DHT22...）
 */
void vProducerTask(void *pvParameters)
{
	uint32_t ulSample = 0;

	(void)pvParameters;                 // 本任务未用到参数，显式消警告

	while (1)
	{
		ulSample++;
		xQueueSend(xDataQueue, &ulSample, portMAX_DELAY);  // 入队（队列满则阻塞）
		vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));       // 500ms，让出CPU
	}
}

/*
 * 消费者：阻塞等队列，取到值显示到OLED并翻转LED1。
 * LED1 翻转节奏 = 实际收到的数据节奏，肉眼可验证队列通路。
 */
void vConsumerTask(void *pvParameters)
{
	uint32_t ulValue;

	(void)pvParameters;

	while (1)
	{
		if (xQueueReceive(xDataQueue, &ulValue, portMAX_DELAY) == pdPASS)  // 阻塞等
		{
			OLED_ShowNum(2, 5, ulValue, 5);
			LED1_Turn();
		}
	}
}

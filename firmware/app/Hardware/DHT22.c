#include "DHT22.h"
#include "dwt_delay.h"
#include "FreeRTOS.h"
#include "task.h"

#define DHT_PORT   GPIOA
#define DHT_PIN    GPIO_Pin_1
#define DHT_RCC    RCC_APB2Periph_GPIOA

static void DHT_PinOut(void)
{
	GPIO_InitTypeDef gi;
	gi.GPIO_Pin = DHT_PIN;
	gi.GPIO_Mode = GPIO_Mode_Out_PP;
	gi.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DHT_PORT, &gi);
}

static void DHT_PinIn(void)
{
	GPIO_InitTypeDef gi;
	gi.GPIO_Pin = DHT_PIN;
	gi.GPIO_Mode = GPIO_Mode_IPU;      /* 模块虽自带上拉，再开内部上拉留余量 */
	gi.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DHT_PORT, &gi);
}

static uint8_t DHT_Level(void)
{
	return (uint8_t)GPIO_ReadInputDataBit(DHT_PORT, DHT_PIN);
}

void DHT22_GPIO_Init(void)
{
	RCC_APB2PeriphClockCmd(DHT_RCC, ENABLE);
	DHT_PinOut();
	GPIO_SetBits(DHT_PORT, DHT_PIN);   /* 空闲高 */
}

bool DHT22_Read(int16_t *temp_tenths, uint16_t *hum_tenths)
{
	uint8_t data[5];
	uint8_t i, j;
	uint32_t tmo;
	uint16_t raw_h, raw_t;

	/* ---- 起始信号：主机拉低 ≥1ms（临界区外，此段无时序敏感） ---- */
	DHT_PinOut();
	GPIO_ResetBits(DHT_PORT, DHT_PIN);
	DWT_Delay_us(1200);
	DHT_PinIn();                       /* 释放总线，等待从机应答 */

	/* ---- 应答 + 40 位数据：时序敏感（µs 级），临界区保护约 3ms ---- */
	taskENTER_CRITICAL();

	DWT_Delay_us(30);                  /* 等待从机 20~40µs 后应答 */

	/* 1) 等从机拉低（应答 80µs 低电平开始） */
	tmo = 1000u;
	while (DHT_Level())
	{
		if (--tmo == 0u) { taskEXIT_CRITICAL(); return false; }
		DWT_Delay_us(1u);
	}
	/* 2) 等应答低电平结束 */
	tmo = 1000u;
	while (!DHT_Level())
	{
		if (--tmo == 0u) { taskEXIT_CRITICAL(); return false; }
		DWT_Delay_us(1u);
	}
	/* 3) 等应答高电平结束（此后进入第 1 位的 50µs 低电平） */
	tmo = 1000u;
	while (DHT_Level())
	{
		if (--tmo == 0u) { taskEXIT_CRITICAL(); return false; }
		DWT_Delay_us(1u);
	}

	/* 4) 40 位：每位 = 50µs 低 + 高电平（0:26~28µs，1:70µs） */
	for (i = 0; i < 5u; i++)
	{
		data[i] = 0u;
		for (j = 0; j < 8u; j++)
		{
			/* 等本位低电平结束（上升沿） */
			tmo = 1000u;
			while (!DHT_Level())
			{
				if (--tmo == 0u) { taskEXIT_CRITICAL(); return false; }
				DWT_Delay_us(1u);
			}
			DWT_Delay_us(40);          /* 40µs 后采样：0 位已回低，1 位仍高 */
			data[i] <<= 1;
			if (DHT_Level())
			{
				data[i] |= 1u;
				/* 等 1 位的高电平结束 */
				tmo = 1000u;
				while (DHT_Level())
				{
					if (--tmo == 0u) { taskEXIT_CRITICAL(); return false; }
					DWT_Delay_us(1u);
				}
			}
		}
	}

	taskEXIT_CRITICAL();

	/* ---- 校验：前 4 字节无符号和的低 8 位 == 第 5 字节（AM2302 手册） ---- */
	if ((uint8_t)(data[0] + data[1] + data[2] + data[3]) != data[4])
	{
		return false;
	}

	raw_h = (uint16_t)(((uint16_t)data[0] << 8) | data[1]);          /* 0.1%RH */
	raw_t = (uint16_t)(((uint16_t)(data[2] & 0x7Fu) << 8) | data[3]); /* 0.1°C  */

	*hum_tenths = raw_h;
	*temp_tenths = (data[2] & 0x80u) ? (int16_t)(-(int16_t)raw_t) : (int16_t)raw_t;
	return true;
}

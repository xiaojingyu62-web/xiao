#include "BH1750.h"
#include "dwt_delay.h"

#define SCL_PORT   GPIOB
#define SCL_PIN    GPIO_Pin_8
#define SDA_PORT   GPIOB
#define SDA_PIN    GPIO_Pin_9
#define BH1750_RCC RCC_APB2Periph_GPIOB

#define BH1750_ADDR_W 0x46u          /* 0x23 << 1      */
#define BH1750_ADDR_R 0x47u

#define CMD_POWER_ON  0x01u
#define CMD_RESET     0x07u
#define CMD_H_RES     0x10u          /* 高分辨率 1lx，典型 120ms */

/* ---- 位拍时序（µs 级，DWT 延时，不依赖 RTOS） ---- */
static void i2c_dly(void) { DWT_Delay_us(4u); }

static void SCL_H(void) { GPIO_SetBits(SCL_PORT, SCL_PIN); i2c_dly(); }
static void SCL_L(void) { GPIO_ResetBits(SCL_PORT, SCL_PIN); i2c_dly(); }
static void SDA_H(void) { GPIO_SetBits(SDA_PORT, SDA_PIN); i2c_dly(); }
static void SDA_L(void) { GPIO_ResetBits(SDA_PORT, SDA_PIN); i2c_dly(); }
static uint8_t SDA_R(void) { return (uint8_t)GPIO_ReadInputDataBit(SDA_PORT, SDA_PIN); }

static void i2c_start(void)
{
	SDA_H();
	SCL_H();
	SDA_L();                          /* SCL 高时 SDA 下降 = 起始 */
	i2c_dly();
	SCL_L();
}

static void i2c_stop(void)
{
	SDA_L();
	SCL_H();
	i2c_dly();
	SDA_H();                          /* SCL 高时 SDA 上升 = 停止 */
	i2c_dly();
}

/* 发送 1 字节，返回 1=收到 ACK */
static uint8_t i2c_write(uint8_t byte)
{
	uint8_t i;
	uint8_t ack;

	for (i = 0; i < 8u; i++)
	{
		if (byte & 0x80u) { SDA_H(); } else { SDA_L(); }
		byte <<= 1;
		SCL_H();
		SCL_L();
	}
	SDA_H();                          /* 释放 SDA 等从机 ACK */
	i2c_dly();
	SCL_H();
	i2c_dly();
	ack = (uint8_t)!SDA_R();          /* 从机拉低 = ACK */
	SCL_L();
	return ack;
}

/* 接收 1 字节，ack=1 回 ACK，ack=0 回 NACK */
static uint8_t i2c_read(uint8_t ack)
{
	uint8_t i;
	uint8_t byte = 0u;

	SDA_H();                          /* 释放 SDA（开漏=输入） */
	for (i = 0; i < 8u; i++)
	{
		byte <<= 1;
		SCL_H();
		i2c_dly();
		if (SDA_R()) { byte |= 1u; }
		SCL_L();
		i2c_dly();
	}
	if (ack) { SDA_L(); } else { SDA_H(); }
	i2c_dly();
	SCL_H();
	i2c_dly();
	SCL_L();
	SDA_H();                          /* 释放总线 */
	return byte;
}

void BH1750_GPIO_Init(void)
{
	GPIO_InitTypeDef gi;

	RCC_APB2PeriphClockCmd(BH1750_RCC, ENABLE);
	gi.GPIO_Pin = SCL_PIN | SDA_PIN;
	gi.GPIO_Mode = GPIO_Mode_Out_OD;  /* 开漏，配合模块板载上拉 */
	gi.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gi);
	GPIO_SetBits(SCL_PORT, SCL_PIN);
	GPIO_SetBits(SDA_PORT, SDA_PIN);
}

static bool BH1750_WriteCmd(uint8_t cmd)
{
	uint8_t ok;

	i2c_start();
	ok = i2c_write(BH1750_ADDR_W);
	if (ok) { ok = i2c_write(cmd); }
	i2c_stop();
	return (bool)ok;
}

bool BH1750_Init(void)
{
	if (!BH1750_WriteCmd(CMD_POWER_ON)) { return false; }
	BH1750_WriteCmd(CMD_RESET);
	if (!BH1750_WriteCmd(CMD_H_RES))   { return false; }
	return true;                       /* 首次有效读数约 180ms 后可用 */
}

bool BH1750_Read(uint16_t *lux)
{
	uint8_t hi, lo;
	uint16_t raw;

	i2c_start();
	if (!i2c_write(BH1750_ADDR_W)) { i2c_stop(); return false; }
	i2c_start();                       /* 重复起始 */
	if (!i2c_write(BH1750_ADDR_R)) { i2c_stop(); return false; }
	hi = i2c_read(1u);                 /* ACK 继续 */
	lo = i2c_read(0u);                 /* NACK 结束 */
	i2c_stop();

	raw = (uint16_t)(((uint16_t)hi << 8) | lo);
	*lux = (uint16_t)(((uint32_t)raw * 5u) / 6u);   /* lux = raw / 1.2 */
	return true;
}

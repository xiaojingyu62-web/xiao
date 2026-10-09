#include "stm32f10x.h"                  // Device header

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//开启GPIOA时钟
	
	GPIO_InitTypeDef GPIO_InitStructure;				//定义结构体变量
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽输出模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;//使用GPIOA的1口和2口
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);				//对GPIOA初始化
	
	GPIO_SetBits(GPIOA,GPIO_Pin_1 | GPIO_Pin_2);//先置为高电平，防止上电就亮
}

/**
  * @brief 点亮LED1,即把GPIOA1口置低电平
  * @param  无
  * @retval 无
  */
void LED1_ON(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);
}

/**
  * @brief 熄灭LED1,即把GPIOA1口置高电平
  * @param  无
  * @retval 无
  */
void LED1_OFF(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_1);
}

/**
  * @brief LED1取反
  * @param  无
  * @retval 无
  */
void LED1_Turn(void)
{
	if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_1) == 0)//若A1口输出寄存器为0，设置为1
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_1);
	}
	else											//为0则置1
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_1);
	}
}

/**
  * @brief 点亮LED2,即把GPIOA2口置低电平
  * @param  无
  * @retval 无
  */
void LED2_ON(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_2);
}

/**
  * @brief 熄灭LED2,即把GPIOA2口置高电平
  * @param  无
  * @retval 无
  */
void LED2_OFF(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_2);
}

/**
  * @brief LED2取反
  * @param  无
  * @retval 无
  */
void LED2_Turn(void)
{
	if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_2) == 0)//若A2口输出寄存器为0，设置为1
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_2);
	}
	else											//为0则置1
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_2);
	}
}

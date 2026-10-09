#include "stm32f10x.h"                  // Device header
#include "Delay.h"

void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//打开GPIOB时钟
	
	GPIO_InitTypeDef GPIO_InitStructure;				//定义结构体变量
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		//上拉输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_11;//用B1口和B11口
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//输出速度,但输入用不上,写50没影响
	GPIO_Init(GPIOB,&GPIO_InitStructure);				//GPIOB初始化
}

/**
  * @brief 获取按键值
  * @param  无
  * @retval KeyNum,要读取的按键值
  */
uint8_t Key_GetNum(void)
{
	uint8_t KeyNum = 0;							//默认为0,没有按键按下返回0
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1) == 0)//读取B1口的输入,如果按下(0)
	{
		Delay_ms(20);							//按键消抖
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1) == 0);//直到松手往下走
		Delay_ms(20);							//按键消抖
		KeyNum = 1;								//返回键码1
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11) == 0)//读取B11口的输入,如果按下(0)
	{
		Delay_ms(20);							//按键消抖
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11) == 0);//直到松手往下走
		Delay_ms(20);							//按键消抖
		KeyNum = 2;								//返回键码2
	}

	return KeyNum;
}

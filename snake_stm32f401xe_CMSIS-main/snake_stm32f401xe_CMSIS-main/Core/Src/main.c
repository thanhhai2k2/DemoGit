/*
 * main.c
 *
 *  Created on: Jan 12, 2022
 *      Author: ROJEK
 */


#include <stm32f401xe_tim.h>
#include "stm32f4xx.h"
#include "stm32f401xe_gpio.h"
#include "stm32f401xe_i2c.h"
#include "stm32f401xe_rcc.h"
#include "stm32f401xe_pwr.h"
#include "stm32f401xe.h"
#include "SSD1306_OLED.h"
#include "GFX_BW.h"
#include "stdint.h"
#include "font_8x5.h"
#include "stddef.h"
#include "stdio.h"
#include "Snek.h"
#include "eeprom.h"

void SysClockInit(void);
void GPIO_Config(void);
void TIM9_Config(TimerHandle_t *p_handle_tim);
void I2C1_Config(I2c_Handle_t *p_handle_i2c1);
snek_game_t snek_game;

int main()
{
	I2c_Handle_t h_i2c1;
	TimerHandle_t h_tim9;

	// Configure RCC
	SysClockInit();
	RCC_ClockFreqs freqs = {0};
	RCC_GetClockFrequencies(&freqs);

	RCC_CLOCK_PWR_ENABLE();
	Pwr_EnablePvd(kPvdLevel22V, kPvdModeIrqRT);

	// Configure peripherals
	GPIO_Config();
	TIM9_Config(&h_tim9);
	TIM_StartTimer(&h_tim9);
	I2C1_Config(&h_i2c1);

	// Configure devices
	SSD1306_Init(&h_i2c1);
	Eeprom_Init(&h_i2c1);
	GFX_SetFont(font_8x5);

	// Enable IRQs
	NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);

	while(1)
	{
		snek(&snek_game);
	}
}

// handler for button DOWN
void EXTI9_5_IRQHandler(void)
{
	uint8_t temp_gpio_pin;

	if (EXTI->PR & (0b1 << SNEK_BUTTON_DOWN))
	{
		GPIO_ClearPendingEXTIFlag(SNEK_BUTTON_DOWN);
		temp_gpio_pin = SNEK_BUTTON_DOWN;
	}
	else if (EXTI->PR & (0b1 << SNEK_BUTTON_ENTER))
	{
		GPIO_ClearPendingEXTIFlag(SNEK_BUTTON_ENTER);
		temp_gpio_pin = SNEK_BUTTON_ENTER;
	}
	else if (EXTI->PR & (0b1 << SNEK_BUTTON_UP))
	{
		GPIO_ClearPendingEXTIFlag(SNEK_BUTTON_UP);
		temp_gpio_pin = SNEK_BUTTON_UP;
	}
	else if (EXTI->PR & (0b1 << SNEK_BUTTON_LEFT))
	{
		GPIO_ClearPendingEXTIFlag(SNEK_BUTTON_LEFT);
		temp_gpio_pin = SNEK_BUTTON_LEFT;
	}

	snek_button_callback(temp_gpio_pin, &snek_game);
}

// handler for buttons up/left/enter
void EXTI15_10_IRQHandler(void)
{
	uint8_t temp_gpio_pin;
	// if button is clicked
	if (EXTI->PR & (0b1 << SNEK_BUTTON_RIGHT))
	{
		GPIO_ClearPendingEXTIFlag(SNEK_BUTTON_RIGHT);
		temp_gpio_pin = SNEK_BUTTON_RIGHT;
	}

	snek_button_callback(temp_gpio_pin, &snek_game);
}

// game tick
void TIM1_BRK_TIM9_IRQHandler(void)
{
	TIM_ClearUpdateFlag(TIM9);
	snek_gametick_callback(&snek_game);

}

// system clock init function
void SysClockInit(void)
{
	RCC_ClockInitTypeDef pClockInit;

	pClockInit.apb1_prescaler = RCC_ABP_PRESCALER_DIV2;
	pClockInit.apb2_prescaler = RCC_ABP_PRESCALER_NODIV;
	pClockInit.flash_latency = RCC_FLASHLATENCY_2WS;
	pClockInit.hpre_prescaler = RCC_HPRE_PRESCALER_NODIV;
	pClockInit.oscillator_type = RCC_OSCILLATORTYPE_HSI;
	pClockInit.vos_scale = RCC_VOS_SCALE2;

	// init pll
	pClockInit.pll.source = RCC_PLL_SOURCE_HSI;
	pClockInit.pll.state = RCC_PLL_STATE_ENABLE;
	pClockInit.pll.pllm = 16;
	pClockInit.pll.plln = 336;
	pClockInit.pll.pllp = RCC_PLLP_DIV4;
	pClockInit.pll.pllq = 7;

	RCC_InitClock(&pClockInit);
}

//configure GPIO used in the project
void GPIO_Config(void)
{
	GPIO_Handle_t GPIOx;
	GPIOx.PinConfig.PinNumber = GPIO_PIN_5;
	GPIOx.PinConfig.Mode = GPIO_PIN_MODE_OUTPUT;
	GPIOx.PinConfig.OutputType = GPIO_PIN_OT_PP;
	GPIOx.PinConfig.PullUpPullDown = GPIO_PIN_PUPD_NOPULL;
	GPIOx.pGPIOx = GPIOA;
	GPIO_InitPin(&GPIOx);

	GPIOx.PinConfig.Mode = GPIO_PIN_MODE_EXTI_FT;
	GPIOx.pGPIOx = SNEK_BUTTON_ENTER_PORT;
	GPIOx.PinConfig.PinNumber = SNEK_BUTTON_ENTER;
	GPIO_InitPin(&GPIOx);

	GPIOx.pGPIOx = SNEK_BUTTON_DOWN_PORT;
	GPIOx.PinConfig.PinNumber = SNEK_BUTTON_DOWN;
	GPIO_InitPin(&GPIOx);

	GPIOx.pGPIOx = SNEK_BUTTON_UP_PORT;
	GPIOx.PinConfig.PinNumber = SNEK_BUTTON_UP;
	GPIO_InitPin(&GPIOx);

	GPIOx.pGPIOx = SNEK_BUTTON_LEFT_PORT;
	GPIOx.PinConfig.PinNumber = SNEK_BUTTON_LEFT;
	GPIO_InitPin(&GPIOx);

	GPIOx.pGPIOx = SNEK_BUTTON_RIGHT_PORT;
	GPIOx.PinConfig.PinNumber = SNEK_BUTTON_RIGHT;
	GPIO_InitPin(&GPIOx);
	
}

void I2C1_Config(I2c_Handle_t *p_handle_i2c1)
{
	p_handle_i2c1->p_i2cx = I2C1;
	p_handle_i2c1->i2c_config.abp1_freq_mhz = 42;
	p_handle_i2c1->i2c_config.speed = I2C_SPEED_FAST_DUTY0;

	I2c_Init(p_handle_i2c1);
}

void TIM9_Config(TimerHandle_t *p_handle_tim)
{

	p_handle_tim->p_timx = TIM9;
	p_handle_tim->timer_config.prescaler = 4199;
	p_handle_tim->timer_config.autoreload = 9999;

	TIM_InitTimer(p_handle_tim);

	TIM_EnableIRQ(p_handle_tim, TIM_IRQFLAG_UIE);
}

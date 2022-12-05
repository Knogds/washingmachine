#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <stdint.h>
#include "washingmachine.h"


void adc_setup(void)
{
	rcc_periph_clock_enable(RCC_ADC1);

        gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO0 | GPIO1);

	/*
	The ADC can be powered-on by setting the ADON bit in the ADC_CR2 register. When the
	ADON bit is set for the first time, it wakes up the ADC from Power Down mode.
	*/
	ADC1_CR2 &= ~(ADC_CR2_ADON);
	ADC1_CR2 |= ADC_CR2_ADON;

        /* Wait for ADC starting up. */
        for (int i = 0; i < 800000; i++)    /* Wait a bit. */
                __asm__("nop");


	/*
	Calibration
	The ADC has an built-in self calibration mode. Calibration significantly reduces accuracy
	errors due to internal capacitor bank variations. During calibration, an error-correction code
	(digital word) is calculated for each capacitor, and during all subsequent conversions, the
	error contribution of each capacitor is removed using this code.
	Calibration is started by setting the CAL bit in the ADC_CR2 register. Once calibration is
	over, the CAL bit is reset by hardware and normal conversion can be performed. It is
	recommended to calibrate the ADC once at power-on. The calibration codes are stored in
	the ADC_DR as soon as the calibration phase ends.
	Note:
	It is recommended to perform a calibration after each power-up.
	Before starting a calibration, the ADC must have been in power-on state (ADON bit = ‘1’) for
	at least two ADC clock cycles.
	*/
	ADC1_CR2 |= ADC_CR2_CAL;
	/* wait until calibration is over */
	while (!(ADC1_CR2 & ADC_CR2_CAL));


	/* Scan mode
	This mode is used to scan a group of analog channels. Scan mode can be selected by setting the SCAN bit in the ADC_CR1 register.
	Once this bit is set, ADC scans all the channels selected in the ADC_SQRx registers (for regular channels) or in the ADC_JSQR (for injected channels).
	*/
	ADC1_CR1 |= ADC_CR1_SCAN;


	/*
	When using scan mode, DMA bit must be set and the direct memory access controller is
	used to transfer the converted data of regular group channels to SRAM after each update of
	the ADC_DR register.
	Note:
	Only ADC1 and ADC3 have this DMA capability. ADC2-converted data can be transferred in
	dual ADC mode using DMA thanks to master ADC1.
	*/
	ADC1_CR2 |= ADC_CR2_DMA;


	/*
	If the CONT bit is set, conversion does not stop at the last selected
	group channel but continues again from the first selected group channel.
	*/
	ADC1_CR2 |= ADC_CR2_CONT;


	/*
	Auto-injection
	If the JAUTO bit is set, then the injected group channels are automatically converted after
	the regular group channels. This can be used to convert a sequence of up to 20 conversions
	programmed in the ADC_SQRx and ADC_JSQR registers.
	*/
	ADC1_CR1 |= ADC_CR1_JAUTO;


	/*
	ADC injected sequence register 
	Bits 21:20 JL[1:0]: Injected sequence length
	These bits are written by software to define the total number of conversions in the injected
	channel conversion sequence.
	00: 1 conversion
	01: 2 conversions
	10: 3 conversions
	11: 4 conversions
	*/
	ADC1_JSQR |= (1 << 20);
	/* In injected mode, conversions are made in order: JSQ4-selected-channel, JSQ3, JSQ2, JSQ1...
	here we have just 2 conversions: chann. 0 + chann. 1 */
	ADC1_JSQR |= (1 << 15); /* JSQ4[4:1] already zero (channel) after reset */

	/*
	Bits 29:0 SMPx[2:0]: Channel x Sample time selection
	These bits are written by software to select the sample time individually for each channel.
	During sample cycles channel selection bits must remain unchanged.
	000: 1.5 cycles
	001: 7.5 cycles
	010: 13.5 cycles
	011: 28.5 cycles
	100: 41.5 cycles
	101: 55.5 cycles
	110: 71.5 cycles
	111: 239.5 cycles
	SMP0[2:0]
	SMP1[5:3]
	*/
	ADC1_SMPR2 |= (0b011 << 3) | (0b011 << 0); /* 28.5 cycles for channel 0+1 */



	/*
	Conversion starts when ADON bit is set for a second time by software after ADC power-up
	time (t STAB ).
	*/
	ADC1_CR2 |= ADC_CR2_ADON;



        /* Wait for ADC starting up. */
	int i;
        for (i = 0; i < 800000; i++)    /* Wait a bit. */
                __asm__("nop");


}

uint16_t triac_power[100] =
{
59000, 58500, 58000, 57500, 57000, 56500, 56000, 55500, 55000, 54500, 54000, 53500, 53000, 52500, 52000,
51500, 51000, 50500, 50000, 49500, 49000, 48500, 48000, 47500, 47000, 46500, 46000, 45500, 45000, 44500,
44000, 43500, 43000, 42500, 42000, 41500, 41000, 40500, 40000, 39500, 39000, 38500, 38000, 37500, 37000,
36500, 36000, 35500, 35000, 34500, 34000, 33500, 33000, 32500, 32000, 31500, 31000, 30500, 30000, 29500,
29000, 28500, 28000, 27500, 27000, 26500, 26000, 25500, 25000, 24500, 24000, 23500, 23000, 22500, 22000,
21500, 21000, 20500, 20000, 19500, 19000, 18500, 18000, 17500, 17000, 16500, 16000, 15500, 15000, 14500,
14000, 13500, 13000, 12500, 12000, 11500, 11000, 10500, 10000, 9500
};

void motor_set_power_percent(uint8_t percent)
{
	TRIAC = triac_power[percent];
}

void timer_setup(void)
{

	/*\   TRIAC:        */
	/* \  4000 - MAX   */
	/*  \ 59950 - MIN */

	rcc_periph_clock_enable(RCC_TIM1);
	rcc_periph_clock_enable(RCC_TIM4);

	timer_reset(TIM1);
	timer_reset(TIM4);

	TIM1_ARR = 59950; /* together with TIM1_PSC=11 GIVES ABOUT 50hz */
	TIM1_CCR2 = 59950; /* about 4000 - the triac is max open */
	TIM4_ARR = 59950;
	TIM4_CCR1 = 500; /* !! BILO JE 300 - no polako je prestajalo biti premalo snage u motoru... motor je htio vrtiti samo u jednu stranu.. ili cw ili ccw, zasto? povisio na 500 sve je ponovo bilo ok.
the pulse width that triggers the triac. If a triac is triggered with a shorter pulse, it
				should reduce the triacs temperature when it is under heavy load. A too short pulse
				won't trigger the triac at all. */

	TRIAC = 59950;

	gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_TIM1_CH1); /* PA8 */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_TIM1_CH2); /* PA9 */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_TIM4_CH1); /* PB6 */

	/* 50hz je otprilike: */
	/* TIM1_ARR=62550; TIM1_PSC=22; */

	/* OPM delay */
	//TIM1_CCR1 = 1000;

	/* 100hz je otprilike: */
	/* TIM1_CCR2 = 100 je dosta da se ukljuci triac */
	/* TIM1_ARR=59950; TIM1_PSC=11; */



 	/* Control register 1 (TIMx_CR1) */
/*****************************************/
	TIM1_CR1 |= TIM_CR1_CEN;
	TIM4_CR1 |= TIM_CR1_CEN;



	/* Control register 2 (TIMx_CR2) */
/*****************************************/
	TIM1_CR2 |= TIM_CR2_MMS_COMPARE_OC2REF;


	/* Slave mode control register (TIMx_SMCR) */
/***************************************************/
	TIM1_SMCR |= TIM_SMCR_TS_TI1FP1;
	TIM1_SMCR |= TIM_SMCR_SMS_RM;
	TIM4_SMCR |= TIM_SMCR_TS_ITR0;
	TIM4_SMCR |= TIM_SMCR_SMS_RM;
	TIM4_SMCR |= TIM_SMCR_ETP;



	/* DMA/Interrupt enable register (TIMx_DIER) */
/*****************************************************/


	/* Status register (TIMx_SR) */
/*************************************/


	/* Event generation register (TIMx_EGR) */
/************************************************/


	/* Capture/compare mode register 1 (TIMx_CCMR1) */
/********************************************************/
	TIM1_CCMR1 |= TIM_CCMR1_OC2M_PWM2;
	TIM4_CCMR1 |= TIM_CCMR1_OC1M_PWM1;

	/* Capture/compare mode register 2 (TIMx_CCMR2) */
/********************************************************/


/*
	TIM1_CCER |= TIM_CCER_CC2E;
	TIM1_CCMR1 |= TIM_CCMR1_CC1S_IN_TI1; 
	TIM1_CCMR1 |= TIM_CCMR1_CC2S_OUT;
	TIM1_CCMR1 |= TIM_CCMR1_OC2M_PWM1;
	TIM1_SMCR |= TIM_SMCR_SMS_RM; 
	TIM1_SMCR |= TIM_SMCR_TS_TI1FP1;  
	TIM1_CR2 |= TIM_CR2_MMS_ENABLE; 
	TIM1_BDTR |= TIM_BDTR_MOE | TIM_BDTR_BKP;
	TIM1_CR1 |= TIM_CR1_CEN; 
*/



	/* Capture/compare enable register (TIMx_CCER) */
/*******************************************************/
	TIM1_CCER |= TIM_CCER_CC2E;
	TIM4_CCER |= TIM_CCER_CC1E;


	/* Counter (TIMx_CNT) */
/*******************************************************/



	/* Prescaler (TIMx_PSC) */
/*******************************************************/
	TIM1_PSC = 11;
	TIM4_PSC = 11;



	/* Auto-reload register (TIMx_ARR) */
/*******************************************************/


	/* Repetition counter register (TIMx_RCR) */
/*******************************************************/



	/* Capture/compare register 1 (TIMx_CCR1) */
/*******************************************************/



	/* Capture/compare register 2 (TIMx_CCR2) */
/*******************************************************/



	/* Capture/compare register 3 (TIMx_CCR3) */
/*******************************************************/



	/* Capture/compare register 4 (TIMx_CCR4) */
/*******************************************************/



	/* Break and dead-time register (TIMx_BDTR) */
/*******************************************************/
	TIM1_BDTR |= TIM_BDTR_MOE;



	/* DMA control register (TIMx_DCR) */
/*******************************************************/



	/* DMA address for full transfer (TIMx_DMAR) */
/*******************************************************/
}

void exti_setup(void)
{
	#ifdef EXTI_VECTOR_OFFSET_SRAM
	SCB_VTOR = 0;
	SCB_VTOR |= 0x20000000;
	#endif

	#ifdef EXTI_VECTOR_OFFSET_FLASH
	SCB_VTOR = 0;
	SCB_VTOR |= 0x8000000;
	#endif
}

void systick_setup(void)
{
	/* 72MHz = 72M ticks per second */
	/* it should be 72MHz but it is actually about 68MHz */ 
        systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);

        /* 64000000/6400 = 10000 overflows per second */
	/* "When the HSI is used as a PLL clock input, the maximum system clock frequency that can be achieved is
	64 MHz." str. 93 reference */
        systick_set_reload(6399);

        systick_interrupt_enable();

        /* Start counting. */
        systick_counter_enable();
}


void relays_setup(void)
{

	gpio_set_mode(RELAY_MOTOR_PORT,		GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, RELAY_MOTOR_CCW | RELAY_MOTOR_CW);
	gpio_set_mode(RELAY_VALVE_WASH_PORT,	GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, RELAY_VALVE_WASH);
	gpio_set_mode(RELAY_VALVE_PREWASH_PORT,	GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, RELAY_VALVE_PREWASH);
	gpio_set_mode(RELAY_DRAIN_PORT,		GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, RELAY_DRAIN);
	gpio_set_mode(RELAY_HEATER_PORT,	GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, RELAY_HEATER);
	gpio_set_mode(RELAY_LOCK_PORT,		GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, RELAY_LOCK);

	RELAY_MOTOR_CCW_OFF;
	RELAY_MOTOR_CW_OFF;
	RELAY_VALVE_WASH_OFF;
	RELAY_VALVE_PREWASH_OFF;
	RELAY_DRAIN_OFF;
	RELAY_HEATER_OFF;
	RELAY_LOCK_OFF;

}

void buttons_setup(void)
{

	gpio_set_mode(BUTTON_UP_PORT,     GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, BUTTON_UP_GPIO);
	gpio_set_mode(BUTTON_DOWN_PORT,   GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, BUTTON_DOWN_GPIO);
	gpio_set_mode(BUTTON_LEFT_PORT,   GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, BUTTON_LEFT_GPIO);
	gpio_set_mode(BUTTON_RIGHT_PORT,  GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, BUTTON_RIGHT_GPIO);
	gpio_set_mode(BUTTON_MIDDLE_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, BUTTON_MIDDLE_GPIO);

}


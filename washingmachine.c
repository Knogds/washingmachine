#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/timer.h>
#include <stdint.h>
#include "ili9340.h"
#include "washingmachine.h"
#include "xprintf.h"

/*
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/spi.h>
*/

#define asm_nop __asm__("nop")



#define MOTOR_SPINUP_TIMER 200
#define FALSE 0
#define TRUE 1
#define TEMPERATURE_OFF 0
#define TEMPERATURE_60C	940
#define TEMPERATURE_90C	1800


uint16_t heater_desired_temperature;
uint16_t heater_temperature;
uint16_t motor_rpm_average = 0;
uint16_t motor_rpm_average_old = 0;
uint16_t motor_spin_timer = 0;
uint16_t systick_every_five_seconds = 0;
uint16_t systick_every_half_a_second = 0;
uint16_t systick_every_hundredth_of_a_second = 0;
uint16_t systick_every_second = 0;
uint16_t systick_every_third_of_a_second = 0;
uint16_t systick_every_tenth_of_a_second = 0;
uint32_t systick_every_minute = 0;
uint32_t systick_every_thirty_seconds = 0;
uint32_t mapvalue(uint32_t num, uint32_t in_low, uint32_t in_high, uint32_t out_low, uint32_t out_high);
uint8_t motor_power = 21;
uint8_t systick_every_milisecond = 0;
uint8_t systick_poll_buttons_blocked = 0;
uint8_t systick_sleep_seconds_timer_active = 0;
volatile uint32_t systick_minutes_timer = 0;
volatile uint32_t systick_seconds_timer = 0;
volatile uint32_t systick_tenth_of_seconds_timer = 0;
volatile uint8_t systick_motor_busy = 0;

uint16_t rpm_global = 0;
int16_t motor_accel = 0;

uint32_t mapvalue(uint32_t value, uint32_t in_low, uint32_t in_high, uint32_t out_low, uint32_t out_high)
{
	  return (value - in_low) * (out_high - out_low) / (in_high - in_low) + out_low;
}


void sleep_seconds(uint32_t seconds)
{
	systick_sleep_seconds(seconds);
}

void PRIVREMENO_sleep_seconds(uint32_t seconds)
{

	uint32_t i;

	while(seconds){

		ili9340_setCursorXY(10,10);
		xprintf("seconds: %03d", seconds);

		for(i=0; i<10000000; i++){
			__asm__("nop");
		}

		seconds--;

	}
}

void heater_set_temperature(uint16_t temperature)
{
	switch(temperature)
	{
		case (TEMPERATURE_60C):
			heater_desired_temperature = TEMPERATURE_60C;
		break;

		case (TEMPERATURE_90C):
			heater_desired_temperature = TEMPERATURE_90C;
		break;

		case (TEMPERATURE_OFF):
			heater_desired_temperature = TEMPERATURE_OFF;
		break;
	}
	
}

void heater_control_temperature(void)
{
	if (heater_desired_temperature == TEMPERATURE_OFF)
	{
		RELAY_HEATER_OFF;
	}

	if (heater_desired_temperature == TEMPERATURE_60C)
	{
		if (ADC_HEATER_SENSOR_VALUE < TEMPERATURE_60C)
		{
			RELAY_HEATER_ON;
		} else {
			RELAY_HEATER_OFF;
		}
	}

	if (heater_desired_temperature == TEMPERATURE_90C)
	{
		if (ADC_HEATER_SENSOR_VALUE < TEMPERATURE_90C)
		{
			RELAY_HEATER_ON;
		} else {
			RELAY_HEATER_OFF;
		}
	}
}

void mset_rpm (uint16_t rpm, uint16_t seconds)
{
	RELAY_LOCK_ON;
	xprintf("%d",seconds);
	while(systick_motor_busy) __asm__("nop");
}

void sys_tick_handler(void)
{
	/* very aproximate timings because of
	many actions in sys_tick_handler.
	use RTC for more precise timings */ 
	systick_every_minute++;
	systick_every_thirty_seconds++;
	systick_every_five_seconds++;
	systick_every_second++;
	systick_every_half_a_second++;
	systick_every_third_of_a_second++;
	systick_every_tenth_of_a_second++;
	systick_every_hundredth_of_a_second++;
	systick_every_milisecond++;

        if (systick_every_minute == 600000) {
		if (systick_minutes_timer > 0) systick_minutes_timer--;
		systick_every_minute = 0;
	}

        if (systick_every_thirty_seconds == 300000) {
		systick_every_thirty_seconds = 0;
	}

        if (systick_every_five_seconds == 50000) {


		systick_every_five_seconds = 0;
	}

        if (systick_every_second == 10000) {
		if (systick_seconds_timer > 0) systick_seconds_timer--;
		systick_every_second = 0;
        }

        if (systick_every_half_a_second == 5000) {
		systick_every_half_a_second = 0;
	}

        if (systick_every_third_of_a_second == 3333) {

		static uint16_t adc_sensor_motor_rpm_old ;
		motor_accel = ADC_SENSOR_MOTOR_RPM - adc_sensor_motor_rpm_old;
		adc_sensor_motor_rpm_old = ADC_SENSOR_MOTOR_RPM;

		systick_every_third_of_a_second = 0;
	}

        if (systick_every_tenth_of_a_second == 1000) {

		if (systick_tenth_of_seconds_timer > 0) systick_tenth_of_seconds_timer--;
		if (systick_poll_buttons_blocked > 0) systick_poll_buttons_blocked--;

		ili9340_setCursorXY(5,5);
		xprintf("r:%d T:%d rpm:%03d tem:%03d acc:%3d",
				rpm_global, TRIAC, ADC_SENSOR_MOTOR_RPM,
				ADC_HEATER_SENSOR_VALUE, motor_accel);

		display_update_systick();
		motor_rpm_average_update();
		motor_stabilize_rpm(rpm_global);


		systick_every_tenth_of_a_second = 0;
	}

        if (systick_every_hundredth_of_a_second == 100) {

		if (!systick_poll_buttons_blocked)
		{
			buttons_scan();
		}


		systick_every_hundredth_of_a_second = 0;
	}

        if (systick_every_milisecond == 10) {
		systick_every_milisecond = 0;
        }

}

void systick_poll_buttons_block(void)
{
	/* in tenth of seconds - amount of time to block key polling */
	systick_poll_buttons_blocked = 3;
}

void systick_sleep_tenth_of_seconds(uint32_t sleep_units)
{
	systick_tenth_of_seconds_timer = sleep_units;
}

void systick_sleep_seconds(uint32_t sleep_units)
{
	systick_seconds_timer = sleep_units;
}

void systick_sleep_minutes(uint32_t sleep_units)
{
	systick_minutes_timer = sleep_units;
}


void motor_power_plus_1percent(void)
{
	/* limit max power to 50%... */
	if (motor_power < 50){
		motor_power++;
		motor_set_power_percent(motor_power);
	}
}

void motor_power_minus_1percent(void)
{
	if (motor_power > 0){
		motor_power--;
		motor_set_power_percent(motor_power);
	}
}

void motor_stabilize_rpm(uint16_t rpm)
{
	static uint8_t motor_start = 1,
		       rpm_changed = 0;

	static uint16_t pause = 10,
			triacmax = 50000,
			triacmin = 42000,
			triac_old_value,
			rpm_old_value;


	

	if(pause>0){

		pause--;

	} else {

		/* check if motor relays are turned on) */
		if(GPIO_IDR(RELAY_MOTOR_PORT) & (RELAY_MOTOR_CCW | RELAY_MOTOR_CW)){

	/************************************************/

		if((rpm > 0) && (rpm < 500)){

			static uint16_t power;

			if(ADC_SENSOR_MOTOR_RPM <= 50) power=20;
			if(ADC_SENSOR_MOTOR_RPM <= 4)  power=40;

			/*
			if(motor_accel > 0)power = 500;
			if(motor_accel > 1)power = 100;
			if(motor_accel > 2)power = 80;
			if(motor_accel > 5)power = 70;
			if(motor_accel > 10)power = 50;
			if(motor_accel > 20)power = 30;
			if(motor_accel > 30)power = 10;

			if(motor_accel < 0)power = 500;
			if(motor_accel < -1)power = 100;
			if(motor_accel < -2)power = 80;
			if(motor_accel < -5)power = 70;
			if(motor_accel < -10)power = 50;
			if(motor_accel < -20)power = 30;
			if(motor_accel < -30)power = 10;

			if(motor_accel>30);pause=2;
			if(motor_accel<-30);pause=2;
			*/


			ili9340_setCursorXY(5,230);xprintf("%04d",power);


			if((TRIAC-power < triacmax) && (TRIAC+power > triacmin)){

				if (ADC_SENSOR_MOTOR_RPM < rpm){

					 TRIAC = TRIAC-=power;
				}

				if (ADC_SENSOR_MOTOR_RPM > rpm){
					pause=10;

					TRIAC = TRIAC+power;
				}
			}
		}
	/************************************************/
		}
	triac_old_value = TRIAC;
	rpm_old_value = ADC_SENSOR_MOTOR_RPM;
	}

}

void motor_rpm_average_update (void)
{
	/*
	https://en.wikipedia.org/wiki/Exponential_smoothing#Basic_exponential_smoothing
	*/
	float alpha=0.7;
	motor_rpm_average_old = motor_rpm_average;
	motor_rpm_average = (alpha * ADC_SENSOR_MOTOR_RPM) + (1 - alpha) * motor_rpm_average_old;
}


void error_stop_all_relays(void)
{
	TRIAC = 59950;
	RELAY_MOTOR_CCW_OFF;
	RELAY_MOTOR_CW_OFF;
	RELAY_HEATER_OFF;
	RELAY_DRAIN_OFF;
	RELAY_LOCK_OFF;
	RELAY_VALVE_PREWASH_OFF;
	RELAY_VALVE_WASH_OFF;
}



static void sleep(uint32_t cycles)
{
	for (uint32_t i = 0; i < cycles; i++) __asm__("nop");
}

static void clock_setup(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_AFIO);

	/*
	Backup domain control register (RCC_BDCR)
	(used for configuring the realtime clock)
	*/

/* RTC */
/*
#define RCC_BDCR_RTCSEL_SELECT_LSE (1<<8)

	RCC_BDCR |= 
		RCC_BDCR_RTCSEL_SELECT_LSE |
		RCC_BDCR_LSEON |
		RCC_BDCR_RTCEN
		;
*/

	/*
	wait for LSE to stabilize
	*/
/*
	while((RCC_BDCR & RCC_BDCR_LSERDY));
*/

}

int main(void)
{

	clock_setup();
	adc_setup();
	buttons_setup();
	ili9340_init();
	ili9340_clearscreen(SCREEN_BACKGROUND_COLOUR);
	ili9340_setRotation(1);
	exti_setup();
	systick_setup();
	relays_setup();
	timer_setup();

	sleep(1);

	/*
	CW is the only "correct"
	side to spin the motor when
	using higher RPM values
	*/

	while(1){
	}

	return 0;
} 

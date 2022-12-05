#ifndef FILE_WASHINGMACHINE_H
#define FILE_WASHINGMACHINE_H

#define TRIAC TIM1_CCR2
#define ADC_SENSOR_MOTOR_RPM ADC1_JDR2
#define ADC_HEATER_SENSOR_VALUE ADC1_JDR1

#define BUTTON_UP_GPIO		GPIO4
#define BUTTON_UP_PORT		GPIOA
#define BUTTON_LEFT_GPIO	GPIO6
#define BUTTON_LEFT_PORT	GPIOA
#define BUTTON_MIDDLE_GPIO	GPIO2
#define BUTTON_MIDDLE_PORT	GPIOA
#define BUTTON_RIGHT_GPIO	GPIO15
#define BUTTON_RIGHT_PORT	GPIOB
#define BUTTON_DOWN_GPIO	GPIO3
#define BUTTON_DOWN_PORT	GPIOA

#define BUTTON_UP_PORT_IDR	GPIOA_IDR
#define BUTTON_DOWN_PORT_IDR	GPIOA_IDR
#define BUTTON_LEFT_PORT_IDR	GPIOA_IDR
#define BUTTON_MIDDLE_PORT_IDR	GPIOA_IDR
#define BUTTON_RIGHT_PORT_IDR	GPIOB_IDR

#define BUTTON_UP_ACTION	BUTTON_UP_PORT_IDR & BUTTON_UP_GPIO
#define BUTTON_LEFT_ACTION	BUTTON_LEFT_PORT_IDR & BUTTON_LEFT_GPIO
#define BUTTON_MIDDLE_ACTION	BUTTON_MIDDLE_PORT_IDR & BUTTON_MIDDLE_GPIO
#define BUTTON_RIGHT_ACTION	BUTTON_RIGHT_PORT_IDR & BUTTON_RIGHT_GPIO
#define BUTTON_DOWN_ACTION	BUTTON_DOWN_PORT_IDR & BUTTON_DOWN_GPIO

typedef struct program
{
	char*	subprogram_description;
	uint8_t		subprogram_type;
	uint16_t	data_0;
	uint16_t	data_1;	

} program;

extern program fortvatt_huvudtvatt[];
extern program huvudtvatt[];

extern uint16_t triac_power[100];
extern uint16_t motor_rpm_average;
extern uint16_t rpm_global;
extern uint8_t motor_power;

enum
{

	PROGRAM_ADD_WATER,
	PROGRAM_FORTVATT_HUVUDTVATT,
	PROGRAM_HUVUDTVATT,
	PROGRAM_REMOVE_WATER_HIGH_SPEED,
	PROGRAM_REMOVE_WATER_LOW_SPEED_LONG,
	PROGRAM_REMOVE_WATER_LOW_SPEED_SHORT,
	PROGRAM_WASH_WITHOUT_WASHING_POWDER,
	PROGRAM_WASH_WITH_WASHING_POWDER,

	VALVE_BOTH,
	VALVE_PREWASH,
	VALVE_WASH,

	RPM_AVERAGE,
	RPM_DIRECT,	

	CW,
	CCW
};

#define RELAY_ON gpio_clear
#define RELAY_OFF gpio_set 

/* MOTOR */
#define RELAY_MOTOR_PORT GPIOB
#define RELAY_MOTOR_CCW GPIO12 /* relay module IN7 */
#define RELAY_MOTOR_CW GPIO13 /* relay module IN8 */
#define RELAY_MOTOR_CCW_ON RELAY_ON(RELAY_MOTOR_PORT, RELAY_MOTOR_CCW)
#define RELAY_MOTOR_CCW_OFF RELAY_OFF(RELAY_MOTOR_PORT, RELAY_MOTOR_CCW)
#define RELAY_MOTOR_CW_ON RELAY_ON(RELAY_MOTOR_PORT, RELAY_MOTOR_CW)
#define RELAY_MOTOR_CW_OFF RELAY_OFF(RELAY_MOTOR_PORT, RELAY_MOTOR_CW)

/* WASH */
#define RELAY_VALVE_WASH_PORT GPIOA
#define RELAY_VALVE_WASH GPIO12 /* relay module IN6 */
#define RELAY_VALVE_WASH_ON RELAY_ON(RELAY_VALVE_WASH_PORT, RELAY_VALVE_WASH)
#define RELAY_VALVE_WASH_OFF RELAY_OFF(RELAY_VALVE_WASH_PORT, RELAY_VALVE_WASH)

/* PREWASH */
#define RELAY_VALVE_PREWASH_PORT GPIOB
#define RELAY_VALVE_PREWASH GPIO7 /* relay module IN5 */
#define RELAY_VALVE_PREWASH_ON RELAY_ON(RELAY_VALVE_PREWASH_PORT, RELAY_VALVE_PREWASH)
#define RELAY_VALVE_PREWASH_OFF RELAY_OFF(RELAY_VALVE_PREWASH_PORT, RELAY_VALVE_PREWASH)

/* DRAIN */
#define RELAY_DRAIN_PORT GPIOA
#define RELAY_DRAIN GPIO11 /* relay module IN4 */
#define RELAY_DRAIN_ON RELAY_ON(RELAY_DRAIN_PORT, RELAY_DRAIN)
#define RELAY_DRAIN_OFF RELAY_OFF(RELAY_DRAIN_PORT, RELAY_DRAIN)

/* HEATER */
#define RELAY_HEATER_PORT GPIOA
#define RELAY_HEATER GPIO10 /* relay module IN3 */
#define RELAY_HEATER_ON RELAY_ON(RELAY_HEATER_PORT, RELAY_HEATER)
#define RELAY_HEATER_OFF RELAY_OFF(RELAY_HEATER_PORT, RELAY_HEATER)

/* LOCK */
#define RELAY_LOCK_PORT GPIOB
#define RELAY_LOCK GPIO8 /* relay module IN2 */
#define RELAY_LOCK_ON RELAY_ON(RELAY_LOCK_PORT, RELAY_LOCK)
#define RELAY_LOCK_OFF RELAY_OFF(RELAY_LOCK_PORT, RELAY_LOCK)


#define SCREEN_BACKGROUND_COLOUR	ILI9340_WHITE

void PRIVREMENO_sleep_seconds(uint32_t seconds);
void adc_setup(void);
void buttons_scan(void);
void buttons_setup(void);
void error_stop_all_relays(void);
void error_write_message(const char* message);
void exti_setup(void);
void heater_control_temperature(void);
void heater_set_temperature(uint16_t temperature);
void display_update_systick(void);
void motor_power_minus_1percent(void);
void motor_power_plus_1percent(void);
void motor_rpm_average_update (void);
void motor_set_power_percent(uint8_t percent);
void motor_stabilize_rpm(uint16_t rpm);
void mset_rpm(uint16_t rpm, uint16_t seconds);
void mspindown(void);
void mspinup(uint8_t cw_or_ccw);
void program_remove_water_low_speed(void);
void program_run(program programname[]);
void program_skoljning(void);
void relays_setup(void);
void sleep_seconds(uint32_t seconds);
void systick_motor_run(uint16_t seconds);
void systick_motor_spindown(void);
void systick_motor_spinup(void);
void systick_poll_buttons_block(void);
void systick_setup(void);
void systick_sleep_minutes(uint32_t sleep_units);
void systick_sleep_seconds(uint32_t sleep_units);
void systick_sleep_tenth_of_seconds(uint32_t sleep_units);
void timer_setup(void);

#endif /* !FILE_WASHINGMACHINE_H */

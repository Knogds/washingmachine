#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/timer.h>
#include <stdint.h>
#include "ili9340.h"
#include "washingmachine.h"
#include "xprintf.h"

uint8_t menu_selected_item = 0;
uint8_t display_refresh_needed = 1;

enum
{
	/* menus */
	MENU_DEBUG_ADC,
	MENU_DEBUG_INTERRUPT_STATUS,
	MENU_DEBUG_PRINT_MOTOR_AND_SENSOR_STATUS,
	MENU_DEBUG_RTC,
	MENU_ERROR,
	MENU_FINISHED,
	MENU_MAIN_MENU,
	MENU_PROGRAM_FORTVATT_HUVUDTVATT,
	MENU_PROGRAM_HUVUDTVATT,
	MENU_PROGRAM_LANGSAM_CENTRIFUGERING,
	MENU_PROGRAM_SELECTION,
	MENU_PROGRAM_SKOLJNING,
	MENU_SELECT_NEXT_ITEM,
	MENU_SELECT_PREV_ITEM,
	MENU_TEST_THE_MOTOR,
	MENU_TEST_THE_MOTOR_CW,
	MENU_TEST_THE_MOTOR_CCW,
	MENU_TEST_THE_MOTOR_OFF,
	MENU_PREVIOUS,
	MENU_BUTTON_UP_ACTION,
	MENU_BUTTON_DOWN_ACTION,
	MENU_TEST_THE_MOTOR_RPM_MORE,
	MENU_TEST_THE_MOTOR_RPM_LESS,
	MENU_TRIAC_MORE,
	MENU_TRIAC_LESS,
	MENU_PREWASH_RELAY_ON,
	MENU_PREWASH_RELAY_OFF,
	MENU_DRAINPUMP_ON,
	MENU_DRAINPUMP_OFF,
	MENU_WASH_RELAY_ON,
	MENU_WASH_RELAY_OFF,
	MENU_CHOOSE_ITEM
};

typedef struct menu_items
{
	const char* menu_item_text;
	uint16_t posX;
	uint8_t posY;
	uint8_t menu_id;	

} menu_items;

void debug_print_adc_registers(void);
void debug_print_rtc_registers(void);
void menu_clearscreen(void);
void menu_current_activate(menu_items menu[]);
void menu_draw_cursor(char* cursor);
void menu_print_adc_registers(void);
void menu_print_rtc_registers(void);
void menu_render(menu_items menu[]);
void menu_select(uint8_t action);
void menu_test_the_motor(void);
void menu_test_the_motor_ccw(void);
void menu_test_the_motor_cw(void);

menu_items main_menu[] =
{
	/* "what to print", posX, posY, ID */
	{ "Valj tvattprogram",		20, 30, MENU_PROGRAM_SELECTION},
	{ "Testa motorn",		20, 40, MENU_TEST_THE_MOTOR},
	{ "Show motor&sensor status",	20, 50, MENU_DEBUG_PRINT_MOTOR_AND_SENSOR_STATUS },
	{ "Show interrupt debug",	20, 60, MENU_DEBUG_INTERRUPT_STATUS},
	{ "Show ADC registers",		20, 70, MENU_DEBUG_ADC },
	{ "Show RTC registers",		20, 80, MENU_DEBUG_RTC },
	{ 0, 0, 0, 0 }
};

menu_items test_the_hardware[] =
{
	{ "Turn on the motor CW",	20, 30, MENU_TEST_THE_MOTOR_CW},
	{ "Turn on the motor CCW",	20, 40, MENU_TEST_THE_MOTOR_CCW},
	{ "Turn off the motor ralays",	20, 50, MENU_TEST_THE_MOTOR_OFF},
	{ "Motor RPM +",		20, 60, MENU_TEST_THE_MOTOR_RPM_MORE},
	{ "Motor RPM -",		20, 70, MENU_TEST_THE_MOTOR_RPM_LESS},
	{ "TRIAC +",			20, 80, MENU_TRIAC_MORE},
	{ "TRIAC -",			20, 90, MENU_TRIAC_LESS},
	{ "Prewash relay on",		20, 100, MENU_PREWASH_RELAY_ON},
	{ "Prewash relay off",		20, 110, MENU_PREWASH_RELAY_OFF},
	{ "Wash relay on",		20, 120, MENU_WASH_RELAY_ON},
	{ "Wash relay off",		20, 130, MENU_WASH_RELAY_OFF},
	{ "Drain pump on",		20, 140, MENU_DRAINPUMP_ON},
	{ "Drain pump off",		20, 150, MENU_DRAINPUMP_OFF},
	{ 0, 0, 0, 0 }
};

menu_items program_menu[] =
{
	{ "Program huvudtvatt",			20, 30, MENU_PROGRAM_HUVUDTVATT},
	{ "Program fortvatt och huvudtvatt",	20, 40, MENU_PROGRAM_FORTVATT_HUVUDTVATT},
	{ "Langsam centrifugering",		20, 50, MENU_PROGRAM_LANGSAM_CENTRIFUGERING},
	{ "Skoljning",				20, 60, MENU_PROGRAM_SKOLJNING},
	{ 0, 0, 0, 0 }
};

menu_items *menu_previous,*menu_current=&main_menu[0];


void menu_draw_cursor(char* cursor)
{
	uint16_t X;
	uint8_t Y;
	/* draw a cursor at the selected menu item */
	X = menu_current[menu_selected_item].posX;
	Y = menu_current[menu_selected_item].posY;
	ili9340_setCursorXY((X-10), Y);
	xprintf(cursor);
}

/* draws the whole menu line by line,
   from top to bottom */

void menu_render(menu_items menu[])
{
	uint8_t ptr=0;
	while (menu[ptr].menu_item_text != 0){
		ili9340_setCursorXY(menu[ptr].posX, menu[ptr].posY);
		xprintf("%s", menu[ptr].menu_item_text);
		ptr++;
	}
	menu_draw_cursor(">");
}

/* called by systick interrupt
   10x per second */

void display_update_systick(void)
{
	if(display_refresh_needed){
		menu_render(menu_current);
		display_refresh_needed = 0;
	}
}

/* point the menu_current
   to the right structure
   and clear the screen */

void menu_current_activate(menu_items menu[])
{
		menu_selected_item = 0;
		menu_previous = menu_current;
		menu_current = menu;
		menu_clearscreen();

}


/* select the menu item (menu_selected_item)
   and decide what to do when buttons are pressed */

void menu_select(uint8_t action)
{
	switch(action){
		/* LEFT is pressed */
		case MENU_PREVIOUS:
			menu_selected_item = 0;
			menu_current = menu_previous;
			menu_clearscreen();
			break;

		/* UP is pressed */
		case MENU_BUTTON_UP_ACTION:
			if (menu_selected_item > 0)
			{
				
				menu_draw_cursor(" "); /* erase the old cursor */
				menu_selected_item -= 1;
			} else {
				int i = 0;
				while((menu_current[i+1].menu_item_text) != 0)
				{
					i++;
				}
				menu_draw_cursor(" ");
				menu_selected_item = i; 
			}
		break;

		/* DOWN is pressed */
		case MENU_BUTTON_DOWN_ACTION:
			if ((menu_current[menu_selected_item + 1].menu_item_text) != 0)
			{
				menu_draw_cursor(" ");
				menu_selected_item += 1;
			} else {
				menu_draw_cursor(" ");
				menu_selected_item = 0;
			}
		break;

		/* MIDDLE is pressed */
		case MENU_CHOOSE_ITEM:
		    switch(menu_current[menu_selected_item].menu_id){

/*************************************************************************/
/*********************** program selection *******************************/
			case MENU_PROGRAM_SELECTION:
				menu_current_activate(program_menu);
				break;

/*************************************************************************/
/*********************** hardware testing ********************************/
			case MENU_TEST_THE_MOTOR:
				menu_current_activate(test_the_hardware);
				break;

			case MENU_TEST_THE_MOTOR_CW:
				TRIAC=49000;
				RELAY_MOTOR_CW_OFF;
				RELAY_MOTOR_CCW_OFF;
				RELAY_MOTOR_CW_ON;
				break;

			case MENU_TEST_THE_MOTOR_CCW:
				TRIAC=49000;
				RELAY_MOTOR_CW_OFF;
				RELAY_MOTOR_CCW_OFF;
				RELAY_MOTOR_CCW_ON;
				break;

			case MENU_TEST_THE_MOTOR_OFF:
				motor_set_power_percent(0);
				RELAY_MOTOR_CW_OFF;
				RELAY_MOTOR_CCW_OFF;
				break;

			case MENU_TEST_THE_MOTOR_RPM_MORE:
				if(rpm_global<=1000) rpm_global = rpm_global+10;
				break;

			case MENU_TEST_THE_MOTOR_RPM_LESS:
				if(rpm_global>=10) rpm_global = rpm_global-10;
				break;
				

			case MENU_TRIAC_MORE:
				motor_power_plus_1percent();
				break;

			case MENU_TRIAC_LESS:
				motor_power_minus_1percent();
				break;

			case MENU_PREWASH_RELAY_ON:
				RELAY_VALVE_PREWASH_ON;
				break;

			case MENU_PREWASH_RELAY_OFF:
				RELAY_VALVE_PREWASH_OFF;
				break;

			case MENU_WASH_RELAY_ON:
				RELAY_VALVE_WASH_ON;
				break;

			case MENU_WASH_RELAY_OFF:
				RELAY_VALVE_WASH_OFF;
				break;

			case MENU_DRAINPUMP_ON:
				RELAY_DRAIN_ON;
				break;

			case MENU_DRAINPUMP_OFF:
				RELAY_DRAIN_OFF;
				break;

/*************************************************************************/
/*********************** debugging stuff *********************************/
			case MENU_DEBUG_ADC: 
				menu_print_adc_registers();
				break;
			case MENU_DEBUG_RTC: 
				menu_print_rtc_registers();
				break;
		    }
	}

}

void error_write_message(const char* message)
{
	
	error_stop_all_relays();
	ili9340_drawRect(0,0, 320, 240, 0x000, ILI9340_RED);
	ili9340_setCursorXY(20, 117);
	xprintf(message);
	PRIVREMENO_sleep_seconds(10);
}

void buttons_scan(void)
{

	if (BUTTON_UP_ACTION){
		menu_select(MENU_BUTTON_UP_ACTION);
		display_refresh_needed = 1;
		systick_poll_buttons_block();
	}

	if (BUTTON_LEFT_ACTION){
		menu_select(MENU_PREVIOUS);
		display_refresh_needed = 1;
		/* what if both left+right is pressed */
		if (BUTTON_RIGHT_ACTION){
			error_write_message("USER ABORTED.");
		}
		systick_poll_buttons_block();
	}

	if (BUTTON_MIDDLE_ACTION){
		menu_select(MENU_CHOOSE_ITEM);
		display_refresh_needed = 1;
		systick_poll_buttons_block();
	}

	if (BUTTON_RIGHT_ACTION){
		display_refresh_needed = 1;
		systick_poll_buttons_block();
	}

	if (BUTTON_DOWN_ACTION){
		menu_select(MENU_BUTTON_DOWN_ACTION);
		display_refresh_needed = 1;
		systick_poll_buttons_block();
	}
}

void menu_print_adc_registers(void)
{
	menu_clearscreen();
	while(!(BUTTON_LEFT_ACTION)){
		debug_print_adc_registers();
		while(BUTTON_MIDDLE_ACTION){
			__asm__("nop");
		}
		if(BUTTON_UP_ACTION){
			int i=5000000;
			while(i){
			__asm__("nop");i--;
			}
		}
	}		
	menu_clearscreen();
}

void menu_clearscreen(void)
{
	ili9340_fillRect(0, 0, ILI9340_TFTHEIGHT, ILI9340_TFTWIDTH, ILI9340_WHITE);
}

void debug_print_adc_registers(void)
{
	ili9340_setCursorXY(5,5) ;
	xprintf("ADC_SR:    %032b", ADC_SR(ADC1));
	ili9340_setCursorXY(5,15) ;
	xprintf("ADC_CR1:   %032b", ADC_CR1(ADC1));
	ili9340_setCursorXY(5,25) ;
	xprintf("ADC_CR2:   %032b", ADC_CR2(ADC1));
	ili9340_setCursorXY(5,35) ;
	xprintf("ADC_SMPR1: %032b", ADC_SMPR1(ADC1));
	ili9340_setCursorXY(5,45) ;
	xprintf("ADC_SMPR2: %032b", ADC_SMPR2(ADC1));
	ili9340_setCursorXY(5,55) ;
	xprintf("ADC_JOFR1: %032b", ADC_JOFR1(ADC1));
	ili9340_setCursorXY(5,65) ;
	xprintf("ADC_JOFR2: %032b", ADC_JOFR2(ADC1));
	ili9340_setCursorXY(5,75) ;
	xprintf("ADC_JOFR3: %032b", ADC_JOFR3(ADC1));
	ili9340_setCursorXY(5,85) ;
	xprintf("ADC_JOFR4: %032b", ADC_JOFR4(ADC1));
	ili9340_setCursorXY(5,95) ;
	xprintf("ADC_HTR:   %032b", ADC_HTR(ADC1));
	ili9340_setCursorXY(5,105) ;
	xprintf("ADC_LTR:   %032b", ADC_LTR(ADC1));
	ili9340_setCursorXY(5,115) ;
	xprintf("ADC_SQR1:  %032b", ADC_SQR1(ADC1));
	ili9340_setCursorXY(5,125) ;
	xprintf("ADC_SQR2:  %032b", ADC_SQR2(ADC1));
	ili9340_setCursorXY(5,135) ;
	xprintf("ADC_SQR3:  %032b", ADC_SQR3(ADC1));
	ili9340_setCursorXY(5,145) ;
	xprintf("ADC_JSQR:  %032b", ADC_JSQR(ADC1));
	ili9340_setCursorXY(5,155) ;
	xprintf("ADC_JDR1:  %04d", ADC_JDR1(ADC1));
	ili9340_setCursorXY(5,165) ;
	xprintf("ADC_JDR2:  %04d", ADC_JDR2(ADC1));
	ili9340_setCursorXY(5,175) ;
	xprintf("ADC_JDR3:  %04d", ADC_JDR3(ADC1));
	ili9340_setCursorXY(5,185) ;
	xprintf("ADC_JDR4:  %04d", ADC_JDR4(ADC1));
	ili9340_setCursorXY(5,195) ;
	xprintf("ADC_DR:    %04d", ADC_DR(ADC1));
}

void menu_print_rtc_registers(void)
{
	menu_clearscreen();
	while(!(BUTTON_LEFT_ACTION)){
		debug_print_rtc_registers();
		while(BUTTON_MIDDLE_ACTION){
			__asm__("nop");
		}
		if(BUTTON_UP_ACTION){
			int i=5000000;
			while(i){
			__asm__("nop");i--;
			}
		}
	}		
	menu_clearscreen();
}

void debug_print_rtc_registers(void)
{
	ili9340_setCursorXY(5,5) ;
	xprintf("RTC_CRH:   %032b", RTC_CRH);
	ili9340_setCursorXY(5,15) ;
	xprintf("RTC_CRL:   %032b", RTC_CRL);
	ili9340_setCursorXY(5,25) ;
	xprintf("RTC_PRLH:  %032b", RTC_PRLH);
	ili9340_setCursorXY(5,35) ;
	xprintf("RTC_PRLL:  %032b", RTC_PRLL);
	ili9340_setCursorXY(5,45) ;
	xprintf("RTC_DIVH:  %032b", RTC_DIVH);
	ili9340_setCursorXY(5,55) ;
	xprintf("RTC_DIVL:  %032b", RTC_DIVL);
	ili9340_setCursorXY(5,65) ;
	xprintf("RTC_CNTH:  %032b", RTC_CNTH);
	ili9340_setCursorXY(5,75) ;
	xprintf("RTC_CNTL:  %032b", RTC_CNTL);
	ili9340_setCursorXY(5,85) ;
	xprintf("RTC_ALRH:  %032b", RTC_ALRH);
	ili9340_setCursorXY(5,95) ;
	xprintf("RTC_ALRL:  %032b", RTC_ALRL);
}

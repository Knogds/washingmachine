#include <stdint.h>
#include "ili9340.h"
#include "washingmachine.h"
#include "xprintf.h"

void program_add_water(uint8_t which_valve);
void program_remove_water_high_speed(void);
void program_remove_water_low_speed_long(void);
void program_wash_with_washing_powder(void);
void program_wash_without_washing_powder(void);

program fortvatt_huvudtvatt[] =
{
	{ "Tar vatten 1",		PROGRAM_ADD_WATER, VALVE_PREWASH, 0 },
	{ "Tvatt 1 + tvattpulver",	PROGRAM_WASH_WITHOUT_WASHING_POWDER, 0, 0 },
	{ "Skoljning 1",		PROGRAM_REMOVE_WATER_LOW_SPEED_SHORT, 0, 0 },

	{ "Tar vatten 2 ",		PROGRAM_ADD_WATER, VALVE_WASH, 0 },
	{ "Tvatt 2",			PROGRAM_WASH_WITH_WASHING_POWDER, 0, 0 },
	{ "Skoljning 2",		PROGRAM_REMOVE_WATER_LOW_SPEED_SHORT, 0, 0 },

	{ "Tar vatten 3",		PROGRAM_ADD_WATER, VALVE_WASH, 0 },
	{ "Tvatt 3",			PROGRAM_WASH_WITHOUT_WASHING_POWDER, 0, 0 },
	{ "Skoljning 3",		PROGRAM_REMOVE_WATER_LOW_SPEED_SHORT, 0, 0 },

	{ "Tar vatten 4",		PROGRAM_ADD_WATER, VALVE_WASH, 0 },
	{ "Tvatt 4",			PROGRAM_WASH_WITHOUT_WASHING_POWDER, 0, 0 },
	{ "Skoljning 4",		PROGRAM_REMOVE_WATER_LOW_SPEED_LONG, 0, 0 },

	{ "Tar vatten 5",		PROGRAM_ADD_WATER, VALVE_BOTH, 0 },
	{ "Tvatt 5",			PROGRAM_WASH_WITHOUT_WASHING_POWDER, 0, 0 },
	{ "Skoljning 5 - slut",		PROGRAM_REMOVE_WATER_LOW_SPEED_LONG, 0, 0 },

	{ 0, 0, 0, 0 }
};	

program huvudtvatt[] =
{
	{ "Tar vatten 1",		PROGRAM_ADD_WATER, VALVE_WASH, 0 },
	{ "Tvatt 1 + tvattpulver",	PROGRAM_WASH_WITH_WASHING_POWDER, 0, 0 },
	{ "Skoljning 1",		PROGRAM_REMOVE_WATER_LOW_SPEED_SHORT, 0, 0 },

	{ "Tar vatten 2 ",		PROGRAM_ADD_WATER, VALVE_WASH, 0 },
	{ "Tvatt 2",			PROGRAM_WASH_WITHOUT_WASHING_POWDER, 0, 0 },
	{ "Skoljning 2",		PROGRAM_REMOVE_WATER_LOW_SPEED_SHORT, 0, 0 },

	{ "Tar vatten 3",		PROGRAM_ADD_WATER, VALVE_WASH, 0 },
	{ "Tvatt 3",			PROGRAM_WASH_WITHOUT_WASHING_POWDER, 0, 0 },
	{ "Skoljning 3",		PROGRAM_REMOVE_WATER_LOW_SPEED_SHORT, 0, 0 },

	{ "Tar vatten 4",		PROGRAM_ADD_WATER, VALVE_BOTH, 0 },
	{ "Tvatt 4",			PROGRAM_WASH_WITHOUT_WASHING_POWDER, 0, 0 },
	{ "Skoljning 4 - slut",		PROGRAM_REMOVE_WATER_LOW_SPEED_LONG, 0, 0 },

	{ 0, 0, 0, 0 }
};	

void program_add_water(uint8_t which_valve)
{
	RELAY_LOCK_ON;

	PRIVREMENO_sleep_seconds(5);

	RELAY_DRAIN_ON;
	PRIVREMENO_sleep_seconds(10);
	RELAY_DRAIN_OFF;

	switch(which_valve)
	{
		case VALVE_WASH:
			RELAY_VALVE_WASH_ON;
			break;
		case VALVE_PREWASH:
			RELAY_VALVE_PREWASH_ON;
			break;
		case VALVE_BOTH:
			RELAY_VALVE_PREWASH_ON;
			RELAY_VALVE_WASH_ON;
			break;
	}

	PRIVREMENO_sleep_seconds(30);
	
	uint8_t i;
		ili9340_setCursorXY(20,10);
		xprintf("ponoviti:      ");
	for(i=1; i<=10; i++)
	{
		ili9340_setCursorXY(20,10);
		xprintf("ponoviti:  %02d", i);

		PRIVREMENO_sleep_seconds(5);

		mspinup(CCW);
		mset_rpm(130, 8);
		mspindown();

		PRIVREMENO_sleep_seconds(5);

		mspinup(CW);
		mset_rpm(130, 8);
		mspindown();

	}

	RELAY_VALVE_WASH_OFF;
	RELAY_VALVE_PREWASH_OFF;
	RELAY_LOCK_OFF;
}

void program_wash_with_washing_powder(void)
{
	RELAY_VALVE_WASH_ON;
	uint8_t i;
		ili9340_setCursorXY(20,10);
		xprintf("ponoviti:      ");
	for(i=30; i>0; i--)
	{
		ili9340_setCursorXY(20,10);
		xprintf("ponoviti:  %02d", i);

		PRIVREMENO_sleep_seconds(10);

		mspinup(CCW);
		mset_rpm(130, 8);
		mspindown();

		PRIVREMENO_sleep_seconds(10);

		mspinup(CW);
		mset_rpm(130, 8);
		mspindown();
	}
	RELAY_VALVE_WASH_OFF;
}

void program_wash_without_washing_powder(void)
{
	uint8_t i=0;
		ili9340_setCursorXY(20,10);
		xprintf("ponoviti:      ");
	for(i=7; i>0; i--)
	{

		ili9340_setCursorXY(20,10);
		xprintf("ponoviti:  %02d", i);

		PRIVREMENO_sleep_seconds(10);

		mspinup(CCW);
		mset_rpm(130, 8);
		mspindown();

		PRIVREMENO_sleep_seconds(10);

		mspinup(CW);
		mset_rpm(130, 8);
		mspindown();
	}
}

void program_remove_water_low_speed(void)
{
		/*
		mspinup(CCW);
		mset_rpm(100, 8);
		mspindown();
		*/
		PRIVREMENO_sleep_seconds(4);
		mspinup(CW);
	RELAY_DRAIN_ON;
		mset_rpm(150, 10);
		mset_rpm(250, 30);
		mset_rpm(500, 5);
		mset_rpm(200, 5);
		mset_rpm(500, 5);
		mset_rpm(200, 5);
		mset_rpm(500, 60);
		mset_rpm(600, 30);
	RELAY_DRAIN_OFF;
		mspindown();
}

void program_remove_water_low_speed_long(void)
{
		/*
		mspinup(CCW);
		mset_rpm(100, 8);
		mspindown();
		*/
	RELAY_DRAIN_ON;
		PRIVREMENO_sleep_seconds(4);
		mspinup(CW);
		mset_rpm(150, 20);
		mset_rpm(200, 60);
		mset_rpm(500, 60);
		mset_rpm(600, 60);
	RELAY_DRAIN_OFF;
		mspindown();
}

void program_remove_water_high_speed(void)
{

		mspinup(CW);
		mset_rpm(130, 3);
		mspindown();

		PRIVREMENO_sleep_seconds(2);

		mspinup(CCW);
		mset_rpm(130, 3);
		mspindown();

		PRIVREMENO_sleep_seconds(2);
		mspinup(CW);
		mset_rpm(130, 2);
	RELAY_DRAIN_ON;
		mset_rpm(130, 8);
		mset_rpm(200, 20);
		mset_rpm(250, 20);
		mset_rpm(500, 30);
		mset_rpm(1000, 30);
		mset_rpm(1100, 20);
		mset_rpm(1200, 20);
		mset_rpm(1400, 20);
		mset_rpm(1600, 20);
	RELAY_DRAIN_OFF;
		mspindown();
		PRIVREMENO_sleep_seconds(30);
		mspinup(CCW);
		mset_rpm(130, 4);
		mspindown();
}

void program_run(program programname[])
{
	
	uint8_t posY = 20;
	static uint8_t program_counter = 0;
	
	ili9340_clearscreen(SCREEN_BACKGROUND_COLOUR);
	
	program* prg; 
	prg = &programname[program_counter];

	ili9340_setCursorXY(20,posY);
	xprintf("descr: %s", prg->subprogram_description);

	/*
	char*	subprogram_description;
	uint8_t		subprogram_type;
	uint16_t	data_0;
	uint16_t	data_1;	
	*/

	switch(prg->subprogram_type){

		case PROGRAM_ADD_WATER:
			program_add_water(prg->data_0);
		break;	
		case PROGRAM_WASH_WITH_WASHING_POWDER:
			program_wash_with_washing_powder();
		break;	
		case PROGRAM_WASH_WITHOUT_WASHING_POWDER:
			program_wash_without_washing_powder();
		break;	
		case PROGRAM_REMOVE_WATER_LOW_SPEED_LONG:
			program_remove_water_low_speed_long();
		break;	
		case PROGRAM_REMOVE_WATER_LOW_SPEED_SHORT:
			program_remove_water_low_speed();
		break;	
		case PROGRAM_REMOVE_WATER_HIGH_SPEED:
			program_remove_water_high_speed();
		break;	
	}
	systick_sleep_seconds(19);
}

void program_skoljning(void)
{
		program_add_water(VALVE_WASH);
		program_wash_without_washing_powder();
		program_remove_water_low_speed();

		program_add_water(VALVE_WASH);
		program_wash_without_washing_powder();
		program_remove_water_low_speed();

		program_add_water(VALVE_WASH);
		program_wash_without_washing_powder();
		program_remove_water_low_speed_long();

		error_write_message("KLAR!!!");
}


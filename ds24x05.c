#include "ds24x05.h"

struct ds2405
{
    unsigned char* rom;
	unsigned char state;
};

volatile struct ds2405 switches[SWITCH_COUNT];
volatile unsigned char index = 0;

char find_index(unsigned char* value);

char DS24x05_Toggle(unsigned char* rom)
{
	if (!OW_Reset()) return 0;
	OW_MatchROM(rom);

	char i = find_index(rom);
	if (i != -1){
		if (switches[i].state == false){
			switches[i].state = true;
		}else{
			switches[i].state = false;
		}
	}

	return 1;
}

void DS24x05_Add(unsigned char* rom){
	switches[index].rom = rom;
	switches[index].state = false;

	index++;
}


unsigned char DS24x05_GetState(unsigned char* rom){
	char i = find_index(rom);
	if (i != -1){
		return switches[i].state;
	}
	return false;
}


char find_index(unsigned char* value){
   unsigned char i;
   for (i=0; i<SWITCH_COUNT; i++)
   {
	 if (switches[i].rom == value)
	 {
	    return i;
	 }
   }
   return(-1);
}





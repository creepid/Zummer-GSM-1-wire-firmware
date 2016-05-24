#include "mt8870.h"
#include <avr/io.h>

void MT8870_Init(void){
	MT_DDR &=~0x0F;
	MT_PORT |=0x00;

	STD_DDR &= ~(1 << STD_DDD);     
    STD_PORT |= (1 << STD_PIN_NUMBER); 
}

unsigned char getCode(unsigned char code){
	switch(code){
		case 0x0A: return 0x00;
		case 0x0B: return 0x0E;
		case 0x0C: return 0x0F;
		case 0x0D: return 0x0A;
		case 0x0E: return 0x0B;
		case 0x0F: return 0x0C;
		case 0x00: return 0x0D;
		default: return code;
	}
}

unsigned char MT8870_GetStat(void){
	unsigned char stat;
	stat = MT_PIN & 0x0F;
	return getCode(stat);
}

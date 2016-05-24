#ifndef MT8870_H
#define MT8870_H

#define MT_DDR DDRC
#define MT_PORT PORTC
#define MT_PIN PINC

#define STD_DDD DDD2
#define STD_DDR DDRD
#define STD_PORT PORTD
#define STD_PIN_NUMBER 2

#define SetBit(address,bit) (address|=(1<<bit))
#define ClrBit(address,bit) (address&=~(1<<bit))
#define InvBit(address,bit) (address^=(1<<bit))
#define IsBitOn(address,bit)  (address&(1<<bit))
#define IsBitOff(address,bit) (!(address&(1<<bit)))

void MT8870_Init(void);
unsigned char MT8870_GetStat(void);

#endif

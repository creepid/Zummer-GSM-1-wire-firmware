#include "main.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <util/delay.h>
#include "delay.h"
#include "onewire.h"
#include "ds18x20.h"
#include "ds24x05.h"
#include "mt8870.h"

unsigned char low(unsigned char ch){
	return ch & 0x0F;
}

unsigned char high(unsigned char ch){
	return ch & 0xF0;
}

void USART_init()
{
	// Set baud rate
	UBRRH = 0;
	UBRRL = 51;
	UCSRA = 0;
	// Enable receiver and transmitter
	UCSRB = (1<<TXEN);
	// Set frame format
	UCSRC = (1<<UCSZ1) | (1<<UCSZ0) | (1<<URSEL);
}

void USART0_write(unsigned char data)
{
	while ( !( UCSRA & (1<<UDRE)) ) ;
	UDR = data;
}

FILE usart_str = FDEV_SETUP_STREAM(USART0_write, NULL, _FDEV_SETUP_WRITE); // для функции printf

void print_address(unsigned char* address) {
	printf("%.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X", 
		    address[0],address[1],address[2],address[3],address[4],address[5],address[6],address[7]);
}

volatile unsigned char	commands[COMMANDS_COUNT];

volatile unsigned char	nDevices;	// количество сенсоров
volatile unsigned char	owDevicesIDs[MAXDEVICES][8];	// Их ID

volatile unsigned char	switches[SWITCH_COUNT];
volatile unsigned char  switchCurr = 0xFF;

void addSwitchIndex(unsigned char indexInDevices){
	if (switchCurr == 0xFF){
		switchCurr = 0;
	}
	switches[switchCurr] = indexInDevices;
	switchCurr++;
}


unsigned char search_ow_devices(void) // поиск всех устройств на шине
{ 
	unsigned char	i;
   	unsigned char	id[OW_ROMCODE_SIZE];
   	unsigned char	diff, sensors_count;

	sensors_count = 0;

	for( diff = OW_SEARCH_FIRST; diff != OW_LAST_DEVICE && sensors_count < MAXDEVICES ; )
    { 
		OW_FindROM( &diff, &id[0] );

      	if( diff == OW_PRESENCE_ERR ) break;

      	if( diff == OW_DATA_ERR )	break;

      	for (i=0;i<OW_ROMCODE_SIZE;i++)
         	owDevicesIDs[sensors_count][i] = id[i];
		
		sensors_count++;
    }
	return sensors_count;

}

void INT0_Init(){
    MCUCR |= (1 << ISC00)|(1 << ISC01);    // set INT0 to trigger on rising edge
    GICR |= (1 << INT0);      // Turns on INT0
}

void commands_Reset(){
	for (unsigned char i=0; i<COMMANDS_COUNT; i++){
		commands[i] = 0xFF;
	}
}

void ports_Init(){
	DDRB = 0b00000010; PORTB = 0b00000010;
	DDRC = 0b00000000; PORTC = 0b00000000;
	DDRD = 0b00000010; PORTD = 0b00000000;
}

void get_Command(void){
	unsigned char stat = MT8870_GetStat();

	if (commands[0] == 0xFF && stat == high(OW_DS2405_FAMILY_CODE)){
		commands[0] = stat;

	}else if(commands[1] == 0xFF && commands[0] == high(OW_DS2405_FAMILY_CODE) 
			 && stat == low(OW_DS2405_FAMILY_CODE)){
		commands[1] = stat;

	}else if (commands[0] == high(OW_DS2405_FAMILY_CODE) 
			  && commands[1] == low(OW_DS2405_FAMILY_CODE) && commands[2] == 0xFF 
			  && (stat == COMMAND_ON || stat == COMMAND_OFF)){
		commands[2] = stat;

	}else{

		commands_Reset();
	}
}

void toggleSwitches(void){
	if (commands[0] == high(OW_DS2405_FAMILY_CODE) 
		&& commands[1] == low(OW_DS2405_FAMILY_CODE)
		&& (commands[2] == COMMAND_OFF || commands[2] == COMMAND_ON)){
		unsigned char stat = MT8870_GetStat();

		for (unsigned char i=0; i<switchCurr; i++){

				if (stat == COMMAND_OFF
					&& DS24x05_GetState(owDevicesIDs[i]) == true){

					DS24x05_Toggle(owDevicesIDs[i]);

					printf("\r");
					print_address(owDevicesIDs[i]);
					printf(" - Switch OFF");
				}else if (stat == COMMAND_ON
						  && DS24x05_GetState(owDevicesIDs[i]) == false){

					DS24x05_Toggle(owDevicesIDs[i]);

					printf("\r");
					print_address(owDevicesIDs[i]);
					printf(" - Switch ON");
				}
		}
		commands_Reset();
	}
}


ISR (INT0_vect)
{
    if (switchCurr != 0xFF){
		get_Command();
	}
}

int main(void)
{
	stdout = &usart_str; // указываем, куда будет выводить printf 

	ports_Init();

	MT8870_Init();

	USART_init(); // включаем uart

	INT0_Init();

	timerDelayInit();

	nDevices = search_ow_devices(); // ищем все устройства

	printf("---------- Found %d devices ----------", nDevices);


	for (unsigned char i=0; i<nDevices; i++) // теперь сотируем устройства и запрашиваем данные
	{
		// узнать устройство можно по его груповому коду, который расположен в первом байте адресса
		switch (owDevicesIDs[i][0])
		{
			case OW_DS18B20_FAMILY_CODE: { // если найден термодатчик DS18B20
				printf("\r"); print_address(owDevicesIDs[i]); // печатаем знак переноса строки, затем - адрес
				printf(" - Thermometer DS18B20"); // печатаем тип устройства 
				DS18x20_StartMeasureAddressed(owDevicesIDs[i]); // запускаем измерение
				timerDelayMs(800); // ждем минимум 750 мс, пока конвентируется температура
				unsigned char	data[2]; // переменная для хранения старшего и младшего байта данных
				DS18x20_ReadData(owDevicesIDs[i], data); // считываем данные
				unsigned char	themperature[3]; // в этот массив будет записана температура
				DS18x20_ConvertToThemperature(data, themperature); // преобразовываем температуру в человекопонятный вид
				printf(": %d.%d C", themperature[1],themperature[2]);
			} break;
			case OW_DS18S20_FAMILY_CODE: { // если найден термодатчик DS18B20
				printf("\r"); print_address(owDevicesIDs[i]); // печатаем знак переноса строки, затем - адрес
				printf(" - Thermometer DS18S20"); // печатаем тип устройства 
			} break;

			case OW_DS1990_FAMILY_CODE: { // если найден электронный ключ DS1990
				printf("\r"); print_address(owDevicesIDs[i]); // печатаем знак переноса строки, затем - адрес
				printf(" - Serial button DS1990"); // печатаем тип устройства 
			} break;
			case OW_DS2430_FAMILY_CODE: { // если найдена EEPROM
				printf("\r"); print_address(owDevicesIDs[i]); // печатаем знак переноса строки, затем - адрес
				printf(" - EEPROM DS2430"); // печатаем тип устройства 
			} break;
			case OW_DS2413_FAMILY_CODE: { // если найден ключ
				printf("\r"); print_address(owDevicesIDs[i]); // печатаем знак переноса строки, затем - адрес
				printf(" - Switch 2413"); // печатаем тип устройства 
			} break;
			case OW_DS2405_FAMILY_CODE: { // если найден ключ
				printf("\r"); 
				print_address(owDevicesIDs[i]); // печатаем знак переноса строки, затем - адрес
				printf(" - Switch 2405"); // печатаем тип устройства 

				addSwitchIndex(i);
				DS24x05_Add(owDevicesIDs[i]);
			} break;
		}

	}

	commands_Reset();	
	sei();

	for(;;)
	{	
		toggleSwitches();
	}

}



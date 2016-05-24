
#ifndef MAIN_H
#define MAIN_H

#define TASCII(numb) (numb+48)

#define true 1
#define false 0

#define vuchar volatile unsigned char 
#define vuint volatile unsigned int
#define COMMANDS_COUNT 3

#define COMMAND_ON 0x0E
#define COMMAND_OFF 0x0F

#define sbi(reg,bit) reg |= (1<<bit)
#define cbi(reg,bit) reg &= ~(1<<bit)
#define ibi(reg,bit) reg ^= (1<<bit)
#define CheckBit(reg,bit) (reg&(1<<bit))

void RunTasks(void);


#endif

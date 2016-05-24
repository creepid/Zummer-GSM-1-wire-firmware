#ifndef DS24x05_H
#define DS24x05_H

#include "onewire.h"
#define DS24X05_CHECK_CRC

#ifndef SWITCH_COUNT
	#define SWITCH_COUNT 8
#endif

#ifndef true
	#define true 1
#endif

#ifndef false
	#define false 0
#endif

char DS24x05_Toggle(unsigned char* rom);
void DS24x05_Add(unsigned char* rom);
unsigned char DS24x05_GetState(unsigned char* rom);

#endif

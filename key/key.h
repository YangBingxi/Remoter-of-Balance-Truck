#ifndef _KEY_H
#define _KEY_H
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/pin_map.h"
#include "grlib/grlib.h"
#include "driverlib/gpio.h"
#include "driverlib/adc.h"
#include "inc/hw_gpio.h"
#include "uart/uart.h"
#include "driverlib/interrupt.h"
#include "uart/uartstdio.h"
#include "inc/hw_ints.h"
#include "delay/delay.h"

extern uint32_t ReadPin0;
extern uint32_t ReadPin4;
extern int  KeyPress4;


void Key_Configure(void);
void Key_Interrupt(void);
int Key_Scan(int PF);
void Int_Handler_GPIOF(void);

#endif

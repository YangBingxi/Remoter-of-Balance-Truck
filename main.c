/**
  ******************************************************************************
  * 文件名程: main.c
  * 作    者: By Sw Young
  * 版    本: V1.0
  * 功    能:
  * 编写日期: 2018.4.10
  ******************************************************************************
  * 说明：用于小车的遥控
  * 硬件平台：TM4C123G
  *   *****
  * 软件设计说明：
  *   *****
  * Github：
  ******************************************************************************
**/
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
#include "uart/uartstdio.h"
#include "key/key.h"
#include "delay/delay.h"

#include "0.96'OLED/bmp.h"
#include "0.96'OLED/OLED.h"

typedef unsigned int uint;
typedef unsigned char uchar;
double ValueAvg;
uint8_t sw = 0;
uint8_t sw_flag = 0,order_flag = 1;
uint8_t Car_Mode = 0;
/*
 * ADC值域说明：
 * 0-4095对应电压的0-3.3v
 */
int main(void)
{
    uint32_t ulADC0_Value,ulADC1_Value,ulADC2_Value,NEW_ulADC2_Value;
    //设置系统时钟为50MHz (400/2/4=50)
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL |SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    Uart0Iint();//串口0初始化
    Uart2Iint();//串口2初始化
    OLED_Init();            //初始化OLED
    OLED_Clear();
    OLED_ShowString(36,0,"Remote",16);
    OLED_ShowString(0,3,"Mode:",16);
    OLED_ShowNum(50,3,Car_Mode,1,16);
    UARTprintf("M%d",Car_Mode);

    //初始化ADC0/PE3
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    /*设置ADC参考电压为外部3V
    ADCReferenceSet(ADC0_BASE, ADC_REF_EXT_3V);*/

    //ADC外部参考电压始终为3.3V
    //配置ADC采集序列          ADC0_BASE/采样序列号/处理器通过ADCProcessorTrigger()函数产生的一个触发/优先级，0最高
    ADCSequenceConfigure(ADC0_BASE,0,ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceConfigure(ADC0_BASE,1,ADC_TRIGGER_PROCESSOR, 1);
    ADCSequenceConfigure(ADC1_BASE,0,ADC_TRIGGER_PROCESSOR, 0);

    //配置采样序列发生器的步进       ADC0_BASE/采样序列号/步进值 (该参数决定了触发产生时ADC捕获采样序列的次序。
    //对于第一个采样序列，其值可以是0～7；对于第二和第三个采样序列，其值从0～3；对于第四个采样序列，其值只能取0。)/
    //选择被采样的通道,该步进可以定义成序列的末尾（ADC_CTL_END位），同时它也可以配置成在步进完成后产生一个中断（ADC_CTL_IE位）
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0 |ADC_CTL_END | ADC_CTL_IE);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH2 |ADC_CTL_END | ADC_CTL_IE);
    ADCSequenceStepConfigure(ADC1_BASE, 0, 0, ADC_CTL_CH1 |ADC_CTL_END | ADC_CTL_IE);

    //使能ADC采集序列
    ADCSequenceEnable(ADC0_BASE, 0);
    ADCSequenceEnable(ADC0_BASE, 1);
    ADCSequenceEnable(ADC1_BASE, 0);

    ADCIntClear(ADC0_BASE, 0);
    ADCIntClear(ADC0_BASE, 1);
    ADCIntClear(ADC1_BASE, 0);

    //sw
    Key_Configure();

    while(1)
    {
        //触发采集
        ADCProcessorTrigger(ADC0_BASE, 0);
        ADCProcessorTrigger(ADC0_BASE, 1);
        ADCProcessorTrigger(ADC1_BASE, 0);
        //等待采集结束
        while(!ADCIntStatus(ADC0_BASE, 0, false)) ;
        ADCIntClear(ADC0_BASE, 0);

        //获取采集结果  0-4095代表0-3.3V
        ADCSequenceDataGet(ADC0_BASE, 0, &ulADC0_Value);
        ValueAvg=ulADC0_Value*3.300/4095;
        //UARTprintf("ulADC0_Value%d\n", ulADC0_Value);

        while(!ADCIntStatus(ADC0_BASE, 1, false)) ;
        ADCIntClear(ADC0_BASE, 1);

        //获取采集结果  0-4095代表0-3.3V
        ADCSequenceDataGet(ADC0_BASE, 1, &NEW_ulADC2_Value);
        ValueAvg=NEW_ulADC2_Value*3.300/4095;
        if(abs(NEW_ulADC2_Value-ulADC2_Value)>10)
        {
            if(ulADC2_Value>1000)
                UARTprintf("T%d", ulADC2_Value);
            else if(ulADC2_Value>100)
            {
                UARTprintf("T0");
                UARTprintf("%d", ulADC2_Value);
            }
            else if(ulADC2_Value>10)
            {
                UARTprintf("T00");
                UARTprintf("%d", ulADC2_Value);
            }
        }

        //UARTprintf("ulADC2_Value%d\n", ulADC2_Value);
        ulADC2_Value = NEW_ulADC2_Value;

        while(!ADCIntStatus(ADC1_BASE, 0, false)) ;
        ADCIntClear(ADC0_BASE, 0);
        ADCSequenceDataGet(ADC1_BASE, 0, &ulADC1_Value);
        ValueAvg=ulADC1_Value*3.300/4095;
        //UARTprintf("ulADC1_Value%d\n", ulADC1_Value);
        SysCtlDelay(SysCtlClockGet()/100);

        //delay_ms(1000);
        Key_Scan(0);
        Key_Scan(4);


        if(true)
        {
            if(ulADC0_Value>0&&ulADC0_Value<1000)
            {
                UARTprintf("B005");
                order_flag = 0;
                OLED_ShowString(110,6,"B",16);
            }
            if(ulADC0_Value>4000&&ulADC0_Value<5000)
            {
                UARTprintf("F005");
                order_flag = 0;
                OLED_ShowString(110,6,"F",16);

            }
            if(ulADC1_Value>0&&ulADC1_Value<1000)
            {
                UARTprintf("L005");
                order_flag = 0;
                OLED_ShowString(110,6,"L",16);

            }
            if(ulADC1_Value>4000&&ulADC1_Value<5000)
            {
                UARTprintf("R005");
                order_flag = 0;
                OLED_ShowString(110,6,"R",16);

            }
            if(ulADC1_Value>1000&&ulADC1_Value<4000&&ulADC0_Value>1000&&ulADC0_Value<4000)
            {
                if(Car_Mode==0)
                {
                    UARTprintf("#");
                    order_flag = 0;
                    OLED_ShowString(110,6,"#",16);
                }

            }

        }
        Delay_ms(20);
        if(ulADC1_Value>1000&&ulADC1_Value<4000&&ulADC0_Value>1000&&ulADC0_Value<4000)
            order_flag = 1;
    }

}

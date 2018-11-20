/**
  ******************************************************************************
  * �ļ�����: main.c
  * ��    ��: By Sw Young
  * ��    ��: V1.0
  * ��    ��:
  * ��д����: 2018.4.10
  ******************************************************************************
  * ˵��������С����ң��
  * Ӳ��ƽ̨��TM4C123G
  *   *****
  * ������˵����
  *   *****
  * Github��
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
 * ADCֵ��˵����
 * 0-4095��Ӧ��ѹ��0-3.3v
 */
int main(void)
{
    uint32_t ulADC0_Value,ulADC1_Value,ulADC2_Value,NEW_ulADC2_Value;
    //����ϵͳʱ��Ϊ50MHz (400/2/4=50)
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL |SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    Uart0Iint();//����0��ʼ��
    Uart2Iint();//����2��ʼ��
    OLED_Init();            //��ʼ��OLED
    OLED_Clear();
    OLED_ShowString(36,0,"Remote",16);
    OLED_ShowString(0,3,"Mode:",16);
    OLED_ShowNum(50,3,Car_Mode,1,16);
    UARTprintf("M%d",Car_Mode);

    //��ʼ��ADC0/PE3
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    /*����ADC�ο���ѹΪ�ⲿ3V
    ADCReferenceSet(ADC0_BASE, ADC_REF_EXT_3V);*/

    //ADC�ⲿ�ο���ѹʼ��Ϊ3.3V
    //����ADC�ɼ�����          ADC0_BASE/�������к�/������ͨ��ADCProcessorTrigger()����������һ������/���ȼ���0���
    ADCSequenceConfigure(ADC0_BASE,0,ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceConfigure(ADC0_BASE,1,ADC_TRIGGER_PROCESSOR, 1);
    ADCSequenceConfigure(ADC1_BASE,0,ADC_TRIGGER_PROCESSOR, 0);

    //���ò������з������Ĳ���       ADC0_BASE/�������к�/����ֵ (�ò��������˴�������ʱADC����������еĴ���
    //���ڵ�һ���������У���ֵ������0��7�����ڵڶ��͵������������У���ֵ��0��3�����ڵ��ĸ��������У���ֵֻ��ȡ0��)/
    //ѡ�񱻲�����ͨ��,�ò������Զ�������е�ĩβ��ADC_CTL_ENDλ����ͬʱ��Ҳ�������ó��ڲ�����ɺ����һ���жϣ�ADC_CTL_IEλ��
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0 |ADC_CTL_END | ADC_CTL_IE);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH2 |ADC_CTL_END | ADC_CTL_IE);
    ADCSequenceStepConfigure(ADC1_BASE, 0, 0, ADC_CTL_CH1 |ADC_CTL_END | ADC_CTL_IE);

    //ʹ��ADC�ɼ�����
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
        //�����ɼ�
        ADCProcessorTrigger(ADC0_BASE, 0);
        ADCProcessorTrigger(ADC0_BASE, 1);
        ADCProcessorTrigger(ADC1_BASE, 0);
        //�ȴ��ɼ�����
        while(!ADCIntStatus(ADC0_BASE, 0, false)) ;
        ADCIntClear(ADC0_BASE, 0);

        //��ȡ�ɼ����  0-4095����0-3.3V
        ADCSequenceDataGet(ADC0_BASE, 0, &ulADC0_Value);
        ValueAvg=ulADC0_Value*3.300/4095;
        //UARTprintf("ulADC0_Value%d\n", ulADC0_Value);

        while(!ADCIntStatus(ADC0_BASE, 1, false)) ;
        ADCIntClear(ADC0_BASE, 1);

        //��ȡ�ɼ����  0-4095����0-3.3V
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

#include "key.h"
#include "0.96'OLED/OLED.h"

uint32_t ReadPin0;
uint32_t ReadPin4;
int KeyPress4=0;
extern uint8_t sw_flag,Car_Mode;
void Key_Configure(void)
{
    //ʹ��GPIO����
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    //����PF0,ֱ�ӶԼĴ������в���
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    //����GPIO����
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0);
    //  GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_DIR_MODE_IN);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    //����GPIO����
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_0);
    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);


}

void Key_Interrupt(void)
{//ע�⣬��ʹ�ô˺���ʱ�����ڳ�ʼ�����������    void Int_Handler_GPIOF(void)   �жϷ������
    //�ж�����
       /****��ʼ���ⲿ�жϲ��������ⲿ�ж�Ϊ�͵�ƽ������ʽ********/
       GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_0);                     //���ⲿ�ж�
       GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_LOW_LEVEL);//PD0�½��ش���
       //GPIOIntRegister(GPIO_PORTD_BASE, Int_Handler_GPIOD);
       IntEnable(INT_GPIOD);
       //IntPrioritySet(INT_GPIOD, 0);                                   //�ж����ȼ�
       IntMasterEnable();

}

int Key_Scan(int PF)
{

    int  KeyFlag=5;
    if(PF==0)
    {
       ReadPin0=GPIOPinRead(GPIO_PORTD_BASE,GPIO_PIN_0);
       if((ReadPin0&GPIO_PIN_0)  != GPIO_PIN_0)
       {
           Delay_ms(10);//delay 100ms
           ReadPin0=GPIOPinRead(GPIO_PORTD_BASE,GPIO_PIN_0);
           if((ReadPin0&GPIO_PIN_0)  != GPIO_PIN_0)
           {
               KeyFlag=0;
               while(!GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_0));
               if(sw_flag==0)
               {
                   UARTprintf("STOP");
                   OLED_ShowString(110,6,"#",16);
               }
               else
               {
                   UARTprintf("START");
                   OLED_ShowString(110,6," ",16);
               }
               sw_flag =~ sw_flag;
           }
       }
    }
    if(PF==4)
    {
        KeyFlag=0;
        ReadPin4=GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4);
        if((ReadPin4&GPIO_PIN_4)  != GPIO_PIN_4)
       {
           Delay_ms(10);//delay 100ms
           ReadPin4=GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4);
           if((ReadPin4&GPIO_PIN_4)  != GPIO_PIN_4)
           {
               if(KeyPress4>=0&&KeyPress4<=4)
                        KeyPress4=(1+KeyPress4);
               if(KeyPress4==4)
                   KeyPress4 = 0;
               Car_Mode = KeyPress4;
               OLED_ShowNum(50,3,Car_Mode,1,16);
               UARTprintf("M%d",Car_Mode);

               KeyFlag=1;
               while(!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4));
           }

       }
    }
       return KeyFlag;
}

void Int_Handler_GPIOF(void)
  {
      /***********��׼�жϷ������������**********/
        uint32_t ui32IntStatus;
        ui32IntStatus = GPIOIntStatus(GPIO_PORTF_BASE, true);
      GPIOIntClear(GPIO_PORTF_BASE, ui32IntStatus);//����жϱ�־λ
      //ֻ��һ������
     if((ui32IntStatus & GPIO_PIN_4)  == GPIO_PIN_4)//PF4
                 {

                     //���ܣ�����PF4ʱ�л�����
                     //��ʼֵΪ�㣬�������κγ���
                     //=1   ->   �ֶ�ģʽ
                     //=2   ->   �Զ�ģʽ
                     //=3   ->   ���ģʽ
                     //=4   ->   ͣ��
                     //�����л���ѭ�������ظ�
                     //�ô�KeyPress4��ѡ�������
                    if(KeyPress4>=0&&KeyPress4<=4)
                             KeyPress4=(1+KeyPress4);
              /*********EX0 = 0; *�ر��ⲿ�ж�****������main�������ǰ�ظ������ⲿ�ж�**************/
                          GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_4);
                 }
//     if((ui32IntStatus & GPIO_PIN_0)  == GPIO_PIN_0)//PF0
//                     {
//             //�˴���д����Ҫ�ĳ���
//                     }
}

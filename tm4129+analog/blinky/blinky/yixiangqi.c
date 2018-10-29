#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "utils/buttons.h"

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

int32_t t;

//*****************************************************************************
//
// Delay for the specified number of seconds.  Depending upon the current
// SysTick value, the delay will be between N-1 and N seconds (i.e. N-1 full
// seconds are guaranteed, aint32_t with the remainder of the current second).
//
//*****************************************************************************
void
Delay(uint32_t ui32Seconds)
{
    //
    // Loop while there are more seconds to wait.
    //
    while(ui32Seconds--) 
    {
        //
        // Wait until the SysTick value is less than 1000.
        //
        while(ROM_SysTickValueGet() > 1000)
        {
        }

        //
        // Wait until the SysTick value is greater than 1000.
        //
        while(ROM_SysTickValueGet() < 1000)
        {
        }
    }
}

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}

//�жϷ�����
void PortFIntHandler(void)
{
	//�����жϴ���Դ��־λ
	uint32_t ulIntSta;
	
	//��ȡ�жϴ���Դ
	ulIntSta = GPIOIntStatus(GPIO_PORTF_BASE,true);
	//����жϱ�־
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_0|GPIO_PIN_4);
	
	//�ж��жϴ���Դ
	if(ulIntSta == GPIO_PIN_0)
	{//��ʱ����
		t += 5;
		if(t > 510)	t = 510;
	}if(ulIntSta == GPIO_PIN_4)
	{//��ʱ�Լ�
		t -= 5;
		if(t <= 0)	t = 5;
	}
}

//�жϷ�����
void PortDIntHandler(void)
{
	//���ñ�־λ
	static uint8_t flag=1;

	//����жϱ�־
	GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_2);
	
	//�ж�PF1��ƽ
	if(flag)		
	{//PF1Ϊ�ߵ�ƽʱ�õ�
		SysCtlDelay(t);
		GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1,0);
		flag = 0;
	}
	else
	{//PF1Ϊ�͵�ƽʱ�ø�
		SysCtlDelay(t);
		GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, GPIO_PIN_1);
		flag = 1;
	}
}
/*******************************************************
/*������
/*
/*���ţ�PD1	��������		PD2	�������
/*			PF0 ���󰴼�		PF4	��С����
/*			PF1 ��ʾ��
*******************************************************/
int main(void)
{	
	//����ʱ��
	SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
								SYSCTL_XTAL_25MHZ);
	//���ô���
	ConfigureUART();
	//���ð���
	ButtonsInit();
	
	
	//ʹ��GPIOF
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	//����PF1Ϊ���
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
	//����PF1Ϊ2mA������
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1 ,
                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);	
	//ʹ��GPIOD
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	//����PD1Ϊ���
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_1);
	//����PD1Ϊ2mA������
	GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_1 ,
                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	//����PD2Ϊ����
	GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_2);
	//����PD2Ϊ2mA������
	GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_2 ,
                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
												 
	
	//GPIOFע���жϣ�ʹ�жϷ���ʱִ�еڶ�����������ĺ���
	GPIOIntRegister(GPIO_PORTF_BASE, PortFIntHandler);	
	//����PF0��PF4�жϴ���
	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4 , GPIO_FALLING_EDGE);
	//ʹ��PF0��FP4�ж�
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4);
	
	//GPIODע���жϣ�ʹ�жϷ���ʱִ�еڶ�����������ĺ���
	GPIOIntRegister(GPIO_PORTD_BASE, PortDIntHandler);
	//����PD2�жϴ���
	GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_2 , GPIO_BOTH_EDGES);
	//ʹ��PD2�ж�
	GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_2);
	
	//ʹ��ȫ���ж�
	IntMasterEnable();
	
	
	//�������
	UARTprintf("START!\n");
	//��t����ֵ
	t = 5;
	//���
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
	
	while(1)
	{

	}
	
}

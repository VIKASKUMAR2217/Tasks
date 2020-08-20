#include <stdio.h>
#include <nrf.h>

#define PIN_TXD        (6)

typedef void (*callback_t)(void);
void timerCallback(void);

void initTimer(void);
void startTimer(uint32_t uTime, callback_t callback);
void stopTimer(void);
void initUSART(void);

volatile static uint32_t mTime;
callback_t callback = NULL;

int main(void)
{

	initTimer();
	
	initUSART();
	
	startTimer(1000, timerCallback); /* 1000 us */
	
	stopTimer();
	
	while(1)
	{
	}
	
	return 0;
}

void initTimer(void)
{
	/*Set to 32-bit timer mode*/
	NRF_TIMER0->BITMODE = TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos;
	
	/*To reduce the frequency at 1MHz i.e. PCLK1M*/
	NRF_TIMER0->PRESCALER = 4 << TIMER_PRESCALER_PRESCALER_Pos;
	
	// Enable IRQ on EVENTS_COMPARE[0]
	NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos;
	
	/*Register and Enable the TIMER0 Interrupt routine*/
	NVIC_EnableIRQ(TIMER0_IRQn);
}

void timerCallback(void)
{
	  /*store the data into transmit buffer*/
		NRF_UARTE0->TXD.PTR = (uint32_t)mTime;
	
		/*Start the transfer data */
		NRF_UARTE0->TASKS_STARTTX = 1;
	
		/* Wait until the transfer is complete */
		while (NRF_UARTE0->EVENTS_ENDTX == 0)
		{
		}
		
		/*stop the UART TX for low power mode*/
		NRF_UARTE0->TASKS_STOPTX = 1;
}

void initUSART(void)
{
	/*Disabled the hardware flow control*/
	NRF_UARTE0->CONFIG = UART_CONFIG_HWFC_Disabled << UART_CONFIG_HWFC_Pos;
	
	/*include 1 parity bit*/
	NRF_UARTE0->CONFIG = UART_CONFIG_PARITY_Included << UART_CONFIG_PARITY_Pos;
	
	/*set baud rate at 115200 */
	NRF_UARTE0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud115200 << UART_BAUDRATE_BAUDRATE_Pos;
	
	/*select the TX pin as PIN 6*/
	NRF_UARTE0->PSEL.TXD = PIN_TXD;
	
	/*Enable the UART as TX only*/
	NRF_UARTE0->ENABLE = UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos;
	
	NRF_UARTE0->TASKS_STARTTX = 1;
	
}

void startTimer(uint32_t uTime, callback_t callback)
{
	/*Register callback function*/
	callback = timerCallback;
	
	/*capture compare value and generates EVENTS_COMPARE[0]*/
	NRF_TIMER0->CC[0] = uTime;
	
	/*start the task and at every uTime it will interrupt the TIMER0_IRQHandler*/
	NRF_TIMER0->TASKS_START = 1;
}

void stopTimer(void)
{
	/*Stop the TIMER0_IRQHandler interrupt*/
	NRF_TIMER0->TASKS_STOP = 1;
}

void TIMER0_IRQHandler(void)
{
	volatile static uint32_t count;
	
	mTime++;
	
	if(count++ == 509)
	{    
		/*to print at every 509 tick count*/
		callback();
		
		count = 0;
	}
	
	/* reset the EVENTS_COMPARE[0] */
	NRF_TIMER0->EVENTS_COMPARE[0] = 0;
}

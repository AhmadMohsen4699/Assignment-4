#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

SemaphoreHandle_t xmutex;

void vHIGH(void *pvParameters);
void vMID(void *pvParameters);
void vLOW(void *pvParameters);

xTaskHandle xMID_Handle;
xTaskHandle xHIGH_Handle;

void Init(){
  SYSCTL_RCGCGPIO_R |= 0x00000020;   //Initialize clock to PORTF
  while((SYSCTL_PRGPIO_R&0x00000020) == 0){}  //safety for clock initialization
  GPIO_PORTF_LOCK_R = 0x4C4F434B;
  GPIO_PORTF_CR_R = 0x1F;       //Enable change to PORTF
  GPIO_PORTF_DIR_R = 0x0E;      //Make led ports as output
  GPIO_PORTF_DEN_R = 0x1F;      // digital enable to pins
  GPIO_PORTF_PUR_R = 0x11;
}

int main(void)
{
	Init();
	xmutex = xSemaphoreCreateMutex();
	if(xmutex!=NULL)
	{
	xTaskCreate(vLOW,"Task 1",100, NULL, 2,NULL);
	xTaskCreate(vMID,"Task 2",100, NULL,1,&xMID_Handle);
	xTaskCreate(vHIGH,"Task 3",100, NULL,1,&xHIGH_Handle);
	vTaskStartScheduler();
	}
	
}

void vLOW(void *pvParameters)
{
	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	
	for(;;)
	{
		xSemaphoreTake(xmutex, portMAX_DELAY);
		GPIO_PORTF_DATA_R=0x00;
		vTaskPrioritySet(xHIGH_Handle,4);
		GPIO_PORTF_DATA_R=0x02;
		vTaskPrioritySet(xMID_Handle,3);
		xSemaphoreGive(xmutex);	
	}
}

void vHIGH(void *pvParameters)
{
	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	for(;;)
	{
		xSemaphoreTake(xmutex, portMAX_DELAY);
		GPIO_PORTF_DATA_R=0x04;
		xSemaphoreGive(xmutex);	
		vTaskDelayUntil(&xLastWakeTime,640/portTICK_RATE_MS);
		
	}
}

void vMID(void *pvParameters)
{
	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	for(;;)
	{
		GPIO_PORTF_DATA_R=0x08;
		vTaskDelayUntil(&xLastWakeTime,640/portTICK_RATE_MS);
	}
}

void vApplicationIdleHook(void){}

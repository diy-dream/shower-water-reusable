/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include "tim.h"
#include "usart.h"
#include "WS2812.h"
#include "stripEffects.h"
#include "lwow/lwow.h"
#include "lwow/devices/lwow_device_ds18x20.h"
#include "scan_devices.h"
#include "Buttons.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define UPDATE_INTERVAL 	15 //refresh rate: 1/0.015ms = 66Hz
#define TASK_INTERVAL		10000
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern const lwow_ll_drv_t lwow_ll_drv_stm32_hal;
lwow_t ow;
lwow_rom_t rom_ids[20];
size_t rom_found;
float actual_temp = 0;
int error_count = 0;
static uint8_t tempReadyCtn = 0;
static bool tempReady = false;
/* USER CODE END Variables */
/* Definitions for MainTask */
osThreadId_t MainTaskHandle;
const osThreadAttr_t MainTask_attributes = {
  .name = "MainTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for circularRingRed */
osThreadId_t circularRingRedHandle;
const osThreadAttr_t circularRingRed_attributes = {
  .name = "circularRingRed",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for ReadTemperature */
osThreadId_t ReadTemperatureHandle;
const osThreadAttr_t ReadTemperature_attributes = {
  .name = "ReadTemperature",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for circularRingGre */
osThreadId_t circularRingGreHandle;
const osThreadAttr_t circularRingGre_attributes = {
  .name = "circularRingGre",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for errorTask */
osThreadId_t errorTaskHandle;
const osThreadAttr_t errorTask_attributes = {
  .name = "errorTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for errorSignalRing */
osThreadId_t errorSignalRingHandle;
const osThreadAttr_t errorSignalRing_attributes = {
  .name = "errorSignalRing",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for updateTimer */
osTimerId_t updateTimerHandle;
const osTimerAttr_t updateTimer_attributes = {
  .name = "updateTimer"
};
/* Definitions for buttonBinarySem */
osSemaphoreId_t buttonBinarySemHandle;
const osSemaphoreAttr_t buttonBinarySem_attributes = {
  .name = "buttonBinarySem"
};
/* Definitions for readTemperatureBinarySem */
osSemaphoreId_t readTemperatureBinarySemHandle;
const osSemaphoreAttr_t readTemperatureBinarySem_attributes = {
  .name = "readTemperatureBinarySem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
uint8_t getButton();
void water_valve_control(bool activate);
/* USER CODE END FunctionPrototypes */

void Main_Task(void *argument);
void CircularRingRedTask(void *argument);
void readTemperatureTask(void *argument);
void CircularRingGreen(void *argument);
void ErrorTask(void *argument);
void ErrorSignalRingLedTask(void *argument);
void vTimerUpdateCallback(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	water_valve_control(false); //Water valve off
	ws2812_init(&htim1, NULL);
	fillBufferBlack();
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of buttonBinarySem */
  buttonBinarySemHandle = osSemaphoreNew(1, 1, &buttonBinarySem_attributes);

  /* creation of readTemperatureBinarySem */
  readTemperatureBinarySemHandle = osSemaphoreNew(1, 1, &readTemperatureBinarySem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of updateTimer */
  updateTimerHandle = osTimerNew(vTimerUpdateCallback, osTimerPeriodic, NULL, &updateTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of MainTask */
  MainTaskHandle = osThreadNew(Main_Task, NULL, &MainTask_attributes);

  /* creation of circularRingRed */
  circularRingRedHandle = osThreadNew(CircularRingRedTask, NULL, &circularRingRed_attributes);

  /* creation of ReadTemperature */
  ReadTemperatureHandle = osThreadNew(readTemperatureTask, NULL, &ReadTemperature_attributes);

  /* creation of circularRingGre */
  circularRingGreHandle = osThreadNew(CircularRingGreen, NULL, &circularRingGre_attributes);

  /* creation of errorTask */
  errorTaskHandle = osThreadNew(ErrorTask, NULL, &errorTask_attributes);

  /* creation of errorSignalRing */
  errorSignalRingHandle = osThreadNew(ErrorSignalRingLedTask, NULL, &errorSignalRing_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  osThreadSuspend(ReadTemperatureHandle);
  osThreadSuspend(circularRingRedHandle);
  osThreadSuspend(circularRingGreHandle);
  osThreadSuspend(errorSignalRingHandle);
  osThreadSuspend(errorTaskHandle);
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_Main_Task */
/**
  * @brief  Function implementing the MainTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Main_Task */
void Main_Task(void *argument)
{
  /* USER CODE BEGIN Main_Task */
	osThreadSuspend(ReadTemperatureHandle);
	osThreadSuspend(circularRingRedHandle);
	osThreadSuspend(circularRingGreHandle);

	bool isStarted = false;
	bool isAborted = false;
	uint32_t systemStartTime = 0;

	for(;;){
		ButtonState buttons = getButtonState();

		switch(buttons){
			case BUTTON_SHORT:
				isStarted = true;
				isAborted = false;
				systemStartTime = xTaskGetTickCount();
				break;
			case BUTTON_LONG:
				isAborted = true;
				isStarted = false;
				break;
			case BUTTON_DOUBLE:
			case BUTTON_NONE:
				break;
		}
		if(isStarted){
			// Resume the readTemperature task
			osThreadResume(ReadTemperatureHandle);

			// Resume the led ring task
			osThreadResume(circularRingRedHandle);

			if(tempReady){
				if(actual_temp < CORRECT_TEMPERATURE){
					if((xTaskGetTickCount() - systemStartTime) > TEMPERATURE_ERROR_TIME){
						water_valve_control(false);
						osThreadSuspend(circularRingRedHandle);
						osThreadResume(errorTaskHandle);
						fillBufferBlack();
						osDelay(4000);
						HAL_NVIC_SystemReset();
					}
					// Activate the relay to open the valve for the water
					else water_valve_control(true);
				}else{
					// Deactivate the relay to close the valve because the temperature of the water is good !
					water_valve_control(false);

					//Suspend temperature en led ring task
					//osThreadSuspend(readTemperatureTask);
					osThreadSuspend(circularRingRedHandle);

					osThreadResume(circularRingGreHandle);

					osDelay(4000);

					osThreadSuspend(circularRingGreHandle);
					fillBufferBlack();
					HAL_NVIC_SystemReset();
				}
			}
		}else if(isAborted){
			// Deactivate the relay to close the valve because the temperature of the water is good !
			water_valve_control(false);
			osThreadSuspend(circularRingRedHandle);
			fillBufferBlack();
			osDelay(2000);
			HAL_NVIC_SystemReset();
		}
	}
  /* USER CODE END Main_Task */
}

/* USER CODE BEGIN Header_CircularRingRedTask */
/**
* @brief Function implementing the circularRingRed thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_CircularRingRedTask */
void CircularRingRedTask(void *argument)
{
  /* USER CODE BEGIN CircularRingRedTask */
	stripEffect_ColorWheel(50);
  /* USER CODE END CircularRingRedTask */
}

/* USER CODE BEGIN Header_readTemperatureTask */
/**
* @brief Function implementing the ReadTemperature thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_readTemperatureTask */
void readTemperatureTask(void *argument)
{
  /* USER CODE BEGIN readTemperatureTask */
	float avg_temp;
	size_t avg_temp_count;

	/* Initialize 1-Wire library and set user argument to NULL */
	lwow_init(&ow, &lwow_ll_drv_stm32_hal, &huart1);

	/* Get onewire devices connected on 1-wire port */
	do {
		if (scan_onewire_devices(&ow, rom_ids, LWOW_ARRAYSIZE(rom_ids), &rom_found) == lwowOK) {
			printf("Devices scanned, found %d devices!\r\n", (int)rom_found);
		} else {
			printf("Device scan error\r\n");
			error_count++;
			if(error_count >= MAX_ERROR_COUNT){
				osThreadResume(errorTaskHandle);
			}
		}
		if (rom_found == 0) {
			osDelay(1000);
		}
	} while (rom_found == 0);

	if (rom_found > 0) {
		/* Infinite loop */
		actual_temp = 0;
		tempReadyCtn = 0;
		tempReady = false;
		while (1) {
			printf("Start temperature conversion\r\n");
			lwow_ds18x20_start(&ow, NULL);      /* Start conversion on all devices, use protected API */
			osDelay(1000);                      /* Release thread for 1 second */

			/* Read temperature on all devices */
			avg_temp = 0;
			avg_temp_count = 0;
			for (size_t i = 0; i < rom_found; i++) {
				if (lwow_ds18x20_is_b(&ow, &rom_ids[i])) {
					float temp;
					uint8_t resolution = lwow_ds18x20_get_resolution(&ow, &rom_ids[i]);
					if (lwow_ds18x20_read(&ow, &rom_ids[i], &temp)) {
						printf("Sensor %02u temperature is %d.%d degrees (%u bits resolution)\r\n",
							(unsigned)i, (int)temp, (int)((temp * 1000.0f) - (((int)temp) * 1000)), (unsigned)resolution);

						avg_temp += temp;
						actual_temp = temp;
						avg_temp_count++;
						if(tempReadyCtn >= 2){
							tempReady = true;
						}else{
							tempReadyCtn++;
						}
					} else {
						printf("Could not read temperature on sensor %u\r\n", (unsigned)i);

						if(error_count > MAX_ERROR_COUNT){
							osThreadResume(errorTaskHandle);
						}else{
							error_count++;
						}
					}
				}
			}
			if (avg_temp_count > 0) {
				avg_temp = avg_temp / avg_temp_count;
			}
			printf("Average temperature: %d.%d degrees\r\n", (int)avg_temp, (int)((avg_temp * 100.0f) - ((int)avg_temp) * 100));
		}
	}
	printf("Terminating application thread\r\n");
	osThreadExit();
  /* USER CODE END readTemperatureTask */
}

/* USER CODE BEGIN Header_CircularRingGreen */
/**
* @brief Function implementing the circularRingGre thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_CircularRingGreen */
void CircularRingGreen(void *argument)
{
  /* USER CODE BEGIN CircularRingGreen */
	stripEffect_CircularRing(50, 0, 255, 0);
  /* USER CODE END CircularRingGreen */
}

/* USER CODE BEGIN Header_ErrorTask */
/**
* @brief Function implementing the errorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ErrorTask */
void ErrorTask(void *argument)
{
  /* USER CODE BEGIN ErrorTask */
	// Deactivate the relay to close the valve because the temperature of the water is good !
	water_valve_control(false);

	osThreadSuspend(ReadTemperatureHandle);
	osThreadSuspend(circularRingRedHandle);
	osThreadSuspend(circularRingGreHandle);

	osThreadResume(errorSignalRingHandle);
	osDelay(5000);
	osThreadSuspend(errorSignalRingHandle);

	osSemaphoreAcquire(buttonBinarySemHandle , osWaitForever);
	osThreadTerminate(MainTaskHandle);
	MainTaskHandle = osThreadNew(Main_Task, NULL, &MainTask_attributes);
  /* USER CODE END ErrorTask */
}

/* USER CODE BEGIN Header_ErrorSignalRingLedTask */
/**
* @brief Function implementing the errorSignalRing thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ErrorSignalRingLedTask */
void ErrorSignalRingLedTask(void *argument)
{
  /* USER CODE BEGIN ErrorSignalRingLedTask */
	stripEffect_HeartBeat(250, 255, 0, 0);
  /* USER CODE END ErrorSignalRingLedTask */
}

/* vTimerUpdateCallback function */
void vTimerUpdateCallback(void *argument)
{
  /* USER CODE BEGIN vTimerUpdateCallback */
	ws2812_update();
  /* USER CODE END vTimerUpdateCallback */
}

/* Private application code --------------------------------------------------*/
void water_valve_control(bool activate){
	if(activate){
		printf("Turn on the valve to put the cold water into the cistern");
		HAL_GPIO_WritePin(relayControl_GPIO_Port, relayControl_Pin, GPIO_PIN_RESET);
	}else{
		printf("Turn off the valve to put the water into the normal circuit");
		HAL_GPIO_WritePin(relayControl_GPIO_Port, relayControl_Pin, GPIO_PIN_SET);
	}
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

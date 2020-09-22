#include "ws2812.h"

/* Variables -----------------------------------------------*/
static uint8_t LEDbuffer[LED_BUFFER_SIZE];

TIM_HandleTypeDef *TimHandle;
TIM_OC_InitTypeDef *sConfig;
GPIO_InitTypeDef GPIO_InitStruct;
DMA_HandleTypeDef hdma_tim;

/* Functions -----------------------------------------------*/

void ws2812_init(TIM_HandleTypeDef *htim1, TIM_OC_InitTypeDef *sConfigOC) {
	TimHandle = htim1;
	sConfigOC = sConfig;

	fillBufferBlack();

	/*##-3- Start PWM signal generation in DMA mode ############################*/
	HAL_TIM_PWM_Start_DMA(TimHandle, TIM_CHANNEL_1, (uint32_t *) LEDbuffer, LED_BUFFER_SIZE);
}

void ws2812_update(void) {
	//HAL_TIM_PWM_ConfigChannel(TimHandle, sConfig, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start_DMA(TimHandle, TIM_CHANNEL_1, (uint32_t *) LEDbuffer, LED_BUFFER_SIZE);
}

void setLEDcolor(uint32_t LEDnumber, uint8_t RED, uint8_t GREEN, uint8_t BLUE) {
	uint8_t tempBuffer[24];
	uint32_t i;
	uint32_t LEDindex;
	LEDindex = LEDnumber % LED_NUMBER;

	for (i = 0; i < 8; i++) // GREEN data
		tempBuffer[i] = ((GREEN << i) & 0x80) ? WS2812_1 : WS2812_0;
	for (i = 0; i < 8; i++) // RED
		tempBuffer[8 + i] = ((RED << i) & 0x80) ? WS2812_1 : WS2812_0;
	for (i = 0; i < 8; i++) // BLUE
		tempBuffer[16 + i] = ((BLUE << i) & 0x80) ? WS2812_1 : WS2812_0;

	for (i = 0; i < 24; i++)
		LEDbuffer[RESET_SLOTS_BEGIN + LEDindex * 24 + i] = tempBuffer[i];
}

void setWHOLEcolor(uint8_t RED, uint8_t GREEN, uint8_t BLUE) {
	uint32_t index;

	for (index = 0; index < LED_NUMBER; index++)
		setLEDcolor(index, RED, GREEN, BLUE);
}

void fillBufferBlack(void) {
	/*Fill LED buffer - ALL OFF*/
	uint32_t index, buffIndex;
	buffIndex = 0;

	for (index = 0; index < RESET_SLOTS_BEGIN; index++) {
		LEDbuffer[buffIndex] = WS2812_RESET;
		buffIndex++;
	}
	for (index = 0; index < LED_DATA_SIZE; index++) {
		LEDbuffer[buffIndex] = WS2812_0;
		buffIndex++;
	}
	LEDbuffer[buffIndex] = WS2812_0;
	buffIndex++;
	for (index = 0; index < RESET_SLOTS_END; index++) {
		LEDbuffer[buffIndex] = 0;
		buffIndex++;
	}
}

void fillBufferWhite(void) {
	/*Fill LED buffer - ALL OFF*/
	uint32_t index, buffIndex;
	buffIndex = 0;

	for (index = 0; index < RESET_SLOTS_BEGIN; index++) {
		LEDbuffer[buffIndex] = WS2812_RESET;
		buffIndex++;
	}
	for (index = 0; index < LED_DATA_SIZE; index++) {
		LEDbuffer[buffIndex] = WS2812_1;
		buffIndex++;
	}
	LEDbuffer[buffIndex] = WS2812_0;
	buffIndex++;
	for (index = 0; index < RESET_SLOTS_END; index++) {
		LEDbuffer[buffIndex] = 0;
		buffIndex++;
	}
}

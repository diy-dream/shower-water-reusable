/*
 * Buttons.c
 *
 *  Created on: 21 sept. 2020
 *      Author: Alan
 */


#include "Buttons.h"
#include "FreeRTOS.h"
#include "task.h"
#include "gpio.h"
uint32_t lastButtonTime = 0;

uint8_t getButton() {
	return HAL_GPIO_ReadPin(controlSwitch_GPIO_Port, controlSwitch_Pin) == GPIO_PIN_RESET ?
			1 : 0;
}

ButtonState getButtonState() {
	/*
	 * Read in the buttons and then determine if a state change needs to occur
	 */

	/*
	 * If the previous state was  00 Then we want to latch the new state if
	 * different & update time
	 * If the previous state was !00 Then we want to search if we trigger long
	 * press (buttons still down), or if release we trigger press
	 * (downtime>filter)
	 */
	static uint8_t previousState = 0;
	static uint32_t previousStateChange = 0;
	const uint16_t timeout = 1000;
	uint8_t currentState;
	currentState = getButton();

	if (currentState)
		lastButtonTime = xTaskGetTickCount();
	if (currentState == previousState) {
		if (currentState == 0)
			return BUTTON_NONE;
		if ((xTaskGetTickCount() - previousStateChange) > timeout) {
			// User has been holding the button down
			// We want to send a button is held message
			if (currentState == 0x01)
				return BUTTON_LONG;
			else
				return BUTTON_NONE; // Both being held case, we dont long hold this
		} else
			return BUTTON_NONE;
	} else {
		// A change in button state has occurred
		ButtonState retVal = BUTTON_NONE;
		if (currentState) {
			// User has pressed a button down (nothing done on down)
			if (currentState != previousState) {
				// There has been a change in the button states
				// If there is a rising edge on one of the buttons from double press we
				// want to mask that out As users are having issues with not release
				// both at once
				if (previousState == 0x01)
					currentState = 0x01;
			}
		} else {
			// User has released buttons
			// If they previously had the buttons down we want to check if they were <
			// long hold and trigger a press
			if ((xTaskGetTickCount() - previousStateChange) < timeout) {
				// The user didn't hold the button for long
				// So we send button press

				if (previousState == 0x01)
					retVal = BUTTON_SHORT;
			}
		}
		previousState = currentState;
		previousStateChange = xTaskGetTickCount();
		return retVal;
	}
	return BUTTON_NONE;
}

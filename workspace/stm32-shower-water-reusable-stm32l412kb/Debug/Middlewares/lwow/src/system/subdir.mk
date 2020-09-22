################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/lwow/src/system/lwow_ll_stm32_hal.c \
../Middlewares/lwow/src/system/lwow_sys_cmsis_os.c 

OBJS += \
./Middlewares/lwow/src/system/lwow_ll_stm32_hal.o \
./Middlewares/lwow/src/system/lwow_sys_cmsis_os.o 

C_DEPS += \
./Middlewares/lwow/src/system/lwow_ll_stm32_hal.d \
./Middlewares/lwow/src/system/lwow_sys_cmsis_os.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/lwow/src/system/lwow_ll_stm32_hal.o: ../Middlewares/lwow/src/system/lwow_ll_stm32_hal.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32L412xx -DDEBUG -c -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/lwow/src/include/lwow -I../Middlewares/lwow/src/include -I../Middlewares/snippets/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Middlewares/lwow/src/system/lwow_ll_stm32_hal.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Middlewares/lwow/src/system/lwow_sys_cmsis_os.o: ../Middlewares/lwow/src/system/lwow_sys_cmsis_os.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32L412xx -DDEBUG -c -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/lwow/src/include/lwow -I../Middlewares/lwow/src/include -I../Middlewares/snippets/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Middlewares/lwow/src/system/lwow_sys_cmsis_os.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"


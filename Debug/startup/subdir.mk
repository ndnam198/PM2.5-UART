################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../startup/startup_mke02z4.c 

OBJS += \
./startup/startup_mke02z4.o 

C_DEPS += \
./startup/startup_mke02z4.d 


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DCPU_MKE02Z16VLC4 -DCPU_MKE02Z16VLC4_cm0plus -DFSL_RTOS_BM -DSDK_OS_BAREMETAL -DSDK_DEBUGCONSOLE=0 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"E:\PM2_5-Temp-Humidity-2\MKE02Z16xxx4_Project\drivers" -I"E:\PM2_5-Temp-Humidity-2\MKE02Z16xxx4_Project\CMSIS" -I"E:\PM2_5-Temp-Humidity-2\MKE02Z16xxx4_Project\device" -I"E:\PM2_5-Temp-Humidity-2\MKE02Z16xxx4_Project\board" -I"E:\PM2_5-Temp-Humidity-2\MKE02Z16xxx4_Project\source" -I"E:\PM2_5-Temp-Humidity-2\MKE02Z16xxx4_Project" -I"E:\PM2_5-Temp-Humidity-2\MKE02Z16xxx4_Project\drivers" -I"E:\PM2_5-Temp-Humidity-2\MKE02Z16xxx4_Project\device" -I"E:\PM2_5-Temp-Humidity-2\MKE02Z16xxx4_Project\CMSIS" -O0 -fno-common -g3 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="../$(@D)/"=. -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../LWIP/Target/ethernetif.c 

OBJS += \
./LWIP/Target/ethernetif.o 

C_DEPS += \
./LWIP/Target/ethernetif.d 


# Each subdirectory must supply rules for building sources it contributes
LWIP/Target/%.o LWIP/Target/%.su LWIP/Target/%.cyclo: ../LWIP/Target/%.c LWIP/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/Middlewares/Third_Party/LwIP/src/include -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/Middlewares/Third_Party/LwIP/src/include/compat -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/Middlewares/Third_Party/LwIP/src/include/compat/stdc -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/Middlewares/Third_Party/LwIP/src/include/compat/posix -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/Middlewares/Third_Party/LwIP/src/include/lwip -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/Middlewares/Third_Party/LwIP/src/include/lwip/prot -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/Middlewares/Third_Party/LwIP/src/include/lwip/priv -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/Middlewares/Third_Party/LwIP/src/include/lwip/apps -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/Middlewares/Third_Party/LwIP/src/include/netif -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/Middlewares/Third_Party/LwIP/src/include/netif/ppp -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/Middlewares/Third_Party/LwIP/system -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/Middlewares/Third_Party/LwIP/system/arch -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/Middlewares/Third_Party/LwIP/system/arch -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/LWIP -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/LWIP/App -I/home/nidhi/Downloads/C-DAC/Project/STM_Codes/LWIP_RX2/LWIP/Target -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-LWIP-2f-Target

clean-LWIP-2f-Target:
	-$(RM) ./LWIP/Target/ethernetif.cyclo ./LWIP/Target/ethernetif.d ./LWIP/Target/ethernetif.o ./LWIP/Target/ethernetif.su

.PHONY: clean-LWIP-2f-Target


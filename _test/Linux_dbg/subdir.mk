################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../mqtt_domoticz.cpp \
../main.cpp 

OBJS += \
./mqtt_domoticz.o \
./main.o 

CPP_DEPS += \
./mqtt_domoticz.d \
./main.d 

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -D__linux_build__ -I"../../modbus" -I"../../tcpip" -I"../../socket" -I"../../adapters" -I"../../threads" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



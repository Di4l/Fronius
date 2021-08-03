################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../tcp.cpp \
../tcpsock.cpp 

OBJS += \
./tcp.o \
./tcpsock.o 

CPP_DEPS += \
./tcp.d \
./tcpsock.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DDLLBUILD -DUSE_UNIX -I"E:\proyectos\electronica\fronius\src\adapters" -I"E:\proyectos\electronica\fronius\src\socket" -I"E:\proyectos\electronica\fronius\src\threads\src" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



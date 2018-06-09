################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/funciones_consola.c \
../src/funciones_planificador.c \
../src/planificador.c 

OBJS += \
./src/funciones_consola.o \
./src/funciones_planificador.o \
./src/planificador.o 

C_DEPS += \
./src/funciones_consola.d \
./src/funciones_planificador.d \
./src/planificador.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2018-1c-A-la-grande-le-puse-Jacketing/sisopLib" -Ireadline -Icommons -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



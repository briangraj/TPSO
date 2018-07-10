################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/compactacion.c \
../src/coordinador.c \
../src/distribucion.c \
../src/get.c \
../src/hilo_esi.c \
../src/hilo_instancia.c \
../src/set.c \
../src/store.c \
../src/t_instancia.c \
../src/t_mensaje.c \
../src/t_solicitud.c 

OBJS += \
./src/compactacion.o \
./src/coordinador.o \
./src/distribucion.o \
./src/get.o \
./src/hilo_esi.o \
./src/hilo_instancia.o \
./src/set.o \
./src/store.o \
./src/t_instancia.o \
./src/t_mensaje.o \
./src/t_solicitud.o 

C_DEPS += \
./src/compactacion.d \
./src/coordinador.d \
./src/distribucion.d \
./src/get.d \
./src/hilo_esi.d \
./src/hilo_instancia.d \
./src/set.d \
./src/store.d \
./src/t_instancia.d \
./src/t_mensaje.d \
./src/t_solicitud.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2018-1c-A-la-grande-le-puse-Jacketing/sisopLib" -Icommons -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



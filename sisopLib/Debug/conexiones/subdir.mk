################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../conexiones/archivos.c \
../conexiones/estructuras_coord.c \
../conexiones/serializacion.c \
../conexiones/sockets.c \
../conexiones/strings.c \
../conexiones/threads.c 

OBJS += \
./conexiones/archivos.o \
./conexiones/estructuras_coord.o \
./conexiones/serializacion.o \
./conexiones/sockets.o \
./conexiones/strings.o \
./conexiones/threads.o 

C_DEPS += \
./conexiones/archivos.d \
./conexiones/estructuras_coord.d \
./conexiones/serializacion.d \
./conexiones/sockets.d \
./conexiones/strings.d \
./conexiones/threads.d 


# Each subdirectory must supply rules for building sources it contributes
conexiones/%.o: ../conexiones/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -Icommons -Ipthread -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



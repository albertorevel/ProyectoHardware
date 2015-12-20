################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../8led.c \
../Bmp.c \
../button.c \
../lcd.c \
../led.c \
../main.c \
../stack.c \
../sudoku_2015.c \
../timer.c \
../timer2.c 

ASM_SRCS += \
../candidatos_arm.asm \
../candidatos_thumb.asm \
../provocarExc.asm \
../recalcular.asm 

OBJS += \
./8led.o \
./Bmp.o \
./button.o \
./candidatos_arm.o \
./candidatos_thumb.o \
./lcd.o \
./led.o \
./main.o \
./provocarExc.o \
./recalcular.o \
./stack.o \
./sudoku_2015.o \
./timer.o \
./timer2.o 

C_DEPS += \
./8led.d \
./Bmp.d \
./button.d \
./lcd.d \
./led.d \
./main.d \
./stack.d \
./sudoku_2015.d \
./timer.d \
./timer2.d 

ASM_DEPS += \
./candidatos_arm.d \
./candidatos_thumb.d \
./provocarExc.d \
./recalcular.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC C Compiler'
	arm-none-eabi-gcc -I"Z:\Php2\Practica2\common" -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -mapcs-frame -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.asm
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC Assembler'
	arm-none-eabi-gcc -x assembler-with-cpp -I"Z:\Php2\Practica2\common" -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



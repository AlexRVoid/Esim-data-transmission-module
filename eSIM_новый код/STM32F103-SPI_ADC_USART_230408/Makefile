#==========================================================
#	File:	Makefile for Cortex-M3
#	Date:	2021-07-17
#==========================================================

# Включаем наши файлы в проект, при появлении новых файлов
# не забывайте этот раздел обновлять
SRC_C = startup.c
SRC_C += main.c

#----------------------------------------------------------
# Указываем пути до GCC, а так же пути к директориям где хранятся 
# файлы исходных кодов и заголовочные файлы

#CROSS_PATH = C:/Tools/CodeSourcery/2010-09-51/
CROSS_PATH = "C:\Tools\GNU Tools ARM Embedded\6 2017-q1-update/"

CROSS = $(CROSS_PATH)/bin/arm-none-eabi-

INCLUDES += -I$(CROSS_PATH)/arm-none-eabi/include
INCLUDES += -I$(CROSS_PATH)/arm-none-eabi/include/lib

INCLUDES += -ISystem
VPATH += System

#-------------------- ключи компиляции ---------------------
FLAGS_C  = $(INCLUDES) -I.
FLAGS_C += -Os$(OPTIMIZATION) # Оптимизация по размеру
FLAGS_C += -Wall # Разрешаем вывод сообщений о предупреждениях и ошибках
FLAGS_C += -c # Создать из исходных файлов объектные .o
FLAGS_C += -fno-builtin
# Поместить каждую функцию и элемент данных в отдельный раздел выходного файла, 
# если целевой объект поддерживает произвольные разделы (2 строки)
FLAGS_C += -ffunction-sections
FLAGS_C += -fdata-sections
FLAGS_C += -msoft-float # Программная эмуляция команд с плавающей запятой
FLAGS_C += -mtune=cortex-m3 # Для улучшения производительности кода
FLAGS_C += -mcpu=cortex-m3 # Тип процессора ARM
FLAGS_C += -mthumb # Генерирует код для 16 битного набора инструкций Thumb
FLAGS_C += -gdwarf-2 # В target.lss Си строки + их компиляция в ASM

FLAGS_LD = -Xlinker -Map=target/target.map
FLAGS_LD += -Wl,--gc-sections
FLAGS_LD += -mcpu=cortex-m3
FLAGS_LD += -mthumb
FLAGS_LD += -static   
FLAGS_LD += -nostdlib

#----------------------------------------------------------
all: clean target.elf

%.elf: $(SRC_C:%.c=target/%.o)
	@echo Linking: $@
	@$(CROSS)gcc $(FLAGS_LD) -T'System/stm32f103c6t6.lsf' -o 'target/$@' $^ $(LD_LIB)
	@echo '-----------------------------------------------------------'
	@$(CROSS)size 'target/target.elf'
	@$(CROSS)objcopy -O binary 'target/target.elf' 'target/target.bin'
	@$(CROSS)objcopy -O ihex 'target/target.elf' 'target/target.hex'
	@$(CROSS)objdump -h -S -z 'target/target.elf' > 'target/target.lss'
	@$(CROSS)nm -n 'target/target.elf' > 'target/target.sym'
	@rm -f target/*.o

$(SRC_C:%.c=target/%.o): target/%.o: %.c
	@echo Compiling: $<
	@$(CROSS)gcc $(FLAGS_C) -c $< -o $@

clean:
	@echo '-----------------------------------------------------------'
	@rm -f target/*.*	

.PHONY : all clean
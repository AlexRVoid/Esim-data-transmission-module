#==========================================================
#	File:	Makefile for Cortex-M3
#	Date:	2021-07-17
#==========================================================

# �������� ���� ����� � ������, ��� ��������� ����� ������
# �� ��������� ���� ������ ���������
SRC_C = startup.c
SRC_C += main.c

#----------------------------------------------------------
# ��������� ���� �� GCC, � ��� �� ���� � ����������� ��� �������� 
# ����� �������� ����� � ������������ �����

#CROSS_PATH = C:/Tools/CodeSourcery/2010-09-51/
CROSS_PATH = "C:\Tools\GNU Tools ARM Embedded\6 2017-q1-update/"

CROSS = $(CROSS_PATH)/bin/arm-none-eabi-

INCLUDES += -I$(CROSS_PATH)/arm-none-eabi/include
INCLUDES += -I$(CROSS_PATH)/arm-none-eabi/include/lib

INCLUDES += -ISystem
VPATH += System

#-------------------- ����� ���������� ---------------------
FLAGS_C  = $(INCLUDES) -I.
FLAGS_C += -Os$(OPTIMIZATION) # ����������� �� �������
FLAGS_C += -Wall # ��������� ����� ��������� � ��������������� � �������
FLAGS_C += -c # ������� �� �������� ������ ��������� .o
FLAGS_C += -fno-builtin
# ��������� ������ ������� � ������� ������ � ��������� ������ ��������� �����, 
# ���� ������� ������ ������������ ������������ ������� (2 ������)
FLAGS_C += -ffunction-sections
FLAGS_C += -fdata-sections
FLAGS_C += -msoft-float # ����������� �������� ������ � ��������� �������
FLAGS_C += -mtune=cortex-m3 # ��� ��������� ������������������ ����
FLAGS_C += -mcpu=cortex-m3 # ��� ���������� ARM
FLAGS_C += -mthumb # ���������� ��� ��� 16 ������� ������ ���������� Thumb
FLAGS_C += -gdwarf-2 # � target.lss �� ������ + �� ���������� � ASM

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
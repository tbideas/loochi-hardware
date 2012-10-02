

# Target board information
F_CPU = 8000000
MCU = attiny861
ARCH = AVR8

# AVRdude parameters
AVRDUDE_PROGRAMMER = usbtiny

##  Fuse config
# 0xFF is the default value (or 0x01 the other bits are not used)- self programming disabled
FUSE_EXTENDED = 0x01
# Default is 0xDF
FUSE_HIGH     = 0xDF
# Default is 0x62 - But we do not want the DIV8
FUSE_LOW      = 0xE2

# Optimization level, can be [0, 1, 2, 3, s].
#     0 = turn off optimization. s = optimize for size.
#     (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
OPT=s

# C standard
CSTANDARD = -std=c99

CDEFS = -DF_CPU=$(F_CPU)UL -DARCH=$(ARCH)

CFLAGS += $(CDEFS)
CFLAGS += -O$(OPT)
CFLAGS += -funsigned-char
CFLAGS += -funsigned-bitfields
CFLAGS += -ffunction-sections
CFLAGS += -fno-inline-small-functions
CFLAGS += -fpack-struct
CFLAGS += -fshort-enums
CFLAGS += -fno-strict-aliasing
CFLAGS += -Wall
CFLAGS += -Wstrict-prototypes
#CFLAGS += -mshort-calls
#CFLAGS += -fno-unit-at-a-time
#CFLAGS += -Wundef
#CFLAGS += -Wunreachable-code
#CFLAGS += -Wsign-compare
#CFLAGS += -Wa,-adhlns=$(<:%.c=$(OBJDIR)/%.lst)
#CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += $(CSTANDARD)

ALL_CFLAGS = -mmcu=$(MCU) -I. $(CFLAGS)

CC = avr-gcc
AVRDUDE = avrdude
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
RM = rm -f


TARGET=pwmadc
SRC = main.c adc.c usi.c pwm.c

##### Do not edit below this line ######


OBJ = $(SRC:%.c=%.o)


all: build program

build: $(TARGET).hex $(TARGET).lss
	
$(TARGET).o: $(TARGET).c
	$(CC) $(ALL_CFLAGS) -c $(TARGET).c

%.o: %.c
	$(CC) -c $(ALL_CFLAGS) $< -o $@
	
$(TARGET).elf: $(OBJ)
	$(CC) -g -mmcu=$(MCU) -o $(TARGET).elf $(OBJ)
	
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex

# Create extended listing file from ELF output file.
%.lss: %.elf
	$(OBJDUMP) -h -S -z $< > $@

program: $(TARGET).hex
	$(AVRDUDE) -c $(AVRDUDE_PROGRAMMER) -p $(MCU) -U flash:w:$(TARGET).hex

clean:
	$(RM) $(OBJ) $(TARGET).elf $(TARGET).hex $(TARGET).lss

fuse:
	$(AVRDUDE) -c $(AVRDUDE_PROGRAMMER) -p $(MCU) -U efuse:w:$(FUSE_EXTENDED):m -U lfuse:w:$(FUSE_LOW):m -U hfuse:w:$(FUSE_HIGH):m
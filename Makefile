# no HAL, no IDE; gcc, a linker script and the reference manual

TARGET  = firmware
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

# soft float for now; the FPU needs turning on in startup before hard float is safe
CPU     = -mcpu=cortex-m4 -mthumb

CFLAGS  = $(CPU) -std=c11 -Wall -Wextra -O0 -g3 -ffreestanding \
          -ffunction-sections -fdata-sections -Iinclude
LDFLAGS = $(CPU) -T linker/stm32f407.ld -nostartfiles \
          -Wl,--gc-sections -Wl,-Map=build/$(TARGET).map

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,build/%.o,$(SRCS))

all: build/$(TARGET).elf build/$(TARGET).bin

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@
	$(SIZE) $@

build/$(TARGET).bin: build/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

build:
	mkdir -p build

run: build/$(TARGET).elf
	renode renode/run.resc

clean:
	rm -rf build

.PHONY: all run clean

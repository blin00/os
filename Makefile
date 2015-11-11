C_SRCS=$(wildcard src/*.c)
ASM_SRCS=$(wildcard src/*.s)
OBJECTS=$(C_SRCS:.c=.o) $(ASM_SRCS:.s=.o)
DEPS=$(patsubst src/%.c,dep/%.d,$(C_SRCS))
CC=i686-elf-gcc
CFLAGS=-ffreestanding -fno-stack-protector -mpreferred-stack-boundary=2 -Wall -Wextra -O3 -pipe
LD=i686-elf-gcc
LDFLAGS=-T link.ld -ffreestanding -nostdlib -lgcc -O3 -pipe
AS=nasm
ASFLAGS=-f elf32

.PHONY: all kernel iso run bochs clean

all: kernel iso

kernel: kernel.elf

iso: os.iso

kernel.elf: $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o kernel.elf

os.iso: kernel.elf
	cp kernel.elf iso/boot/kernel.elf
	grub-mkrescue -d /usr/lib/grub/i386-pc -o os.iso iso

run: os.iso
	qemu-system-i386 -cdrom os.iso

bochs: os.iso
	bochs -q

clean:
	rm -rf src/*.o dep *.iso *.elf iso/boot/*.elf

%.o: %.c
	@mkdir -p dep
	$(CC) -MMD -MP -MF $(patsubst src/%.o,dep/%.d,$@) -c $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

-include $(DEPS)

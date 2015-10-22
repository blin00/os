OBJECTS=boot.o kernel.o io.o util.o interrupt.o interrupt_asm.o string.o memory.o
CC=i686-elf-gcc
CFLAGS=-c -ffreestanding -fno-stack-protector -Wall -Wextra -O3
LD=i686-elf-gcc
LDFLAGS=-T link.ld -ffreestanding -O3 -nostdlib -lgcc
AS=nasm
ASFLAGS=-f elf32

all: kernel.elf os.iso

kernel.elf: $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o kernel.elf

os.iso: kernel.elf
	cp kernel.elf iso/boot/kernel.elf
	grub-mkrescue -d /usr/lib/grub/i386-pc -o os.iso iso

run: kernel.elf
	qemu-system-i386 -kernel kernel.elf

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf *.o *.iso *.elf iso/boot/*.elf

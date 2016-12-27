Unnamed 32-bit protected mode OS under development.

## Build Requirements
* i686-elf toolchain
    * See http://wiki.osdev.org/GCC_Cross-Compiler
    * Currently working with gcc 6.3.0, binutils 2.27
* nasm
* grub-mkrescue (to build a bootable ISO, optional)

## Features
* [Fortuna](https://www.schneier.com/academic/fortuna/) CSPRNG
* Basic heap memory management
* Multithreading

## Notes
* Fortuna implementation has not been verified to be secure
* Constant-time AES implementation from https://github.com/bitcoin-core/ctaes (MIT)
* SHA256 implementation from https://github.com/B-Con/crypto-algorithms (public domain)

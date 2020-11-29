#/cygdrive/c/Users/marti/Nextcloud/Coden/tas/misc/qemu_cortexa_mmu

CFLAGS="-mcpu=cortex-a9 -ggdb -std=c99 -marm"
LDFLAGS="-nostartfiles -nodefaultlibs -Map=out/program.map --script=linker.ld"

arm-none-eabi-as -c -o out/startup.o startup.s
arm-none-eabi-gcc $CFLAGS -c -o out/main.o main.c
arm-none-eabi-ld $LDFLAGS -o out/program.elf out/startup.o out/main.o

arm-none-eabi-objdump -D out/program.elf > out/program.dis
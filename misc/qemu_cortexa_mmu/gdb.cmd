#set confirm off
file out/program.elf
dir ./

target remote 127.0.0.1:1234

#layout regs
#layout src

set disassemble-next-line on

break enable_mmu
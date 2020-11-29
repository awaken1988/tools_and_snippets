MEMORY
{
  RAM (rwx) : org = 0x48000000, l = 16M
}

SEARCH_DIR(out)
ENTRY(reset_entry)



SECTIONS
{
  .text : { *(.text.irqtable) }  

  .text : { *(.text) }          >RAM

  .data : { *(.data) }          >RAM

  .bss  (NOLOAD) :
  {
    . = ALIGN(32);
    _bss_start = .;
    *(.bss) *(COMMON)
    . = ALIGN(8);
    _bss_end = .;
  } >RAM

    .stack  (NOLOAD) : 
  {
    . = ALIGN(32);
    . += 4096;
    stack_top = ALIGN(32);
  }  >RAM


  .heap  (NOLOAD) :
  {
    . = ALIGN(0x100000);
    heap_start = .;
    . += 0x100000;
    heap_end = .;
  }  >RAM


 
}
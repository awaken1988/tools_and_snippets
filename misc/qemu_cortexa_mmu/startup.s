.global reset_entry
.global _start

.section .text.irqtable
exception_table_start:
    b reset_entry
    b exception_undef_instr /* 0x4  Undefined Instruction */
    b exception_software_irq /* 0x8  Software Interrupt */
    b exception_prefetch_abort /* 0xC  Prefetch Abort */
    b exception_data_abort /* 0x10 Data Abort */
    b exception_reserverd /* 0x14 Reserved */
    b exception_irq /* 0x18 IRQ */
    b exception_fiq /* 0x1C FIQ */

exception_undef_instr:
    b .
exception_software_irq:
    b .
exception_prefetch_abort:
    b .
exception_data_abort:
    b .
exception_reserverd:
    b .
exception_irq:
    b .
exception_fiq:
    b . 

_start:
    nop
reset_entry:
    ldr r0, =#stack_top
    mov sp, r0

    ldr r0, =exception_table_start
    MCR p15, 0, r0, c12, c0, 0 // Write Rt to VBAR

    ldr r0, =_bss_start
	ldr r1, =_bss_end
    ldr r2, =#0
	bss_clear_loop:
		str r2, [r0], #8
		cmp r0, r1
		bne bss_clear_loop

    bl main
    b  .
    
    
str1: .word 0xDEADBEEF






.end

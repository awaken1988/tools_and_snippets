//references:
//  https://stackoverflow.com/questions/23328471/problems-enabling-mmu-on-arm-cortex-a
//  https://stackoverflow.com/questions/37495516/arm-setup-cache-and-mmu-from-armasm-to-gnu-arm-assembler

typedef unsigned int uint32_t;

#define MMU_TABLE_LVL1_ENTRIES 4096

uint32_t mmu_lvl1[MMU_TABLE_LVL1_ENTRIES] __attribute__((aligned(16*1024))) ;

void mmu_init()
{
    for(uint32_t i_sec=0; i_sec<MMU_TABLE_LVL1_ENTRIES; i_sec++) {
        uint32_t section = 0;
        section |= 0x2;
        section |= i_sec << 20; //Section base address
        section |= 0x0 << 3;    //C
        section |= 0x0 << 2;    //B
        section |= 0xf<<5;      //domain
        section |= 0x3<<10;     //AP[0:1]

        mmu_lvl1[i_sec] = section;
    }



    uint32_t tmp=0;

    __asm__ volatile("push {r0,r1}");
    __asm__ volatile("MRC p15, 0, r1, c1, c0, 0" : : :);    //
    __asm__ volatile("BIC r1, r1, #(0x1 << 12)" : : :);     //     @ Disable Instruction cache
    __asm__ volatile("BIC r1, r1, #(0x1 << 2)" : : :);      //      @ Disable Data cache
    __asm__ volatile("MCR p15, 0, r1, c1, c0, 0" : : :);    //    
    __asm__ volatile("mcr p15, 0, r1, c8, c7, 0" : : :);    //    @ Invalidate TLB
    __asm__ volatile("mov r0, #0xffffffff" : : :);          //
    __asm__ volatile("mcr p15, 0, r0, c3, c0, 0" : : :);    //    @ Set DACR to all "manager" - no permissions checking
    __asm__ volatile("dsb");
    __asm__ volatile("isb");
    __asm__ volatile("pop {r0,r1}");


    //TTBCR
    {
         uint32_t ttbcr = 0;
        __asm__ volatile("MRC p15, 0, %[ttbcr], c2, c0, 2 " : [ttbcr] "=r" (ttbcr) : :  );
        ttbcr &= ~( (0x1<<31) | (0x3f<<0) );
        __asm__ volatile("MCR p15, 0,  %[ttbcr], c2, c0, 2"  : : [ttbcr] "r" (ttbcr) : );
    }
    
    //TTBR
    {
        uint32_t ttbr = 0;
        ttbr |= ((uint32_t)mmu_lvl1);

        __asm__ volatile("MCR p15, 0, %[ttbr], c2, c0, 0"  : : [ttbr] "r" (ttbr) : );
    }


    //enable MMU
    __asm__ volatile("push {r0,r1}");
    __asm__ volatile("MRC p15, 0, R1, c1, C0, 0"); // ;Read control register
    __asm__ volatile("ORR R1, #0x1"); //  ;Set M bit
    __asm__ volatile("ORR R1, #(0x1<<2)"); //   ;Enable D Cache
    __asm__ volatile("ORR R1, #(0x1<<11)"); //  ;Enable I Cache
    __asm__ volatile("MCR p15, 0,R1,C1, C0,0"); //  ;Write control register and enable MMU
    __asm__ volatile("dsb");
    __asm__ volatile("isb");
    __asm__ volatile("pop {r0,r1}");
}


int main()
{
    mmu_init();

    int x = 0;
    x++;

    while(1);
    return 0;
}

void exit (int status)
{
    while(1);
}
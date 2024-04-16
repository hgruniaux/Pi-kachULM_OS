#include "mmu.h"
#include <stddef.h>
#include <stdint.h>

#define PAGE_LENGTH (PAGE_SIZE / sizeof(uint64_t))
#define PAGESIZE 4096

// descriptor
#define PT_PAGE 0b11
#define PT_BLOCK 0b01

// accessibility
#define PT_KERNEL (0 << 6)  // privileged, supervisor EL1 access only
#define PT_USER (1 << 6)    // unprivileged, EL0 access allowed
#define PT_RW (0 << 7)      // read-write
#define PT_RO (1 << 7)      // read-only
#define PT_AF (1 << 10)     // accessed flag
#define PT_NX (1UL << 54)   // no execute
// shareability
#define PT_OSH (2 << 8)  // outter shareable
#define PT_ISH (3 << 8)  // inner shareable

// defined in MAIR register
#define PT_MEM (0 << 2)  // normal memory
#define PT_DEV (1 << 2)  // device MMIO
#define PT_NC (2 << 2)   // non-cachable

#define TTBR_CNP 1

/**
 * Set up page translation tables and enable virtual memory
 */
void mmu_init(volatile uint64_t* ttbr0) {
  volatile uint64_t* ttbr0_lvl2 = (uint64_t*)ttbr0 + PAGESIZE;

  ttbr0[0] = (uintptr_t)ttbr0_lvl2 | PT_PAGE;  // physical address

  for (uint64_t i = 0; i < PAGE_LENGTH; ++i) {
    ttbr0_lvl2[i] = (i << 30) |  // physical address
                    PT_BLOCK |   // map 2M block
                    PT_AF |      // accessed flag
                    PT_DEV;
  }

  /* okay, now we have to set system registers to enable MMU */

  // first, set Memory Attributes array, indexed by PT_MEM, PT_DEV, PT_NC in our example
  {
    uint64_t r = (0xFF << 0) |  // AttrIdx=0: normal, IWBWA, OWBWA, NTR
                 (0x04 << 8) |  // AttrIdx=1: device, nGnRE (must be OSH too)
                 (0x44 << 16);  // AttrIdx=2: non cacheable
    asm volatile("msr mair_el1, %0" : : "r"(r));
  }

  // next, specify mapping characteristics in translate control register
  {
    uint64_t r = (0b00LL << 37) |   // TBI=0, no tagging
                 (0b101LL << 32) |  // IPS=autodetected
                 (0b10LL << 30) |   // TG1=4k
                 (0b11LL << 28) |   // SH1=3 inner
                 (0b01LL << 26) |   // ORGN1=1 write back
                 (0b01LL << 24) |   // IRGN1=1 write back
                 (0b0LL << 23) |    // EPD1 enable higher half
                 (16LL << 16) |     // T1SZ=16, 4 levels (256TB)
                 (0b00LL << 14) |   // TG0=4k
                 (0b11LL << 12) |   // SH0=3 inner
                 (0b01LL << 10) |   // ORGN0=1 write back
                 (0b01LL << 8) |    // IRGN0=1 write back
                 (0b0LL << 7) |     // EPD0 enable lower half
                 (16LL << 0);       // T0SZ=16, 4 levels (256TB)
    asm volatile("msr tcr_el1, %0; isb" : : "r"(r));
  }

  // tell the MMU where our translation tables are. TTBR_CNP bit not documented, but required
  {
    // lower half, user space
    asm volatile("msr ttbr0_el1, %0" : : "r"((unsigned long)ttbr0 + TTBR_CNP));
    // upper half, kernel space
    asm volatile("msr ttbr1_el1, %0" : : "r"((unsigned long)ttbr0 + TTBR_CNP));
  }

  // finally, toggle some bits in system control register to enable page translation
  {
    uint64_t r;
    asm volatile("dsb ish; isb; mrs %0, sctlr_el1" : "=r"(r));
    r |= 0xC00800;      // set mandatory reserved bits
    r &= ~((1 << 25) |  // clear EE, little endian translation tables
           (1 << 24) |  // clear E0E
           (1 << 19) |  // clear WXN
           (1 << 12) |  // clear I, no instruction cache
           (1 << 4) |   // clear SA0
           (1 << 3) |   // clear SA
           (1 << 2) |   // clear C, no cache at all
           (1 << 1));   // clear A, no aligment check
    r |= (1 << 0);      // set M, enable MMU
    asm volatile("msr sctlr_el1, %0; isb" : : "r"(r));
  }
}

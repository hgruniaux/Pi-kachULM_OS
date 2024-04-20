#include "mmu_defs.hpp"

#include "libk/utils.hpp"
#include "mmu_table.hpp"

#define resolve_symbol_pa(symbol)                     \
  ({                                                  \
    uintptr_t __dest;                                 \
    asm volatile("adr %x0, " #symbol : "=r"(__dest)); \
    __dest;                                           \
  })

#define resolve_symbol_va(symbol)                      \
  ({                                                   \
    uintptr_t __dest;                                  \
    asm volatile("ldr %x0, =" #symbol : "=r"(__dest)); \
    __dest;                                            \
  })

static inline constexpr PagesAttributes kernel_code = {.sh = Shareability::InnerShareable,
                                                       .exec = ExecutionPermission::PrivilegedExecute,
                                                       .rw = ReadWritePermission::ReadOnly,
                                                       .access = Accessibility::Privileged,
                                                       .type = MemoryType::Normal};

static inline constexpr PagesAttributes rw_memory = {.sh = Shareability::InnerShareable,
                                                     .exec = ExecutionPermission::NeverExecute,
                                                     .rw = ReadWritePermission::ReadWrite,
                                                     .access = Accessibility::Privileged,
                                                     .type = MemoryType::Normal};

static inline constexpr PagesAttributes ro_memory = {.sh = Shareability::InnerShareable,
                                                     .exec = ExecutionPermission::NeverExecute,
                                                     .rw = ReadWritePermission::ReadOnly,
                                                     .access = Accessibility::Privileged,
                                                     .type = MemoryType::Normal};

static inline constexpr PagesAttributes device_memory = {.sh = Shareability::OuterShareable,
                                                         .exec = ExecutionPermission::NeverExecute,
                                                         .rw = ReadWritePermission::ReadWrite,
                                                         .access = Accessibility::Privileged,
                                                         .type = MemoryType::Device_nGnRnE};

void setup_initial_mapping(MMUTable* tbl, const uint32_t* dtb) {
  const auto dtb_start = PhysicalPA((uintptr_t)dtb & ~(PAGE_SIZE - 1));
  const size_t dtb_size = libk::from_be(dtb[1]);
  const auto dtb_stop = PhysicalPA(libk::align((uintptr_t)dtb + dtb_size, PAGE_SIZE));

  {
    // Mapping memory before kernel
    const auto va_start = KERNEL_BASE;
    const auto va_end = VirtualPA(resolve_symbol_va(_stext) - PAGE_SIZE);
    const auto pa_start = PhysicalPA(0);
    if (!map_range(tbl, va_start, va_end, pa_start, rw_memory)) {
      libk::halt();
    }
  }

  {
    // Mapping kernel code segment
    const auto va_start = VirtualPA(resolve_symbol_va(_stext));
    const auto va_end = VirtualPA(resolve_symbol_va(_srodata) - PAGE_SIZE);
    const auto pa_start = PhysicalPA(resolve_symbol_pa(_stext));
    if (!map_range(tbl, va_start, va_end, pa_start, kernel_code)) {
      libk::halt();
    }
  }

  {
    // Mapping read-only segment
    const auto va_start = VirtualPA(resolve_symbol_va(_srodata));
    const auto va_end = VirtualPA(resolve_symbol_va(_srwdata) - PAGE_SIZE);
    const auto pa_start = PhysicalPA(resolve_symbol_pa(_srodata));
    if (!map_range(tbl, va_start, va_end, pa_start, ro_memory)) {
      libk::halt();
    }
  }

  {
    // Mapping read-write segment
    // Goes from the _srwdata symbol to the start of the DeviceTree
    const auto va_start = VirtualPA(resolve_symbol_va(_srwdata));
    // Convert the physical address of the DTB to a virtual one
    const auto va_end = VirtualPA(KERNEL_BASE + dtb_start - PAGE_SIZE);
    const auto pa_start = PhysicalPA(resolve_symbol_pa(_srwdata));
    if (!map_range(tbl, va_start, va_end, pa_start, rw_memory)) {
      libk::halt();
    }
  }

  {
    // Mapping DeviceTree
    // Convert the physical addresses of the DTB to virtual ones
    const auto va_start = VirtualPA(KERNEL_BASE + dtb_start);
    const auto va_end = VirtualPA(KERNEL_BASE + dtb_stop);
    if (!map_range(tbl, va_start, va_end, dtb_start, ro_memory)) {
      libk::halt();
    }
  }

  {
    const auto va_start = VirtualPA(KERNEL_BASE + dtb_stop + PAGE_SIZE);
    const auto va_end = VirtualPA(KERNEL_BASE + 0xffffff000);
    if (!map_range(tbl, va_start, va_end, dtb_stop + PAGE_SIZE, rw_memory)) {
      libk::halt();
    }
  }
}

void inline setup_mair() {
  static constexpr uint64_t device_nGnRnE_mair = 0b00000000;
  static constexpr uint64_t device_nGRE_mair = 0b00001000;
  static constexpr uint64_t normal_no_cache = 0b01000100;
  static constexpr uint64_t normal = 0b11001100;

  static constexpr uint64_t r = (device_nGnRnE_mair << 8 * ((size_t)MemoryType::Device_nGnRnE)) |
                                (device_nGRE_mair << 8 * ((size_t)MemoryType::Device_nGRE)) |
                                (normal_no_cache << 8 * ((size_t)MemoryType::Normal_NoCache)) |
                                (normal << 8 * ((size_t)MemoryType::Normal));
  asm volatile("msr mair_el1, %0" : : "r"(r));
}

void inline setup_tcr() {
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

void inline setup_ttbr0_ttbr1(MMUTable* tbl) {
  static constexpr uint64_t TTBR_CNP = 0x1;

  // lower half, user space
  asm volatile("msr ttbr0_el1, %0" : : "r"(tbl->pgd + TTBR_CNP));
  // upper half, kernel space
  asm volatile("msr ttbr1_el1, %0" : : "r"(tbl->pgd + TTBR_CNP));

  asm volatile("dsb ish; isb;");
}

void inline setup_sctlr() {
  uint64_t r;
  asm volatile("mrs %0, sctlr_el1" : "=r"(r));
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

  /* TODO :
   * - Set UWXN and WXN to 0 in SCTLR_EL1
   * - Set TCR2_EL1 to 0
   * - Set TCR_EL1.AS to 0 (ASID of 8bit)
   * - Set TCR_EL1.A1 to 0 (The ASID is defined by TTBR0_EL1, ie. the process)
   *
   * And an important thing, be careful with identity mapping when you reach MMIO_BASE. You'll have to use a different
   * AttrIndex and shareability. For normal memory it should be inner shareable and AttrIndex should point to a MAIR
   * value of 0xCC or 0xFF (depending on you want alloc or not), while for device memory it must be outer shareable and
   * AttrIndex must point to a MAIR value of 0x04 in order to work properly.
   */

  asm volatile("msr sctlr_el1, %0" : : "r"(r));
  asm volatile("isb");
}

struct MMUTableHandleData {
  const size_t first_page;
  const size_t upper_bound;
  size_t nb_allocated;
};

extern "C" bool alloc_page(void* handle_ptr, VirtualPA* page) {
  auto* handle = (MMUTableHandleData*)handle_ptr;

  if (handle->first_page + PAGE_SIZE * (handle->nb_allocated + 1) < handle->upper_bound) {
    *page = VirtualPA(handle->first_page + PAGE_SIZE * handle->nb_allocated++);
    return true;
  }

  return false;
}

extern "C" bool resolve_pa(void*, PhysicalPA va, VirtualPA* pa) {
  // In our case, Virtual Space == Physical Space !
  *pa = va;
  return true;
}

extern "C" bool resolve_va(void*, PhysicalPA pa, VirtualPA* va) {
  // In our case, Virtual Space == Physical Space !
  *va = pa;
  return true;
}

extern "C" void mmu_init(const uint32_t* dtb) {
  const auto alloc_start = PhysicalPA(resolve_symbol_pa(_kend));
  const auto dtb_start = PhysicalPA((uintptr_t)dtb & ~(PAGE_SIZE - 1));

  MMUTableHandleData handle = {
      .first_page = alloc_start,
      .upper_bound = dtb_start,
      .nb_allocated = 0,
  };

  VirtualPA pgd;
  if (!alloc_page((void*)&handle, &pgd)) {
    libk::halt();
  }

  MMUTable tbl = {
      .kind = MMUTable::Kind::Kernel,
      .pgd = pgd,
      .asid = 0,
      .handle = (void*)&handle,
      .alloc = (AllocFun)resolve_symbol_pa(alloc_page),
      .free = nullptr,  // We don't free anything here !
      .resolve_pa = (ResolvePA)resolve_symbol_pa(resolve_pa),
      .resolve_va = (ResolveVA)resolve_symbol_pa(resolve_va),
  };

  setup_initial_mapping(&tbl, dtb);

  setup_mair();
  setup_tcr();
  setup_ttbr0_ttbr1(&tbl);
  setup_sctlr();
}

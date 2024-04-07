#include <cstddef>
#include <cstdint>
#include "mmu_kernel.hpp"
#include "libk/string.hpp"



BitArray::BitArray(void* begin, size_t taille) : m_array((const uint8_t*) begin)
{
    // Initialisation a 0
    if(begin != nullptr)
    {
        bzero(m_array, taille/8);
    }
}

bool BitArray::get_bit(size_t index) const
{
    return m_array[index / (8 * sizeof(uint8_t))] & (1u << (index % (8*sizeof(uint8_t))));
}

void BitArray::set_bit(size_t index, bool value)
{
    const uint8_t v = m_array[index / (8 * sizeof(uint8_t))];

    if(value)
    {
        m_array[index / (8 * sizeof(uint8_t))] = v | (1u << (index % (8*sizeof(uint8_t))));
    }

    else
    {
        m_array[index / (8 * sizeof(uint8_t))] = v & ~(1u << (index % (8*sizeof(uint8_t))));
    }
}



PageAlloc::PageAlloc(uint64_t memsize) : m_memsize(memsize) , m_pagequant(memsize/PAGESIZE) , memory_needed(m_pagequant/4) , mmap(nullptr,0)
{}

void PageAlloc::setmmap(void* array)
{
    mmap = BitArray(array, memory_needed);
}

// void PageAlloc::setPAGESIZE(uint64_t PAGESIZE)
// {
//     this->PAGESIZE = PAGESIZE;
// }

// void PageAlloc::setMEMSIZE(uint64_t MEMSIZE)
// {
//     this -> MEMSIZE = MEMSIZE;
// }

// uint64_t PageAlloc::getPAGESIZE() const
// {
//     return this -> PAGESIZE;
// }

// uint64_t PageAlloc::getMEMSIZE() const
// {
//     return this -> MEMSIZE;
// }

// bool* PageAlloc::setmmap()
// {
//     std::array<bool, PAGEQUANT> mmap;
//     mmap.fill(true);
//     this -> mmap = mmap;
// }





// /*
//         KERNEL PAGE ALLOC    
// */


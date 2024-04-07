#ifndef MMU_KERNEL
#define MMU_KERNEL

#include "stdint.h"
#define PAGESIZE 4096

typedef void* physical_address;

class BitArray 
{

    public:

        // Constructeurs
        explicit BitArray(void* begin, size_t bit_taille);
        bool get_bit(size_t index) const;
        void set_bit(size_t index, bool value);

        // Opérateurs
        bool operator[](size_t index) const
        {
            return get_bit(index);
        }

    private:

        uint8_t* m_array;

};

class PageAlloc
{

    public:

        // Constructeur
        PageAlloc(uint64_t memsize);

        // Utility functions
        void setmmap(void* array);
        void mark_as_used(physical_address addr);
        bool freshpage(physical_address* addr);
        void freepage(physical_address addr);
        bool page_status(physical_address addr);

        const uint64_t memory_needed;
        
    protected:

        BitArray mmap;
        const uint64_t m_memsize;
        const uint64_t m_pagequant;

};

#endif
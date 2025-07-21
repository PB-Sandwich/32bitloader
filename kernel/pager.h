
#pragma once

#include <stdint.h>

typedef struct {
    uint32_t present : 1;
    uint32_t writeable : 1;
    uint32_t userspace : 1;
    uint32_t write_through : 1;
    uint32_t disable_caching : 1;
    uint32_t accessed : 1;
    uint32_t reserved0 : 1; // has been written to
    uint32_t reserved1 : 1; // not sure what this one is, might be page attribute table bit
    uint32_t reserved2 : 1;
    uint32_t available_to_software : 3;
    uint32_t page_table_address : 20;
} PDEEntry;

typedef struct {
    uint32_t present : 1;
    uint32_t writeable : 1;
    uint32_t userspace : 1;
    uint32_t write_through : 1;
    uint32_t disable_caching : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1; // has been written to
    uint32_t pa : 1; // not sure what this one is, might be page attribute table bit
    uint32_t global : 1;
    uint32_t available_to_software : 3;
    uint32_t physical_page_address : 20;
} PTEEntry;

#define TABLE_ENTRIES_LENGTH 1024

typedef struct {
    PTEEntry entries[TABLE_ENTRIES_LENGTH];
} PTETable;

typedef struct {
    PDEEntry entries[TABLE_ENTRIES_LENGTH];
} PDETable;

typedef union {
    PTETable pte;
    PDETable pde;
} PageTable;

enum {
    PAGE_PRESENT = 1,
    PAGE_WRITEABLE = 1 << 1,
    PAGE_USERSPACE = 1 << 2,
    PAGE_WRITE_THROUGH = 1 << 3,
    PAGE_DISABLE_CACHING = 1 << 4,
    PAGE_ACCESSED = 1 << 5,
    PAGE_DIRTY = 1 << 6,
    PAGE_PA = 1 << 7,
    PAGE_GLOBAL = 1 << 8,
};

#define PAGER_ERROR (void*)-1
#define PAGE_SIZE 0x1000

void init_pager();

void load_page_table(PDETable* pde_table);

PageTable* create_new_table();

uintptr_t virt_to_phys(uintptr_t virtual_address, PDETable* table);

// use PAGER_ERROR as address for it to allocate any free page
// if requested page isn't available it will return PAGER_ERROR
// returns the new vitrual address
void* new_page(void* physical_address, PDETable* pde_table, uint32_t flags);
void free_page(void* virtual_address, PDETable* pde_table);

PageTable* soft_copy_table(PageTable* table, uint16_t number_of_entries);

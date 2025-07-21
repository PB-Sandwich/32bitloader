#include "pager.h"
#include <exit.h>
#include <heap.h>
#include <memutils.h>
#include <print.h>
#include <stdint.h>

void* page_tables_base = (void*)0x300000;
void* page_tables_limit = (void*)0x400000 - 1;
void* page_tables_end = (void*)0x300000;

uintptr_t virt_to_phys(uintptr_t virtual_address, PDETable* table)
{
    uint32_t physical_index = virtual_address & 0xfff;
    uint32_t pte_index = (virtual_address >> 12) & 0x3ff;
    uint32_t pde_index = (virtual_address >> 22) & 0x3ff;

    if (!table->entries[pde_index].present) {
        return (uintptr_t)PAGER_ERROR;
    }
    PTETable* sub_table = (PTETable*)(table->entries[pde_index].page_table_address << 12);
    if (!sub_table->entries[pte_index].present) {
        return (uintptr_t)PAGER_ERROR;
    }
    uintptr_t base_addr = sub_table->entries[pte_index].physical_page_address << 12;

    return base_addr + physical_index;
}

void* alloc_table()
{
    page_tables_end += sizeof(PageTable);
    return page_tables_end - sizeof(PageTable);
}

void free_table(PageTable* table)
{
    return;
}

struct FreeMemoryBlock {
    void* start;
    void* end;
    struct FreeMemoryBlock* next;
    struct FreeMemoryBlock* prev;
};

struct FreeMemoryBlock* first_pager_block = NULL;

void pager_remove(struct FreeMemoryBlock* entry, void* start, void* end)
{
    if (start == entry->start && end == entry->end) {
        first_pager_block = entry->next;
        first_pager_block->prev = NULL;
        free(entry);
        return;
    }
    if (start == entry->start) {
        entry->start = end + 1;
        return;
    }
    if (end == entry->end) {
        entry->end = start - 1;
        return;
    }

    struct FreeMemoryBlock* new_entry = malloc(sizeof(struct FreeMemoryBlock));
    if (new_entry == NULL) {
        printf("Failed to malloc in pager remove\n");
        exit_kernel();
    }

    new_entry->prev = entry;
    new_entry->next = entry->next;
    new_entry->start = end + 1;
    new_entry->end = entry->end;

    entry->next = new_entry;
    entry->end = start - 1;
}

void pager_fill(void* start, void* end)
{
    struct FreeMemoryBlock* entry = first_pager_block;

    while (entry->start < start) {
        entry = entry->next;
    }
    struct FreeMemoryBlock* entry_before = entry->prev;
    struct FreeMemoryBlock* entry_after = entry;

    if (entry_before->end + 1 == start && entry_before != NULL) {
        entry_before->end = end;
        return;
    }

    if (entry_after->start - 1 == end && entry_after != NULL) {
        entry_after->start = start;
        return;
    }

    struct FreeMemoryBlock* new_entry = malloc(sizeof(struct FreeMemoryBlock));
    if (new_entry == NULL) {
        printf("Failed to malloc in pager fill\n");
        exit_kernel();
    }

    new_entry->start = start;
    new_entry->end = end;
    new_entry->prev = entry_before;
    new_entry->next = entry_after;

    if (entry_before != NULL) {
        entry_before->next = new_entry;
    }
    if (entry_after != NULL) {
        entry_after->prev = new_entry;
    }

    if (entry_after == first_pager_block) {
        first_pager_block = new_entry;
    }
}

// returns the index into the last level of tables as a flat value
// ex: first free is PDE[1] PTE[5] -> 1 * 1024 + 5
uint32_t get_free_page_index(PageTable* table)
{
    for (int i = 0; i < TABLE_ENTRIES_LENGTH; i++) {

        if (!table->pde.entries[i].present) {
            return i * TABLE_ENTRIES_LENGTH;
        }

        PTETable* sub_table = (void*)(table->pde.entries[i].page_table_address << 12);

        for (int j = 0; j < TABLE_ENTRIES_LENGTH; j++) {
            if (!sub_table->entries[j].present) {
                return j + i * TABLE_ENTRIES_LENGTH;
            }
        }
    }
    return (uint32_t)PAGER_ERROR;
}

void load_page_table(PDETable* pde_table)
{
    __asm__ volatile(
        "mov %%cr3, %%eax\n\t"
        "and $0xfff, %%eax\n\t"
        "or  %0, %%eax\n\t"
        "mov %%eax, %%cr3\n\t"
        :
        : "r"(pde_table)
        : "eax");
}

void init_pager()
{
    first_pager_block = malloc(sizeof(struct FreeMemoryBlock));
    if (first_pager_block == NULL) {
        printf("failed to malloc in init pager\n");
        exit_kernel();
    }
    first_pager_block->next = NULL;
    first_pager_block->prev = NULL;
    first_pager_block->start = 0;
    first_pager_block->end = (void*)0xffffffff;
}

PageTable* create_new_table()
{
    PageTable* new_table = alloc_table();
    if (new_table == NULL) {
        return PAGER_ERROR;
    }

    memset(new_table, 0, sizeof(PageTable));
    return new_table;
}

void* new_page(void* physical_address, PDETable* pde_table, uint32_t flags)
{
    if (physical_address != PAGER_ERROR) {
        physical_address = (void*)((uint32_t)(physical_address + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));
        struct FreeMemoryBlock* entry = first_pager_block;
        pager_remove(entry, physical_address, physical_address + PAGE_SIZE - 1);
    } else {
        struct FreeMemoryBlock* entry = first_pager_block;
        physical_address = entry->start;
        pager_remove(entry, physical_address, physical_address + PAGE_SIZE - 1);
    }

    uint32_t index = get_free_page_index((PageTable*)pde_table);
    uint32_t pde_index = index / TABLE_ENTRIES_LENGTH;
    uint32_t pte_index = index % TABLE_ENTRIES_LENGTH;

    PDEEntry* pde_entry = &pde_table->entries[pde_index];

    PTETable* pte_table = (void*)(pde_entry->page_table_address << 12);
    if (!pde_entry->present) {
        pte_table = (PTETable*)create_new_table();
        if (pte_table == PAGER_ERROR) {
            printf("Failed to malloc aligned pte table in new page\n");
            exit_kernel();
        }

        pde_entry->present = 1;
        pde_entry->writeable = 1;
        pde_entry->write_through = 1;
        *(uint32_t*)pde_entry |= flags;
        pde_entry->page_table_address = (uint32_t)pte_table >> 12;
    }

    PTEEntry* pte_entry = &pte_table->entries[pte_index];

    pte_entry->physical_page_address = (uint32_t)physical_address >> 12;
    pte_entry->present = 1;
    pte_entry->writeable = 1;
    pte_entry->write_through = 1;
    *(uint32_t*)pte_entry |= flags;

    return (void*)((pde_index << 22) | (pte_index << 12));
}

void free_page(void* virtual_address, PDETable* pde_table)
{
    virtual_address = (void*)((uint32_t)(virtual_address + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)); // align address

    uint32_t pte_index = ((uintptr_t)virtual_address >> 12) & 0x3ff;
    uint32_t pde_index = ((uintptr_t)virtual_address >> 22) & 0x3ff;

    if (!pde_table->entries[pde_index].present) {
        printf("Invalid free on non present pde table index\n");
        return;
    }
    PTETable* pte_table = (PTETable*)(pde_table->entries[pde_index].page_table_address << 12);
    if (!pte_table->entries[pte_index].present) {
        printf("Invalid free on non present pte table index\n");
        return;
    }

    pager_fill((void*)virt_to_phys((uintptr_t)virtual_address, pde_table), (void*)virt_to_phys((uintptr_t)virtual_address, pde_table) + PAGE_SIZE - 1);
    pte_table->entries[pte_index].present = 0;
}

PageTable* soft_copy_table(PageTable* table, uint16_t number_of_entries)
{
    PageTable* new_table = create_new_table();
    if (new_table == PAGER_ERROR) {
        return PAGER_ERROR;
    }

    memcpy(new_table, table, sizeof(PTEEntry) * number_of_entries);
    return new_table;
}

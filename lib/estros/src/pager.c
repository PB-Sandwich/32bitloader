#include <estros/pager.h>
#include <stdint.h>

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

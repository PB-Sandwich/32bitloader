#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

uint8_t *heap = NULL;
uint32_t heap_size = 0;
BlockHeader *first_entry = NULL;

void init_heap(uint8_t *buffer, uint32_t size)
{
    heap = buffer;
    heap_size = size;
    first_entry = (BlockHeader *)heap;
    first_entry->size = heap_size - sizeof(BlockHeader);
    first_entry->free = 1;
    first_entry->next = NULL;
    first_entry->prev = NULL;
}

// size is the size the first block should be
void split(BlockHeader *entry, uint32_t size)
{
    uint32_t total_size = entry->size;
    uint32_t size2 = total_size - size - sizeof(BlockHeader);
    BlockHeader *old_next = entry->next;

    entry->size = size;
    entry->next = (BlockHeader *)((uint8_t *)(entry + 1) + size);

    BlockHeader *next = entry->next;
    next->size = size2;
    next->free = 1;
    next->prev = entry;
    next->next = old_next;
}

// p for previouse; n for next
BlockHeader *merge(BlockHeader *entry, uint8_t direction)
{
    BlockHeader *entry2 = NULL;
    switch (direction)
    {
    case 'p':
        entry2 = entry->prev;
        entry2->size += entry->size + sizeof(BlockHeader);
        entry2->next = entry->next;
        entry2->next->prev = entry2;
        return entry2;
        break;
    case 'n':
        entry2 = entry->next;
        entry->size += entry2->size + sizeof(BlockHeader);
        entry->next = entry2->next;
        entry->next->prev = entry;
        return entry;
        break;
    default:
        return NULL;
    }
}

void *malloc(uint32_t size)
{
    BlockHeader *current = first_entry;

    while (current != NULL)
    {
        if (current->free == 0)
        {
            current = current->next;
            continue;
        }
        if (current->size < size)
        {
            current = current->next;
            continue;
        }

        if (current->size > (size + sizeof(BlockHeader)) * 2)
        {
            split(current, size);
            current->free = 0;
            return (void *)(current + 1);
        }
        else
        {
            current->free = 0;
            return (void *)(current + 1);
        }
    }

    return NULL;
}

void *realloc(void *ptr, uint32_t size)
{
    BlockHeader *entry = (BlockHeader *)ptr - 1;

    if ((int32_t)size <= (int32_t)entry->size - (int32_t)sizeof(BlockHeader) * 2)
    {
        split(entry, size);
        BlockHeader *next = entry->next;
        if (next != NULL && next->next != NULL)
        {
            if (next->free == 1 && next->next->free == 1)
            {
                merge(next, 'n');
            }
        }
        return (void *)(entry + 1);
    }
    else if (size <= entry->size)
    {
        return (void *)(entry + 1);
    }

    if (entry->next != NULL && entry->next->free == 1)
    {
        if (size < entry->size + entry->next->size + sizeof(BlockHeader))
        {
            entry = merge(entry, 'n');
            split(entry, size);
            return (void *)(entry + 1);
        }
        if (entry->prev != NULL && entry->prev->free == 1)
        {
            if (size < entry->size + entry->next->size + entry->prev->size + sizeof(BlockHeader) * 2)
            {
                uint32_t old_size = entry->size;
                entry = merge(entry, 'n');
                entry = merge(entry, 'p');
                split(entry, size);
                entry->free = 0;
                memcpy((void *)(entry + 1), ptr, old_size);
                return (void *)(entry + 1);
            }
        }
    }
    if (entry->prev != NULL && entry->prev->free == 1)
    {
        if (size < entry->size + entry->prev->size + sizeof(BlockHeader))
        {
            uint32_t old_size = entry->size;
            entry = merge(entry, 'p');
            split(entry, size);
            entry->free = 0;
            memcpy((void *)(entry + 1), ptr, old_size);
            return (void *)(entry + 1);
        }
    }

    uint8_t *new_ptr = (uint8_t *)malloc(size);
    if (new_ptr == NULL)
    {
        return NULL;
    }

    memcpy(new_ptr, ptr, entry->size);

    free((void *)(entry + 1));

    return new_ptr;
}

void free(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }

    BlockHeader *entry = (BlockHeader *)ptr - 1;
    entry->free = 1;
    while (entry->prev != NULL && entry->prev->free == 1)
    {
        entry = merge(entry, 'p');
    }
    while (entry->next != NULL && entry->next->free == 1)
    {
        entry = merge(entry, 'n');
    }
}

int atoi(char *buffer)
{
    int result = 0;
    int sign_set = 0;
    int sign = 1;
    while (*buffer != '\0')
    {
        if (isdigit(*buffer))
        {
            sign_set = 1;
            result += (*buffer) - '0';
            result *= 10;
        }
        if (!sign_set && *buffer == '-')
        {
            sign = -1;
        }
        buffer++;
    }
    return (result / 10) * sign;
}

char *itoa(int value, char *buffer, int radix)
{
    if (value == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return buffer;
    }
    unsigned v;
    char temp[34] = {[33] = '\0'};
    int32_t i = 0;
    int sign = (value < 0 && radix == 10);
    if (sign)
    {
        v = -value;
    }
    else
    {
        v = (unsigned)value;
    }
    for (; v != 0; i++, v /= radix)
    {
        int t = v % radix;
        temp[i] = ((t < 10) ? (t + '0') : (t + 'A' - 10));
    }
    if (sign)
    {
        temp[i++] = '-';
    }
    int32_t len = i;

    for (; i >= 0; i--)
    {
        buffer[i] = temp[len - i - 1];
    }
    buffer[len] = '\0';
    return buffer;
}

char *int_to_str(int32_t value, char *buffer, int32_t radix, uint32_t *num_len)
{
    if (value == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        if (num_len != NULL)
        {
            *num_len = 1;
        }
        return buffer;
    }
    char temp[34] = {[33] = '\0'};
    int32_t i = 0;
    int sign = (value < 0);
    if (sign)
    {
        value = -value;
    }
    for (; value != 0; i++, value /= radix)
    {
        int t = value % radix;
        temp[i] = ((t < 10) ? (t + '0') : (t + 'A' - 10));
    }
    if (sign)
    {
        temp[i++] = '-';
    }
    int32_t len = i;

    for (; i >= 0; i--)
    {
        buffer[i] = temp[len - i - 1];
    }
    buffer[len] = '\0';
    if (num_len != NULL)
    {
        *num_len = (uint32_t)len;
    }
    return buffer;
}

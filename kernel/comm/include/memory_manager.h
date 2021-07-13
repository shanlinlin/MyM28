#ifndef memory_manager__
#define memory_manager__

#include "debug.h"
#include <stdint.h>
#include <stdio.h>

#ifndef DEBUG_LEVEL_HEAP
#define DEBUG_LEVEL_HEAP 2
#endif

//void* byteset(void* sa, size_t so, int set, size_t n);
//void* bytecpy(void* desa, size_t deso, void const* srca, size_t srco, size_t len);

char*    void2str(void const* s, int o);
unsigned strlen_void(void const* s, int offset);
unsigned strncpy_void(void* des, int offset, char const* src, unsigned size);

void memory_manager_init(void* addr, uintptr_t size);

#if (DEBUG && (DEBUG_LEVEL > DEBUG_LEVEL_HEAP))
#define MALLOC(size_) debug_malloc(size_, __LINE__, __FUNCTION__)
#define FREE(addr_)                                \
    do {                                           \
        debug_free(addr_, __LINE__, __FUNCTION__); \
        addr_ = NULL;                              \
    } while (0)
void* debug_malloc(uintptr_t size, unsigned line, char const* func);
void  debug_free(void const* addr, unsigned line, char const* func);

#else
#define MALLOC memory_malloc
#define FREE memory_free
void* memory_malloc(uintptr_t size);
void  memory_free(void const* addr);
#endif

void     BYTE_memset(void* addr, size_t offset, uint16_t data, size_t len);
void     BYTE_memcpy(void* desa, size_t deso, void const* srca, size_t srco, size_t len);
uint16_t BYTE_get(void const* addr, size_t offset);
void     BYTE_set(void* addr, size_t offset, uint16_t data);

#endif

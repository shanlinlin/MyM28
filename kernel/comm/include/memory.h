#ifndef component_memory__
#define component_memory__

#include <stdint.h>

typedef struct memory__s_manager* memory_p_manager;

/**
 * 初始化一块内存空间，并得到该内存空间的内存管理器
 * @param memory 一块内存空间
 * @param size 这块内存的总大小
 * @return 内存管理器
 */
memory_p_manager memory_init(void* memory, uintptr_t size);

/**
 * 获取分片申请出来的内存块的下一块
 * @param address 当前块的首地址
 * @return 下一块的首地址
 */
void* memory_next(void* address);

/**
 * 获取从当前块开始计算的总大小
 * @param address 申请到的内存的块地址
 * @return 返回从这块开始计算的总大小
 */
uintptr_t memory_size(void* address);

/**
 * 申请连续的内存空间
 * @param manager 内存管理器
 * @param size 想要申请的空间的大小
 * @return 如果申请成功，则返回内存管理器中管理的一个地址，否则返回`NULL`
 */
void* memory__s_manager_malloc(memory_p_manager manager, uintptr_t size);

/**
 * 申请内存空间，允许按指定的最小单元分多块申请，直到满足需要的总大小
 * @param manager 内存管理器
 * @param size 想要申请的空间的大小
 * @param minimum 申请空间中最小块大小
 * @return 如果申请成功，则返回内存管理器中管理的一个地址，否则返回`NULL`
 */
void* memory__s_manager_falloc(memory_p_manager manager, uintptr_t size, uintptr_t minimum);

/**
 * 释放申请的内存
 * @param manager 内存管理器
 * @param address 该内存管理器中申请的内存首地址
 */
void memory__s_manager_free(memory_p_manager manager, void const* address);

/**
 * 查找`address`对应的当前块的首地址
 * @param manager 内存管理器
 * @param address 一个内存单元的地址
 * @return 如果`address`是该内存管理器中申请的内存地址中的一个内存单元地址，则返回这块申请的内存的首地址，否则返回`NULL`
 */
void* memory__s_manager_find(memory_p_manager manager, void* address);

/**
 * 获取内存管理器中管理的剩余空间大小
 * @param manager 内存管理器
 * @return 返回剩余空间大小
 */
uintptr_t memory__s_manager_remain(memory_p_manager manager);

#endif

#ifndef comp_fifo__
#define comp_fifo__

#include "debug.h"

#define FIFO_MANAGER(name_, type_)                                                                               \
    struct fifo_manager_t_##name_ {                                                                              \
        type_          _size;                                                                                    \
        volatile type_ _read;                                                                                    \
        volatile type_ _write;                                                                                   \
    };                                                                                                           \
                                                                                                                 \
    static inline void  fifo_manager_t_##type_##_init(struct fifo_manager_t_##type_* manager);                   \
    static inline void  fifo_manager_t_##type_##_init_size(struct fifo_manager_t_##type_* manager, type_ size);  \
    static inline type_ fifo_manager_t_##type_##_get_free(struct fifo_manager_t_##type_* manager);               \
    static inline type_ fifo_manager_t_##type_##_get_full(struct fifo_manager_t_##type_* manager);               \
    static inline type_ fifo_manager_t_##type_##_get_read(struct fifo_manager_t_##type_* manager);               \
    static inline type_ fifo_manager_t_##type_##_get_write(struct fifo_manager_t_##type_* manager);              \
    static inline void  fifo_manager_t_##type_##_set_read(struct fifo_manager_t_##type_* manager, type_ read);   \
    static inline void  fifo_manager_t_##type_##_set_write(struct fifo_manager_t_##type_* manager, type_ write); \
                                                                                                                 \
    void fifo_manager_t_##type_##_init(struct fifo_manager_t_##type_* manager)                                   \
    {                                                                                                            \
        DEBUG_ASSERT_PARAM(NULL != manager, "null pointer.");                                                 \
        manager->_size  = 0;                                                                                     \
        manager->_read  = 0;                                                                                     \
        manager->_write = 0;                                                                                     \
    }                                                                                                            \
                                                                                                                 \
    void fifo_manager_t_##type_##_init_size(struct fifo_manager_t_##type_* manager, type_ size)                  \
    {                                                                                                            \
        DEBUG_ASSERT_PARAM(NULL != manager, "null pointer.");                                                 \
        manager->_size  = size;                                                                                  \
        manager->_read  = 0;                                                                                     \
        manager->_write = 0;                                                                                     \
    }                                                                                                            \
                                                                                                                 \
    type_ fifo_manager_t_##type_##_get_free(struct fifo_manager_t_##type_* manager)                              \
    {                                                                                                            \
        DEBUG_ASSERT_PARAM(NULL != manager, "null pointer.");                                                 \
        if (manager->_read > manager->_write) {                                                                  \
            return manager->_read - manager->_write - 1;                                                         \
        } else {                                                                                                 \
            return manager->_size - manager->_write + manager->_read;                                            \
        }                                                                                                        \
    }                                                                                                            \
                                                                                                                 \
    type_ fifo_manager_t_##type_##_get_full(struct fifo_manager_t_##type_* manager)                              \
    {                                                                                                            \
        DEBUG_ASSERT_PARAM(NULL != manager, "null pointer.");                                                 \
        return manager->_size - fifo_manager_t_##type_##_get_free(manager);                                      \
    }                                                                                                            \
                                                                                                                 \
    type_ fifo_manager_t_##type_##_get_read(struct fifo_manager_t_##type_* manager)                              \
    {                                                                                                            \
        DEBUG_ASSERT_PARAM(NULL != manager, "null pointer.");                                                 \
        DEBUG_ASSERT_ILLEGAL(fifo_manager_t_##type_##_get_full(manager) > 0, "null pointer.");                  \
        return manager->_read;                                                                                   \
    }                                                                                                            \
                                                                                                                 \
    type_ fifo_manager_t_##type_##_get_write(struct fifo_manager_t_##type_* manager)                             \
    {                                                                                                            \
        DEBUG_ASSERT_PARAM(NULL != manager, "null pointer.");                                                 \
        DEBUG_ASSERT_ILLEGAL(fifo_manager_t_##type_##_get_free(manager) > 0, "null pointer.");                  \
        return manager->_write;                                                                                  \
    }                                                                                                            \
                                                                                                                 \
    void fifo_manager_t_##type_##_set_read(struct fifo_manager_t_##type_* manager, type_ read)                   \
    {                                                                                                            \
        DEBUG_ASSERT_PARAM(NULL != manager, "null pointer.");                                                 \
        DEBUG_ASSERT_ILLEGAL(read == manager->_read, "null pointer.");                                          \
        DEBUG_ASSERT_ILLEGAL(fifo_manager_t_##type_##_get_full(manager) > 0, "null pointer.");                  \
        manager->_read = (++read < manager->_size) ? read : 0;                                                   \
    }                                                                                                            \
                                                                                                                 \
    void fifo_manager_t_##type_##_set_write(struct fifo_manager_t_##type_* manager, type_ write)                 \
    {                                                                                                            \
        DEBUG_ASSERT_PARAM(NULL != manager, "null pointer.");                                                 \
        DEBUG_ASSERT_ILLEGAL(write == manager->_write, "null pointer.");                                        \
        DEBUG_ASSERT_ILLEGAL(fifo_manager_t_##type_##_get_free(manager) > 0, "null pointer.");                  \
        manager->_write = (++write < manager->_size) ? write : 0;                                                \
    }                                                                                                            \
                                                                                                                 \
    typedef struct fifo_manager_t_##type_ fifo_manager_t_##type_

#endif

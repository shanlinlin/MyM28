#ifndef fifo__
#define fifo__

#include "comp_fifo.h"
#include <stdint.h>

/**/ // clang-format off
#define FIFO_SIMPLEX_ERROR_BATCH(macro_) \
macro_( busy         , "busy."         )\
macro_( full         , "write full."   )\
macro_( empty        , "read empty."   )\
macro_( closed_read  , "read closed."  )\
macro_( closed_write , "write closed." )\
/**/ // clang-format on

typedef enum fifo_simplex_error_t {
#define MACRO(error_, message_) fifo_simplex_error_##error_,
    FIFO_SIMPLEX_ERROR_BATCH(MACRO) fifo_simplex_error_sum
#undef MACRO
} fifo_simplex_error_t;

FIFO_MANAGER(fifo, unsigned);

typedef struct fifo_simplex_t {
    fifo_manager_t_fifo fifo_manager;
    void*               data_buffer;
    bool                closed_read;
    bool                closed_write;
} fifo_simplex_t;

void     fifo_simplex_t_init(fifo_simplex_t* fifo);
unsigned fifo_simplex_t_get_free(fifo_simplex_t* fifo);
unsigned fifo_simplex_t_get_full(fifo_simplex_t* fifo);
void     fifo_simplex_t_set_buffer(fifo_simplex_t* fifo, void* buffer, unsigned size);
int      fifo_simplex_t_read(fifo_simplex_t* fifo, void* data, int byte_len);
int      fifo_simplex_t_write(fifo_simplex_t* fifo, void const* data, int byte_len);
void     fifo_simplex_t_close_read(fifo_simplex_t* fifo);
void     fifo_simplex_t_close_write(fifo_simplex_t* fifo);

#endif

#ifndef tftp__
#define tftp__

#include "common.h"
#include <stdint.h>

/**/ // clang-format off
#define TFTP_OPCODE_BATCH(macro_) \
macro_( 1 , read_req  )\
macro_( 2 , write_req )\
macro_( 3 , data      )\
macro_( 4 , ack       )\
macro_( 5 , error     )\

#define TFTP_MODE_BATCH(macro_) \
macro_( octet    )\
macro_( netascii )\

#define TFTP_ERROR_BATCH(macro_) \
macro_( 0 , undef     , "not defined."                      )\
macro_( 1 , not_found , "file not found."                   )\
macro_( 2 , access    , "access violation."                 )\
macro_( 3 , no_space  , "disk full or allocation exceeded." )\
macro_( 4 , bad_op    , "illegal TFTP operation."           )\
macro_( 5 , bad_id    , "unknown transfer ID."              )\
macro_( 6 , exists    , "file already exists."              )\
macro_( 7 , no_user   , "no such user."                     )\
macro_( 8 , option    , "option negotiation error."         )\
/**/ // clang-format on

typedef enum tftp_t_opcode {
#define MACRO(value_, name_) tftp_opcode_##name_ = (value_),
    TFTP_OPCODE_BATCH(MACRO) tftp_opcode_unknown = 0
#undef MACRO
} tftp_t_opcode;

typedef enum tftp_t_mode {
#define MACRO(name_) tftp_mode_##name_,
    TFTP_MODE_BATCH(MACRO) tftp_mode_sum
#undef MACRO
} tftp_t_mode;

typedef enum tftp_t_error {
#define MACRO(value_, name_, msg_) tftp_error_##name_,
    TFTP_ERROR_BATCH(MACRO) tftp_error_sum
#undef MACRO
} tftp_t_error;

int tftp_send_data(int socket, uint16_t block, void const* data, int length);
int tftp_send_ack(int socket, uint16_t block);
int tftp_send_error(int socket, uint16_t error, char const* msg);
int tftp_send_error_default(int socket, uint16_t error);

#endif

#ifndef ethercatmaster_foe__
#define ethercatmaster_foe__

#include "ethercatmaster.h"
#include "kernel/tftp/tftp_server.h"

/**/ // clang-format off
#define FOE_ERROR_BATCH(macro_) \
macro_( 0x8000 , undef     , "Not defined"         )\
macro_( 0x8001 , not_found , "Not found"           )\
macro_( 0x8002 , access    , "Access denied"       )\
macro_( 0x8003 , no_space  , "Disk full"           )\
macro_( 0x8004 , bad_op    , "Illegal"             )\
macro_( 0x8005 , bad_id    , "Packet number wrong" )\
macro_( 0x8006 , exists    , "Already exists"      )\
macro_( 0x8007 , no_user   , "No user"             )\
macro_( 0x8008 , undef     , "Bootstrap only"      )\
macro_( 0x8009 , undef     , "Not Bootstrap"       )\
macro_( 0x800A , undef     , "No rights"           )\
macro_( 0x800B , undef     , "Program Error"       )\
/**/ // clang-format on

typedef enum MAILBOX_FOE_STEP {
    MAILBOX_FOE_STEP_INIT,

    MAILBOX_FOE_STEP_READ_SEND_REQ,
    MAILBOX_FOE_STEP_READ_SEND_ACT,
    MAILBOX_FOE_STEP_READ_WAIT_DATA,
    MAILBOX_FOE_STEP_READ_FINISH,

    MAILBOX_FOE_STEP_WRITE_SEND_REQ,
    MAILBOX_FOE_STEP_WRITE_SEND_DATA,
    MAILBOX_FOE_STEP_WRITE_WAIT_ACT,
    MAILBOX_FOE_STEP_WRITE_RECV_ACT,
    MAILBOX_FOE_STEP_WRITE_FINISH,

    MAILBOX_FOE_STEP_SUM
} MAILBOX_FOE_STEP;

typedef struct TECSlave_FoE {
    tftp_server_t_interface interface;
    char const*             file_name;

    MAILBOX_FOE_STEP step;
    uint32_t         block;
    TtimerEx         timer;
    unsigned         timeout;
    uint32_t         password;
} TECSlave_FoE;

void ECSlave_FoE_DealMailbox(TECSlave* slave, MBXHR const* mailbox);
bool ECSlave_SendFoE(TECSlave* slave);

void ECSlave_FoE_Init(TECSlave_FoE* FoE);
void ECSlave_FoE_ParseOptions(TECSlave_FoE* FoE, char const* options);
int  ECSlave_FoE_StartRead(TECSlave_FoE* FoE, TECSlave* slave, tftp_server_t_interface interface, char const* file_name);
int  ECSlave_FoE_StartWrite(TECSlave_FoE* FoE, TECSlave* slave, tftp_server_t_interface interface, char const* file_name);

#endif

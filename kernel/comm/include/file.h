#ifndef file__
#define file__

/**/ // clang-format off
#define FILE_ERROR_BATCH(macro_) \
macro_( unknown             , "未知错误."   )\
macro_( node_exsited        , "节点已存在." )\
macro_( not_exsited         , "节点不存在." )\
macro_( access_denied       , "访问被拒绝." )\
macro_( disk_full           , "磁盘已满."   )\
macro_( insufficient_memory , "内存不足."   )\
macro_( unsupported         , "不支持的."   )\
macro_( eof                 , "文件结尾."   )\
macro_( busy                , "设备忙."     )\
macro_( mount_fail          , "挂载失败."   )\
/* FoE错误相关接口 */\
macro_( foe_undef     , "not defined."                      )\
macro_( foe_not_found , "file not found."                   )\
macro_( foe_access    , "access violation."                 )\
macro_( foe_no_space  , "disk full or allocation exceeded." )\
macro_( foe_bad_op    , "illegal TFTP operation."           )\
macro_( foe_bad_id    , "unknown transfer ID."              )\
macro_( foe_exists    , "file already exists."              )\
macro_( foe_no_user   , "no such user."                     )\
macro_( foe_option    , "option negotiation error."         )\
/**/ // clang-format on

enum {
#define MACRO(name_, msg_) file__error_##name_,
    FILE_ERROR_BATCH(MACRO) file__error_success = 0
#undef MACRO
};

typedef enum file_e_error {
#define MACRO(name_, msg_) file_error_##name_ = -file__error_##name_,
    FILE_ERROR_BATCH(MACRO) file_error_success = 0
#undef MACRO
} file_e_error;

typedef struct file_s__handle* file_p_handle;

void file_init(void);

int file_open_local(file_p_handle* handle, void const* path, char const* mode, char const* args);

int file_open(file_p_handle* handle, char const* path, char const* mode, char const* args);
int file_error(file_p_handle* handle);
int file_close(file_p_handle* handle);

int file_seek(file_p_handle* handle, long offset, int whence);
int file_read(file_p_handle* handle, void* data, int length);
int file_write(file_p_handle* handle, void const* data, int length);
int file_ctrl(file_p_handle* handle, char const* cmd, void* data);

#endif

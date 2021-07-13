#ifndef tftp_server__
#define tftp_server__

#include "ethernet.h"
#include "tftp.h"

typedef enum tftp_server_t_event {
    tftp_server_event_read,
    tftp_server_event_write,
    tftp_server_event_unlock,
    tftp_server_event_close,

    tftp_server_event_sum
} tftp_server_t_event;

typedef struct tftp_server_t__interface* tftp_server_t_interface;

typedef struct {
    char const* file_name;
    char const* options;
} tftp_server_t_event_access, tftp_server_t_event_read, tftp_server_t_event_write;

typedef void (*tftp_server_t_callback)(void* private, tftp_server_t_interface interface, tftp_server_t_event event, void* data);

void tftp_server_init(void);
int  tftp_server_create(unsigned device, IOETH const* ioeth);
void tftp_server_parse(int socket, WORD* data, WORD length, void* private);

void tftp_server_map_clear(void);
int  tftp_server_unmap(char const* key);
int  tftp_server_map(char const* key, void* private, tftp_server_t_callback callback);

unsigned tftp_server_t_interface_set_buffer(tftp_server_t_interface interface, unsigned size);
void     tftp_server_t_interface_set_error(tftp_server_t_interface interface, tftp_t_error error, char const* message);
void     tftp_server_t_interface_set_finish(tftp_server_t_interface interface);
unsigned tftp_server_t_interface_set_read(tftp_server_t_interface interface, unsigned blksize);
void     tftp_server_t_interface_set_private(tftp_server_t_interface interface, void* private);
void*    tftp_server_t_interface_get_private(tftp_server_t_interface interface);
unsigned tftp_server_t_interface_get_data_len(tftp_server_t_interface interface);
unsigned tftp_server_t_interface_get_free(tftp_server_t_interface interface);
bool     tftp_server_t_interface_check_finish(tftp_server_t_interface interface);
unsigned tftp_server_t_interface_read(tftp_server_t_interface interface, void* data, unsigned blksize);
unsigned tftp_server_t_interface_write(tftp_server_t_interface interface, void const* data, unsigned byte_len);

#endif

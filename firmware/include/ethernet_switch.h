#ifndef ethernet_switch__
#define ethernet_switch__

#include "debug.h"
#include "ethernet.h"
#include <stdbool.h>
#include <stdint.h>

struct ethernet_switch_s_port;

typedef void (*ethernet_switch_p_forward_frame)(struct ethernet_switch_s_port*, void const*, unsigned);

typedef struct ethernet_switch_s_port {
    struct ethernet_switch_s_port*  _next;
    bool                            _link;
    ethernet_s_mac                  _mac;
    ethernet_switch_p_forward_frame _forward_frame;
} ethernet_switch_s_port;

void ethernet_switch_s_port_init(ethernet_switch_s_port* port);
void ethernet_switch_s_port_add(ethernet_switch_s_port* port, ethernet_switch_s_port* add);
void ethernet_switch_s_port_set_forward_frame(ethernet_switch_s_port* port, ethernet_switch_p_forward_frame forward_frame);
void ethernet_switch_s_port_set_link(ethernet_switch_s_port* port, bool link);
void ethernet_switch_s_port_recved_frame(ethernet_switch_s_port* port, void const* buffer, unsigned length);

static inline bool ethernet_switch_s_port_get_link(ethernet_switch_s_port* port)
{
    DEBUG_ASSERT_PARAM(NULL != port, "null pointer.");
    return port->_link;
}

#endif

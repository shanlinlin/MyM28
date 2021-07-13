#ifndef ethercatmaster_eoe__
#define ethercatmaster_eoe__

#include "ethercatmaster.h"

void ECMaster_SetDnsName(TECSlave* pECSlave, char const* pDnsName);

void ECSlave_EoE_DealMailbox(TECSlave* slave, MBXHR const* mailbox);
bool ECSlave_SendEoE(TECSlave* slave);
bool ECSlave_EoE_SendInit(TECSlave* slave, ethercat_s_net_buffer const* buffer);
bool ECSlave_EoE_SendFragment(TECSlave* slave, ethercat_s_net_buffer const* buffer);
void ECSlave_EoE_SetIpParameter(TECSlave* slave);

void ethernet_forward_frame(ethernet_switch_s_port* port, void const* buffer, unsigned length);
void ethercat_forward_frame(ethernet_switch_s_port* port, void const* buffer, unsigned length);

void ethercat_s_net_buffer_init(ethercat_s_net_buffer* buffer);
void ethercat_s_net_buffer_init_fifo(ethercat_s_net_buffer* buffer, ethercat_s_net_block* block, unsigned length);

#endif

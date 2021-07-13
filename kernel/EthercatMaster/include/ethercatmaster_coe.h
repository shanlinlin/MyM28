#ifndef ethercatmaster_coe__
#define ethercatmaster_coe__

#include "ethercatmaster.h"

#define ETHERCAT_SDO_TIMEOUT 100
#define ETHERCAT_SDOMUX_TIMEROUT 100

void ECSlave_CoE_DealMailbox(TECSlave* slave, MBXHR const* mailbox);
bool ECSlave_SendCoE(TECSlave* slave);
void ECSlave_CoE_SDOUploadRequest(TECSlave* slave, uint16_t index, uint16_t subindex, bool complete_access);
void ECSlave_CoE_SDOUploadSegmentRequest(TECSlave* slave, bool toggle);
void ECSlave_CoE_SDODownloadExpeditedRequest(TECSlave* slave, uint16_t index, uint16_t subindex, void const* data, unsigned length);
void ECSlave_CoE_SDODownloadNormalRequest(TECSlave* slave, uint16_t index, uint16_t subindex, void const* data, unsigned length, bool complete_access);
void ECSlave_CoE_SDODownloadSegmentRequest(TECSlave* slave, void const* data, unsigned offset, unsigned length, bool more, bool toggle);

#endif

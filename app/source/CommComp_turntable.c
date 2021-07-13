/*=============================================================================+
|  Function :                                                                  |
|  Task     :                                                                  |
|------------------------------------------------------------------------------|
|  Compile  :                                                                  |
|  Link     :                                                                  |
|  Call     :                                                                  |
|------------------------------------------------------------------------------|
|  Author   : Hankin                                                           |
|  Version  : V1.00                                                            |
|  Creation : 2015/01/14                                                       |
|  Revision :                                                                  |
+=============================================================================*/
#include "CommComp_turntable.h"
#include "CommComp_CANOpenMaster.h"
#include "Error_App.h"
//##############################################################################
// definition
//##############################################################################
#define HYDRAULIC_HEARTBEAT_CYCLE 500
#define HYDRAULIC_LOST_MAX 5

#define ELECTRIC_HEARTBEAT_CYCLE 500
#define ELECTRIC_LOST_MAX 5
#define ELECTRIC_ERROR_REQ_CYCLE 100
//##############################################################################
// enumeration
//##############################################################################
enum CommComp_turntable__e_card_type
{
	CommComp_turntable__CARD_TYPE_HYDRAULIC,
	CommComp_turntable__CARD_TYPE_ELECTRIC,
	CommComp_turntable__CARD_TYPE_HT,

	CommComp_turntable__CARD_TYPE_SUM
};

enum CommComp_turntable_e_hydraulic_index
{
	CommComp_turntable_HYDRAULIC_INDEX_0,
	CommComp_turntable_HYDRAULIC_INDEX_1,

	CommComp_turntable_HYDRAULIC_INDEX_SUM
};

enum CommComp_turntable_e_electric_index
{
	CommComp_turntable_ELECTRIC_INDEX_0,
	CommComp_turntable_ELECTRIC_INDEX_1,

	CommComp_turntable_ELECTRIC_INDEX_SUM
};
//##############################################################################
// data type
//##############################################################################
struct CommComp_turntable__s_config_info
{
	enum CommComp_turntable__e_card_type eCard_type;
	unsigned                             iCard_index;
};

struct CommComp_turntable__s_card_hydraulic
{
	CommComp_S          _sComp;
	CANOpenCompUnique_S _sUnique_CANopen_comp;
	S_CANOpenCHBManager _sCHBManager;

	S_CANOpenCHBManager _sPDOManager;
	Ttimer              _sPDOSendTime;

	CommSubComp_S          _sSubcomp;
	CANOpenSubCompUnique_S _sUnique_CANopen_subcomp;

	unsigned _iRemote_index;
	unsigned _iTurntable_index;
	unsigned _iPF_channel;
	unsigned _iComm_cycle;

	// // (slave) recv PDO 1 (auto)
	// WORD i200000; // 0x160001
	// WORD f200100; // 0x160002
	// WORD i200200; // 0x160003

	// (slave) send PDO 1
	WORD i201000; // 0x1A0001
	WORD i3EC800; // 0x1A0002
	WORD i201100; // 0x1A0003
	WORD i201200; // 0x1A0004
};

struct CommComp_turntable__s_card_HT
{
	CommComp_S          _sComp;
	CANOpenCompUnique_S _sUnique_CANopen_comp;
	S_CANOpenCHBManager _sCHBManager;

	S_CANOpenCHBManager _sPDOManager;
	Ttimer              _sPDOSendTime;
	Ttimer              _sErrorReqTime;

	CommSubComp_S          _sSubcomp;
	CANOpenSubCompUnique_S _sUnique_CANopen_subcomp;

	unsigned _iRemote_index;
	unsigned _iTurntable_index;
	unsigned _iComm_cycle;

	// (slave) send PDO 1
	WORD wError;
	WORD wStatusWord;
	INT  nActAngle;
	INT  nActSpd;
	INT  nActValveVolt;
};

struct CommComp_turntable__s_card_electric
{
	CommComp_S          _sComp;
	CANOpenCompUnique_S _sUnique_CANopen_comp;
	S_CANOpenCHBManager _sCHBManager;

	S_CANOpenCHBManager _aPDOManager[2];
	Ttimer              _sPDOSendTime;
	Ttimer              _sErrorReqTime;

	CommSubComp_S          _sSubcomp;
	CANOpenSubCompUnique_S _sUnique_CANopen_subcomp;

	unsigned _iRemote_index;
	unsigned _iTurntable_index;
	unsigned _iPF_channel;
	unsigned _iComm_cycle;

	// SDO
	DWORD i40DA00; // for calculation speed.
	DWORD i40DB00; // for calculation speed.
	DWORD i40DC00; // for calculation angle.
	DWORD i202500; // error.

	// // (slave) recv PDO 1(auto)
	// DWORD i60FF00; // 0x160002
	// WORD  i604000; // 0x160001
	// WORD  i420800; // 0x160003

	// (slave) send PDO 1
	WORD i604100; // 0x1A0001
	WORD i420900; // 0x1A0002

	// (slave) send PDO 2
	LONG  i414200; // 0x1A0101
	FLOAT f200100; // 0x1A0102
};
//##############################################################################
// function declaration
//##############################################################################
void        CommComp_turntable__s_card_hydraulic_init(struct CommComp_turntable__s_card_hydraulic* pCard);
void        CommComp_turntable__s_card_HT_init(struct CommComp_turntable__s_card_HT* pCard);
bool        CommComp_turntable__s_card_hydraulic_add(struct CommComp_turntable__s_card_hydraulic* pCard, unsigned iRemote_index, unsigned iTurntable_index, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle);
bool        CommComp_turntable__s_card_HT_add(struct CommComp_turntable__s_card_HT* pCard, unsigned iRemote_index, unsigned iTurntable_index, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle);
inline void CommComp_turntable__s_card_hydraulic_del(struct CommComp_turntable__s_card_hydraulic* pCard);
inline void CommComp_turntable__s_card_HT_del(struct CommComp_turntable__s_card_HT* pCard);
inline void CommComp_turntable__s_card_hydraulic_set_PF_channel(struct CommComp_turntable__s_card_hydraulic* pCard, unsigned iPF_channel);
// inline void CommComp_turntable__s_card_HT_set_PF_channel(struct CommComp_turntable__s_card_HT* pCard, unsigned iPF_channel);
void CommComp_turntable__s_card_hydraulic_comp_event_callback(CommComp_S* pComp, CommEvent_U* pEvent);
void CommComp_turntable__s_card_HT_comp_event_callback(CommComp_S* pComp, CommEvent_U* pEvent);
bool CommComp_turntable__s_card_hydraulic_CANopen_SDO_config_finish(struct CANOpenCompUnique_S* psCompUnique, struct CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);
bool CommComp_turntable__s_card_HT_CANopen_SDO_config_finish(struct CANOpenCompUnique_S* psCompUnique, struct CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);
void CommComp_turntable__s_card_hydraulic_CANopen_event_callback(CANOpenCompUnique_S* pUnique, CANOpenDataFrame_S* pData_frame, E_CANOpenEventType eEvent_type);
void CommComp_turntable__s_card_HT_CANopen_event_callback(CANOpenCompUnique_S* pUnique, CANOpenDataFrame_S* pData_frame, E_CANOpenEventType eEvent_type);
void CommComp_turntable__s_card_hydraulic_CANopen_custom_operation(struct CommComp_S* const psComp, E_CustomFuncType eType);
void CommComp_turntable__s_card_HT_CANopen_custom_operation(struct CommComp_S* const psComp, E_CustomFuncType eType);

void        CommComp_turntable__s_card_electric_init(struct CommComp_turntable__s_card_electric* pCard);
bool        CommComp_turntable__s_card_electric_add(struct CommComp_turntable__s_card_electric* pCard, unsigned iRemote_index, unsigned iTurntable_index, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle);
inline void CommComp_turntable__s_card_electric_del(struct CommComp_turntable__s_card_electric* pCard);
inline void CommComp_turntable__s_card_electric_set_PF_channel(struct CommComp_turntable__s_card_electric* pCard, unsigned iPF_channel);
void        CommComp_turntable__s_card_electric_comp_event_callback(CommComp_S* pComp, CommEvent_U* pEvent);
bool        CommComp_turntable__s_card_electric_CANopen_SDO_config_finish(struct CANOpenCompUnique_S* psCompUnique, struct CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);
void        CommComp_turntable__s_card_electric_CANopen_event_callback(CANOpenCompUnique_S* pUnique, CANOpenDataFrame_S* pData_frame, E_CANOpenEventType eEvent_type);
void        CommComp_turntable__s_card_electric_CANopen_custom_operation(struct CommComp_S* const psComp, E_CustomFuncType eType);
//##############################################################################
// data declaration
//##############################################################################
// CommOffsetSeg_S const CommComp_turntable__aCard_read_index_seg[] = {
//    { DBINDEX_POWERCARD0_CONFIG_CARD_ID,
//    DBINDEX_POWERCARD0_INTERFACE_IRMS_RESET },
//};
// CommOffsetSeg_S const CommComp_turntable__aCard_write_index_seg[] = {
//    { DBINDEX_POWERCARD0_INTERFACE_HARDWARE_PN,
//    DBINDEX_POWERCARD0_INTERFACE_SOFTWARE_PN },
//};
//------------------------------------------------------------------------------
struct CommComp_turntable__s_config_info    CommComp_turntable__aConfig_tab[CommComp_turntable_INDEX_SUM];
struct CommComp_turntable__s_card_hydraulic CommComp_turntable_aCard_hydraulic[CommComp_turntable_HYDRAULIC_INDEX_SUM];
struct CommComp_turntable__s_card_electric  CommComp_turntable_aCard_electric[CommComp_turntable_ELECTRIC_INDEX_SUM];
struct CommComp_turntable__s_card_HT        CommComp_turntable_aCard_HT[CommComp_turntable_ELECTRIC_INDEX_SUM];
//##############################################################################
// function prototype
//##############################################################################
/**
 * [CommComp_turntable_init description]
 */
void CommComp_turntable_init(void)
{
	unsigned i;

#if (DEBUG)
	{
		struct CommComp_turntable__s_config_info sConfig_info;

		sConfig_info.iCard_index = -1;
		debug_assert(sConfig_info.iCard_index >= (sizeof(CommComp_turntable_aCard_hydraulic) / sizeof(*CommComp_turntable_aCard_hydraulic)));
	}
#endif
	for (i = 0; i < (sizeof(CommComp_turntable_aCard_hydraulic) / sizeof(*CommComp_turntable_aCard_hydraulic)); ++i)
	{
		CommComp_turntable__s_card_hydraulic_init(&CommComp_turntable_aCard_hydraulic[i]);
	}
	for (i = 0; i < (sizeof(CommComp_turntable_aCard_electric) / sizeof(*CommComp_turntable_aCard_electric)); ++i)
	{
		CommComp_turntable__s_card_electric_init(&CommComp_turntable_aCard_electric[i]);
	}
	for (i = 0; i < (sizeof(CommComp_turntable_aCard_HT) / sizeof(*CommComp_turntable_aCard_HT)); ++i)
	{
		CommComp_turntable__s_card_HT_init(&CommComp_turntable_aCard_HT[i]);
	}

	for (i = 0; i < (sizeof(CommComp_turntable__aConfig_tab) / sizeof(*CommComp_turntable__aConfig_tab)); ++i)
	{
		CommComp_turntable__aConfig_tab[i].eCard_type = CommComp_turntable__CARD_TYPE_SUM;
	}
	//DEBUG_INIT_CHECK(struct CommComp_turntable__s_card_hydraulic, CommComp_turntable__s_card_hydraulic_init);
	//DEBUG_INIT_CHECK(struct CommComp_turntable__s_card_electric, CommComp_turntable__s_card_electric_init);
}

/**
 * [CommComp_turntable_clean description]
 */
void CommComp_turntable_clean(void)
{
	unsigned i;

	for (i = 0; i < (sizeof(CommComp_turntable_aCard_hydraulic) / sizeof(*CommComp_turntable_aCard_hydraulic)); ++i)
	{
		CommComp_turntable__s_card_hydraulic_del(&CommComp_turntable_aCard_hydraulic[i]);
	}
	for (i = 0; i < (sizeof(CommComp_turntable_aCard_electric) / sizeof(*CommComp_turntable_aCard_electric)); ++i)
	{
		CommComp_turntable__s_card_electric_del(&CommComp_turntable_aCard_electric[i]);
	}
	for (i = 0; i < (sizeof(CommComp_turntable_aCard_HT) / sizeof(*CommComp_turntable_aCard_HT)); ++i)
	{
		CommComp_turntable__s_card_HT_del(&CommComp_turntable_aCard_HT[i]);
	}
	for (i = 0; i < (sizeof(CommComp_turntable__aConfig_tab) / sizeof(*CommComp_turntable__aConfig_tab)); ++i)
	{
		CommComp_turntable__aConfig_tab[i].eCard_type = CommComp_turntable__CARD_TYPE_SUM;
	}
}

/**
 * [CommComp_turntable_add_hydraulic description]
 * @param  eComm_type  [description]
 * @param  iNode_ID    [description]
 * @param  iComm_cycle [description]
 * @return             [description]
 */
bool CommComp_turntable_add_hydraulic(unsigned iRemoteNum, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle)
{
	unsigned iHydraulic_sum;
	unsigned i;

	iHydraulic_sum = 0;
	for (i = 0; i < (sizeof(CommComp_turntable__aConfig_tab) / sizeof(*CommComp_turntable__aConfig_tab)); ++i)
	{
		if (CommComp_turntable__CARD_TYPE_SUM == CommComp_turntable__aConfig_tab[i].eCard_type)
		{
			break;
		}
		else if (CommComp_turntable__CARD_TYPE_HYDRAULIC == CommComp_turntable__aConfig_tab[i].eCard_type)
		{
			++iHydraulic_sum;
		}
	}
	DEBUG_ASSERT_WARN(i < (sizeof(CommComp_turntable__aConfig_tab) / sizeof(*CommComp_turntable__aConfig_tab)), "config tab too small.");
	if (i >= (sizeof(CommComp_turntable__aConfig_tab) / sizeof(*CommComp_turntable__aConfig_tab)))
		return false;
	CommComp_turntable__aConfig_tab[i].eCard_type  = CommComp_turntable__CARD_TYPE_HYDRAULIC;
	CommComp_turntable__aConfig_tab[i].iCard_index = iHydraulic_sum;
	DEBUG_ASSERT_ILLEGAL(iHydraulic_sum < (sizeof(CommComp_turntable_aCard_hydraulic) / sizeof(*CommComp_turntable_aCard_hydraulic)), "hydraulic card too small.");
	if (!CommComp_turntable__s_card_hydraulic_add(&CommComp_turntable_aCard_hydraulic[iHydraulic_sum], iRemoteNum, i, eComm_type, iNode_ID, iComm_cycle))
	{
		DEBUG_ASSERT_WARN(false, "hydraulic add fail.");
		CommComp_turntable__aConfig_tab[i].eCard_type = CommComp_turntable__CARD_TYPE_SUM;
		return false;
	}
	return true;
}

bool CommComp_turntable_add_HT(unsigned iRemoteNum, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle)
{
	unsigned wHT_sum;
	unsigned i;

	wHT_sum = 0;
	for (i = 0; i < (sizeof(CommComp_turntable__aConfig_tab) / sizeof(*CommComp_turntable__aConfig_tab)); ++i)
	{
		if (CommComp_turntable__CARD_TYPE_SUM == CommComp_turntable__aConfig_tab[i].eCard_type)
		{
			break;
		}
		else if (CommComp_turntable__CARD_TYPE_HT == CommComp_turntable__aConfig_tab[i].eCard_type)
		{
			++wHT_sum;
		}
	}
	DEBUG_ASSERT_WARN(i < (sizeof(CommComp_turntable__aConfig_tab) / sizeof(*CommComp_turntable__aConfig_tab)), "config tab too small.");
	if (i >= (sizeof(CommComp_turntable__aConfig_tab) / sizeof(*CommComp_turntable__aConfig_tab)))
		return false;
	CommComp_turntable__aConfig_tab[i].eCard_type  = CommComp_turntable__CARD_TYPE_HT;
	CommComp_turntable__aConfig_tab[i].iCard_index = wHT_sum;
	DEBUG_ASSERT_ILLEGAL(wHT_sum < (sizeof(CommComp_turntable_aCard_HT) / sizeof(*CommComp_turntable_aCard_HT)), "hydraulic card too small.");
	if (!CommComp_turntable__s_card_HT_add(&CommComp_turntable_aCard_HT[wHT_sum], iRemoteNum, i, eComm_type, iNode_ID, iComm_cycle))
	{
		DEBUG_ASSERT_WARN(false, "hydraulic add fail.");
		CommComp_turntable__aConfig_tab[i].eCard_type = CommComp_turntable__CARD_TYPE_SUM;
		return false;
	}
	return true;
}

/**
 * [CommComp_turntable_add_electric description]
 * @param  eComm_type  [description]
 * @param  iNode_ID    [description]
 * @param  iComm_cycle [description]
 * @return             [description]
 */
bool CommComp_turntable_add_electric(unsigned iRemoteNum, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle)
{
	unsigned iElectric_sum;
	unsigned i;

	iElectric_sum = 0;
	for (i = 0; i < (sizeof(CommComp_turntable__aConfig_tab) / sizeof(*CommComp_turntable__aConfig_tab)); ++i)
	{
		if (CommComp_turntable__CARD_TYPE_SUM == CommComp_turntable__aConfig_tab[i].eCard_type)
		{
			break;
		}
		else if (CommComp_turntable__CARD_TYPE_ELECTRIC == CommComp_turntable__aConfig_tab[i].eCard_type)
		{
			++iElectric_sum;
		}
	}
	DEBUG_ASSERT_WARN(i < (sizeof(CommComp_turntable__aConfig_tab) / sizeof(*CommComp_turntable__aConfig_tab)), "config tab too small.");
	if (i >= (sizeof(CommComp_turntable__aConfig_tab) / sizeof(*CommComp_turntable__aConfig_tab)))
		return false;
	CommComp_turntable__aConfig_tab[i].eCard_type  = CommComp_turntable__CARD_TYPE_ELECTRIC;
	CommComp_turntable__aConfig_tab[i].iCard_index = iElectric_sum;
	DEBUG_ASSERT_ILLEGAL(iElectric_sum < (sizeof(CommComp_turntable_aCard_electric) / sizeof(*CommComp_turntable_aCard_electric)), "hydraulic card too small.");
	if (!CommComp_turntable__s_card_electric_add(&CommComp_turntable_aCard_electric[iElectric_sum], iRemoteNum, i, eComm_type, iNode_ID, iComm_cycle))
	{
		DEBUG_ASSERT_WARN(false, "electric add fail.");
		CommComp_turntable__aConfig_tab[i].eCard_type = CommComp_turntable__CARD_TYPE_SUM;
		return false;
	}
	return true;
}

/**
 * [CommComp_turntable_set_PF_channel description]
 * @param iTurntable  [description]
 * @param iPF_channel [description]
 */
void CommComp_turntable_set_PF_channel(enum CommComp_turntable_e_index eIndex, unsigned iPF_channel)
{
	unsigned iIndex;
	unsigned i;

	iIndex = eIndex;
	DEBUG_ASSERT_PARAM(iIndex < (sizeof(CommComp_turntable__aConfig_tab) / sizeof(*CommComp_turntable__aConfig_tab)), "subscript out of range");
	i = CommComp_turntable__aConfig_tab[iIndex].iCard_index;
	switch (CommComp_turntable__aConfig_tab[iIndex].eCard_type)
	{
		case CommComp_turntable__CARD_TYPE_HYDRAULIC:
			CommComp_turntable__s_card_hydraulic_set_PF_channel(&CommComp_turntable_aCard_hydraulic[i], iPF_channel);
			break;
		case CommComp_turntable__CARD_TYPE_ELECTRIC:
			CommComp_turntable__s_card_electric_set_PF_channel(&CommComp_turntable_aCard_electric[i], iPF_channel);
		case CommComp_turntable__CARD_TYPE_HT:
			break;
		default:
			DEBUG_ASSERT_WARN(false, "case error");
			break;
	}
}

/**
 * [CommComp_turntable_get_awRsvd description]
 * @param  eIndex [description]
 * @return        [description]
 */
WORD CommComp_turntable_get_awRsvd(enum CommComp_turntable_e_index eIndex)
{
	unsigned iIndex;
	unsigned i;

	iIndex = eIndex;
	DEBUG_ASSERT_PARAM(iIndex < (sizeof(CommComp_turntable__aConfig_tab) / sizeof(*CommComp_turntable__aConfig_tab)), "subscript out of range");
	i = CommComp_turntable__aConfig_tab[iIndex].iCard_index;
	switch (CommComp_turntable__aConfig_tab[iIndex].eCard_type)
	{
		case CommComp_turntable__CARD_TYPE_HYDRAULIC:
			return CommComp_turntable_aCard_hydraulic[i].i201000;
		case CommComp_turntable__CARD_TYPE_ELECTRIC:
			return CommComp_turntable_aCard_electric[i].i420900;
		case CommComp_turntable__CARD_TYPE_HT:
			return CommComp_turntable_aCard_HT[i].wStatusWord;
		default:
			DEBUG_ASSERT_WARN(false, "case error");
			return 0;
	}
}

/**
 * [CommComp_turntable_get_error description]
 * @param  eIndex [description]
 * @return        [description]
 */
WORD CommComp_turntable_get_error(enum CommComp_turntable_e_index eIndex)
{
	unsigned iIndex;
	unsigned i;

	iIndex = eIndex;
	DEBUG_ASSERT_PARAM(iIndex < (sizeof(CommComp_turntable__aConfig_tab) / sizeof(*CommComp_turntable__aConfig_tab)), "subscript out of range");
	i = CommComp_turntable__aConfig_tab[iIndex].iCard_index;
	switch (CommComp_turntable__aConfig_tab[iIndex].eCard_type)
	{
		case CommComp_turntable__CARD_TYPE_HYDRAULIC:
			return CommComp_turntable_aCard_hydraulic[i].i3EC800;
		case CommComp_turntable__CARD_TYPE_ELECTRIC:
			return CommComp_turntable_aCard_electric[i].i202500;
		case CommComp_turntable__CARD_TYPE_HT:
			return CommComp_turntable_aCard_HT[i].wError;
		default:
			DEBUG_ASSERT_WARN(false, "case error");
			return 0;
	}
}

/**
 * [CommComp_turntable_get_act_position_1 description]
 * @param  eIndex [description]
 * @return        [description]
 */
LONG CommComp_turntable_get_act_position_1(enum CommComp_turntable_e_index eIndex)
{
	unsigned iIndex;
	unsigned i;

	iIndex = eIndex;
	DEBUG_ASSERT_PARAM(iIndex < (sizeof(CommComp_turntable__aConfig_tab) / sizeof(*CommComp_turntable__aConfig_tab)), "subscript out of range");
	i = CommComp_turntable__aConfig_tab[iIndex].iCard_index;
	switch (CommComp_turntable__aConfig_tab[iIndex].eCard_type)
	{
		case CommComp_turntable__CARD_TYPE_HYDRAULIC:
			return CommComp_turntable_aCard_hydraulic[i].i201100;
		case CommComp_turntable__CARD_TYPE_ELECTRIC:
			return ((FLOAT)CommComp_turntable_aCard_electric[i].i414200 / CommComp_turntable_aCard_electric[i].i40DC00 * 3600.0 + 0.5);
		case CommComp_turntable__CARD_TYPE_HT:
			return CommComp_turntable_aCard_HT[i].nActAngle;
		default:
			DEBUG_ASSERT_WARN(false, "case error");
			return 0;
	}
}

/**
 * [CommComp_turntable_get_act_speed_1 description]
 * @param  eIndex [description]
 * @return        [description]
 */
WORD CommComp_turntable_get_act_speed_1(enum CommComp_turntable_e_index eIndex)
{
	unsigned iIndex;
	unsigned i;

	iIndex = eIndex;
	DEBUG_ASSERT_PARAM(iIndex < (sizeof(CommComp_turntable__aConfig_tab) / sizeof(*CommComp_turntable__aConfig_tab)), "subscript out of range");
	i = CommComp_turntable__aConfig_tab[iIndex].iCard_index;
	switch (CommComp_turntable__aConfig_tab[iIndex].eCard_type)
	{
		case CommComp_turntable__CARD_TYPE_HYDRAULIC:
			return CommComp_turntable_aCard_hydraulic[i].i201200;
		case CommComp_turntable__CARD_TYPE_ELECTRIC:
			return (INT)(CommComp_turntable_aCard_electric[i].f200100 * CommComp_turntable_aCard_electric[i].i40DB00 / CommComp_turntable_aCard_electric[i].i40DA00);
		case CommComp_turntable__CARD_TYPE_HT:
			return CommComp_turntable_aCard_HT[i].nActSpd;
		default:
			DEBUG_ASSERT_WARN(false, "case error");
			return 0;
	}
}

/**
 * [CommComp_turntable_get_act_voltag description]
 * @param  eIndex [description]
 * @return        [description]
 */
WORD CommComp_turntable_get_act_voltag(enum CommComp_turntable_e_index eIndex)
{
	unsigned iIndex;
	unsigned i;

	iIndex = eIndex;
	DEBUG_ASSERT_PARAM(iIndex < (sizeof(CommComp_turntable__aConfig_tab) / sizeof(*CommComp_turntable__aConfig_tab)), "subscript out of range");
	i = CommComp_turntable__aConfig_tab[iIndex].iCard_index;
	switch (CommComp_turntable__aConfig_tab[iIndex].eCard_type)
	{
		case CommComp_turntable__CARD_TYPE_HT:
			return CommComp_turntable_aCard_HT[i].nActValveVolt;
		default:
			DEBUG_ASSERT_WARN(false, "case error");
			return 0;
	}
}

/**
 * [CommComp_turntable__s_card_hydraulic_init description]
 * @param pCard [description]
 */
void CommComp_turntable__s_card_hydraulic_init(struct CommComp_turntable__s_card_hydraulic* pCard)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	SCommComp_Init(&pCard->_sComp);
	SCommComp_InitEventCallBack(&pCard->_sComp, CommComp_turntable__s_card_hydraulic_comp_event_callback);

	SCANOpenCompUnique_Init(&pCard->_sUnique_CANopen_comp);
	SCANOpenCompUnique_InitEventCallBack(&pCard->_sUnique_CANopen_comp, CommComp_turntable__s_card_hydraulic_CANopen_event_callback);
	SCANOpenCompUnique_InitConfigInfo(&pCard->_sUnique_CANopen_comp, CommComp_turntable__s_card_hydraulic_CANopen_SDO_config_finish);

	CANOpenCHBManager_Init(&pCard->_sCHBManager);
	pCard->_sCHBManager.iLose_max     = HYDRAULIC_LOST_MAX;
	pCard->_sCHBManager.iConflict_max = 5;
	pCard->_sCHBManager.iTick         = HYDRAULIC_HEARTBEAT_CYCLE;

	CANOpenCHBManager_Init(&pCard->_sPDOManager);
	pCard->_sPDOManager.iLose_max     = 50;
	pCard->_sPDOManager.iConflict_max = 50;
	//pCard->_sPDOManager.iTick         = HYDRAULIC_PDO_CYCLE; // hankin 20171031 mask.
	ResetTimer(&pCard->_sPDOSendTime, 0);

	SCommSubComp_Init(&pCard->_sSubcomp);
	SCANOpenSubCompUnique_Init(&pCard->_sUnique_CANopen_subcomp);

	pCard->_iRemote_index    = (unsigned)-1;
	pCard->_iTurntable_index = (unsigned)-1;
	pCard->_iPF_channel      = (unsigned)-1;
	pCard->_iComm_cycle      = 0;

	// (auto)
	// pCard->i200000 = 0;
	// pCard->f200100 = 0;
	// pCard->i200200 = 0;

	pCard->i201000 = 0;
	pCard->i3EC800 = 0;
	pCard->i201100 = 0;
	pCard->i201200 = 0;

	SCommComp_AddSubComp(&pCard->_sComp, &pCard->_sSubcomp);
}

void CommComp_turntable__s_card_HT_init(struct CommComp_turntable__s_card_HT* pCard)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	SCommComp_Init(&pCard->_sComp);
	SCommComp_InitEventCallBack(&pCard->_sComp, CommComp_turntable__s_card_HT_comp_event_callback);

	SCANOpenCompUnique_Init(&pCard->_sUnique_CANopen_comp);
	SCANOpenCompUnique_InitEventCallBack(&pCard->_sUnique_CANopen_comp, CommComp_turntable__s_card_HT_CANopen_event_callback);
	SCANOpenCompUnique_InitConfigInfo(&pCard->_sUnique_CANopen_comp, CommComp_turntable__s_card_HT_CANopen_SDO_config_finish);

	CANOpenCHBManager_Init(&pCard->_sCHBManager);
	pCard->_sCHBManager.iLose_max     = HYDRAULIC_LOST_MAX;
	pCard->_sCHBManager.iConflict_max = 5;
	pCard->_sCHBManager.iTick         = HYDRAULIC_HEARTBEAT_CYCLE;

	CANOpenCHBManager_Init(&pCard->_sPDOManager);
	pCard->_sPDOManager.iLose_max     = 50;
	pCard->_sPDOManager.iConflict_max = 50;
	//pCard->_sPDOManager.iTick         = HYDRAULIC_PDO_CYCLE; // hankin 20171031 mask.
	ResetTimer(&pCard->_sPDOSendTime, 0);
	ResetTimer(&pCard->_sErrorReqTime, 0);

	SCommSubComp_Init(&pCard->_sSubcomp);
	SCANOpenSubCompUnique_Init(&pCard->_sUnique_CANopen_subcomp);

	pCard->_iRemote_index    = (unsigned)-1;
	pCard->_iTurntable_index = (unsigned)-1;
	pCard->_iComm_cycle      = 0;

	pCard->wError        = 0;
	pCard->wStatusWord   = 0;
	pCard->nActAngle     = 0;
	pCard->nActSpd       = 0;
	pCard->nActValveVolt = 0;

	SCommComp_AddSubComp(&pCard->_sComp, &pCard->_sSubcomp);
}

/**
 * [CommComp_turntable__s_card_hydraulic_add description]
 * @param  pCard            [description]
 * @param  iRemote_index    [description]
 * @param  iTurntable_index [description]
 * @param  eComm_type       [description]
 * @param  iNode_ID         [description]
 * @param  iComm_cycle      [description]
 * @return                  [description]
 */
bool CommComp_turntable__s_card_hydraulic_add(struct CommComp_turntable__s_card_hydraulic* pCard, unsigned iRemote_index, unsigned iTurntable_index, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	switch (eComm_type)
	{
		case COMMTYPE_CAN1:
		case COMMTYPE_CAN2:
			SCommComp_SetCommType(&pCard->_sComp, eComm_type);
			SCANOpenCompUnique_SetNodeID(&pCard->_sUnique_CANopen_comp, iNode_ID);
			pCard->_sPDOManager.iTick = iComm_cycle;
			pCard->_iComm_cycle       = iComm_cycle;
			pCard->_iRemote_index     = iRemote_index;
			pCard->_iTurntable_index  = iTurntable_index;
			break;
		default:
			DEBUG_ASSERT_WARN(true, "case error");
			return false;
	}
	return true;
}

bool CommComp_turntable__s_card_HT_add(struct CommComp_turntable__s_card_HT* pCard, unsigned iRemote_index, unsigned iTurntable_index, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	switch (eComm_type)
	{
		case COMMTYPE_CAN1:
		case COMMTYPE_CAN2:
			SCommComp_SetCommType(&pCard->_sComp, eComm_type);
			SCANOpenCompUnique_SetNodeID(&pCard->_sUnique_CANopen_comp, iNode_ID);
			pCard->_sPDOManager.iTick = iComm_cycle;
			pCard->_iComm_cycle       = iComm_cycle;
			pCard->_iRemote_index     = iRemote_index;
			pCard->_iTurntable_index  = iTurntable_index;
			break;
		default:
			DEBUG_ASSERT_WARN(true, "case error");
			return false;
	}
	return true;
}

/**
 * [CommComp_turntable__s_card_hydraulic_del description]
 * @param pCard [description]
 */
inline void CommComp_turntable__s_card_hydraulic_del(struct CommComp_turntable__s_card_hydraulic* pCard)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	SCommComp_SetCommType(&pCard->_sComp, COMMTYPE_SUM);
	pCard->_iRemote_index    = (unsigned)-1;
	pCard->_iTurntable_index = (unsigned)-1;
	pCard->_iPF_channel      = (unsigned)-1;
}

inline void CommComp_turntable__s_card_HT_del(struct CommComp_turntable__s_card_HT* pCard)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	SCommComp_SetCommType(&pCard->_sComp, COMMTYPE_SUM);
	pCard->_iRemote_index    = (unsigned)-1;
	pCard->_iTurntable_index = (unsigned)-1;
	// pCard->_iPF_channel      = (unsigned)-1;
}

/**
 * [CommComp_turntable__s_card_hydraulic_set_PF_channel description]
 * @param pCard       [description]
 * @param iPF_channel [description]
 */
inline void CommComp_turntable__s_card_hydraulic_set_PF_channel(struct CommComp_turntable__s_card_hydraulic* pCard, unsigned iPF_channel)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	--iPF_channel;
	if (iPF_channel > 0xE)
	{
		iPF_channel = (unsigned)-1;
	}
	pCard->_iPF_channel = iPF_channel;
}

/**
 * [CommComp_turntable__s_card_hydraulic_comp_event_callback description]
 * @param pComp  [description]
 * @param pEvent [description]
 */
void CommComp_turntable__s_card_hydraulic_comp_event_callback(CommComp_S* pComp, CommEvent_U* pEvent)
{
	struct CommComp_turntable__s_card_hydraulic* pCard;

	DEBUG_ASSERT_PARAM(NULL != pComp, "null pointer");
	DEBUG_ASSERT_PARAM(NULL != pEvent, "null pointer");
	pCard = ContainerOf(pComp, struct CommComp_turntable__s_card_hydraulic, _sComp);
	switch (pEvent->eEvent)
	{
		case COMMEVENT_COMP_CHANGE_COMMTYPE:;
			{
				CommSubComp_S* psSubComp;

				psSubComp = pComp->PRI_psSubCompHead;
				switch (CommComp_GetCommType(pComp))
				{
					case COMMTYPE_CAN1:
					case COMMTYPE_CAN2:
						while (NULL != psSubComp)
						{
							psSubComp = psSubComp->PRI_psNextSubComp;
						}
						pComp->PRO_pvUnique = &pCard->_sUnique_CANopen_comp;
						CANOpenMaster_ResetComm(&pCard->_sUnique_CANopen_comp);
						break;
					case COMMTYPE_SUM:
						while (NULL != psSubComp)
						{
							psSubComp->PRO_pvUnique = NULL;
							psSubComp               = psSubComp->PRI_psNextSubComp;
						}
						pComp->PRO_pvUnique = NULL;
						SCommComp_InitCustomFunc(pComp, NULL);
						break;
					default:
						DEBUG_ASSERT_WARN(false, "case error");
						break;
				}
			}
			break;
		case COMMEVENT_SUBCOMP_ADD:
		case COMMEVENT_SUBCOMP_DEL:
			break;
		default:
			DEBUG_ASSERT_WARN(false, "case error");
			break;
	}
}

void CommComp_turntable__s_card_HT_comp_event_callback(CommComp_S* pComp, CommEvent_U* pEvent)
{
	struct CommComp_turntable__s_card_HT* pCard;

	DEBUG_ASSERT_PARAM(NULL != pComp, "null pointer");
	DEBUG_ASSERT_PARAM(NULL != pEvent, "null pointer");
	pCard = ContainerOf(pComp, struct CommComp_turntable__s_card_HT, _sComp);
	switch (pEvent->eEvent)
	{
		case COMMEVENT_COMP_CHANGE_COMMTYPE:;
			{
				CommSubComp_S* psSubComp;

				psSubComp = pComp->PRI_psSubCompHead;
				switch (CommComp_GetCommType(pComp))
				{
					case COMMTYPE_CAN1:
					case COMMTYPE_CAN2:
						while (NULL != psSubComp)
						{
							psSubComp = psSubComp->PRI_psNextSubComp;
						}
						pComp->PRO_pvUnique = &pCard->_sUnique_CANopen_comp;
						CANOpenMaster_ResetComm(&pCard->_sUnique_CANopen_comp);
						break;
					case COMMTYPE_SUM:
						while (NULL != psSubComp)
						{
							psSubComp->PRO_pvUnique = NULL;
							psSubComp               = psSubComp->PRI_psNextSubComp;
						}
						pComp->PRO_pvUnique = NULL;
						SCommComp_InitCustomFunc(pComp, NULL);
						break;
					default:
						DEBUG_ASSERT_WARN(false, "case error");
						break;
				}
			}
			break;
		case COMMEVENT_SUBCOMP_ADD:
		case COMMEVENT_SUBCOMP_DEL:
			break;
		default:
			DEBUG_ASSERT_WARN(false, "case error");
			break;
	}
}

/**
 * [CommComp_turntable__s_card_hydraulic_CANopen_SDO_config_finish description]
 * @param  psCompUnique [description]
 * @param  psRetSDOMsg  [description]
 * @param  wConfigIndex [description]
 * @return              [description]
 */
bool CommComp_turntable__s_card_hydraulic_CANopen_SDO_config_finish(struct CANOpenCompUnique_S* psCompUnique, struct CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex)
{
	const CANOpenCommSDOMsg_S asSDOMsgConfigTab[] = {
		// { 0x1000, 0x00, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
		// { 0x1018, 0x01, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
		// { 0x1018, 0x02, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
		{ 0x100C, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, HYDRAULIC_HEARTBEAT_CYCLE },
		{ 0x100D, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, HYDRAULIC_LOST_MAX },

		{ 0x1800, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
	};

	DEBUG_ASSERT_PARAM(NULL != psRetSDOMsg, "null pointer");
	if (wConfigIndex < (sizeof(asSDOMsgConfigTab) / sizeof(*asSDOMsgConfigTab)))
	{
		*psRetSDOMsg = asSDOMsgConfigTab[wConfigIndex];
		switch (psRetSDOMsg->tODID)
		{
			case 0x1800:
				if (0x05 == psRetSDOMsg->bODSubIndex)
				{
					struct CommComp_turntable__s_card_hydraulic* pHydraulic;

					pHydraulic           = ContainerOf(psCompUnique, struct CommComp_turntable__s_card_hydraulic, _sUnique_CANopen_comp);
					psRetSDOMsg->dwValue = pHydraulic->_iComm_cycle;
				}
			default:
				break;
		}
		return false;
	}
	return true;
}

bool CommComp_turntable__s_card_HT_CANopen_SDO_config_finish(struct CANOpenCompUnique_S* psCompUnique, struct CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex)
{
	const CANOpenCommSDOMsg_S asSDOMsgConfigTab[] = {
		// { 0x1000, 0x00, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
		// { 0x1018, 0x01, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
		// { 0x1018, 0x02, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
		// { 0x100C, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 2, HYDRAULIC_HEARTBEAT_CYCLE },
		// { 0x100D, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 1, HYDRAULIC_LOST_MAX },
		{ 0x1017, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 2, HYDRAULIC_HEARTBEAT_CYCLE },

		{ 0x1800, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 2, 0 },
	};

	DEBUG_ASSERT_PARAM(NULL != psRetSDOMsg, "null pointer");
	if (wConfigIndex < (sizeof(asSDOMsgConfigTab) / sizeof(*asSDOMsgConfigTab)))
	{
		*psRetSDOMsg = asSDOMsgConfigTab[wConfigIndex];
		switch (psRetSDOMsg->tODID)
		{
			case 0x1800:
				if (0x05 == psRetSDOMsg->bODSubIndex)
				{
					struct CommComp_turntable__s_card_HT* pCard;

					pCard                = ContainerOf(psCompUnique, struct CommComp_turntable__s_card_HT, _sUnique_CANopen_comp);
					psRetSDOMsg->dwValue = pCard->_iComm_cycle;
				}
			default:
				break;
		}
		return false;
	}
	return true;
}

/**
 * [CommComp_turntable__s_card_hydraulic_CANopen_event_callback description]
 * @param pUnique     [description]
 * @param pData_frame [description]
 * @param eEvent_type [description]
 */
void CommComp_turntable__s_card_hydraulic_CANopen_event_callback(CANOpenCompUnique_S* pUnique, CANOpenDataFrame_S* pData_frame, E_CANOpenEventType eEvent_type)
{
	struct CommComp_turntable__s_card_hydraulic* pCard;

	DEBUG_ASSERT_PARAM(NULL != pUnique, "null pointer");
	pCard = ContainerOf(pUnique, struct CommComp_turntable__s_card_hydraulic, _sUnique_CANopen_comp);
	switch (eEvent_type)
	{
		case CANOPENEVENTTYPE_RECVED_DATAFRAME:;
			{
				E_CANOpenFuncID eFuncID;

				DEBUG_ASSERT_PARAM(NULL != pData_frame, "null pointer");
				eFuncID = CANOpen_GetFuncIDByCOBID(pData_frame->uHead.sStand.bCOBID);
				switch (eFuncID)
				{
					case CANOPENFUNCID_SEND_PDO1:
						pCard->i201000 = pData_frame->uData.sU16.bData0;
						pCard->i3EC800 = pData_frame->uData.sU16.bData1;
						pCard->i201100 = pData_frame->uData.sU16.bData2;
						pCard->i201200 = pData_frame->uData.sU16.bData3;
						CANOpenCHBManager_RecvedMsg(&pCard->_sPDOManager);
						break;
					case CANOPENFUNCID_NMT_ERROR_CTRL:
						CANOpenCHBManager_RecvedMsg(&pCard->_sCHBManager);
						break;
					case CANOPENFUNCID_SEND_SDO:
						if (CANOPENNODESTATE_PREOPERATIONAL == pUnique->PRO_bNodeState)
						{
							ODID_T tODID;
							bool   bErr;
							WORD   wErrCode;

							bErr  = false;
							tODID = ((pData_frame->uData.sSDO.bODID_H << 8) | (pData_frame->uData.sSDO.bODID_L));
							switch (tODID)
							{
								case 0x1000:
									if ((0x00 == pData_frame->uData.sSDO.bODSubIndex) && (0x00100000 != pData_frame->uData.sSDO.bData))
									{
										bErr     = true;
										wErrCode = ERROR_ID_REMOTEIO_PICKERDEVICE_VERIFY_FAIL;
									}
									break;
								case 0x1018:
									if ((0x01 == pData_frame->uData.sSDO.bODSubIndex) && (0x000005BE != pData_frame->uData.sSDO.bData))
									{
										bErr     = true;
										wErrCode = ERROR_ID_REMOTEIO_PICKERDEVICE_VERIFY_FAIL;
									}
									break;
								default:
									break;
							}
							if (bErr)
							{
								Set_CommChipErrorDetail(ACTAXIS_MOLD, wErrCode, pUnique->PRO_bNodeID);
								CANOpenMaster_ResetComm(pUnique);
							}
						}
					default:
						break;
				}
			}
			break;
		case CANOPENEVENTTYPE_CHANGE_NODESTATE:
		{
			WORD wLinkState;

			wLinkState = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);
			if (CANOPENNODESTATE_OPERATIONAL == pUnique->PRO_bNodeState)
			{
				wLinkState |= (WORD)1 << pCard->_iRemote_index;
				SCommComp_InitCustomFunc(&pCard->_sComp, CommComp_turntable__s_card_hydraulic_CANopen_custom_operation);
				ResetTimer(&pCard->_sPDOSendTime, pCard->_iComm_cycle);
			}
			else
			{
				wLinkState &= ~((WORD)1 << pCard->_iRemote_index);
				SCommComp_InitCustomFunc(&pCard->_sComp, NULL);
			}
			DB_SetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE, wLinkState);
			CANOpenCHBManager_Reset(&pCard->_sCHBManager);
			CANOpenCHBManager_Reset(&pCard->_sPDOManager);
		}
		break;
		default:
			DEBUG_ASSERT_WARN(false, "case error");
			break;
	}
}

void CommComp_turntable__s_card_HT_CANopen_event_callback(CANOpenCompUnique_S* pUnique, CANOpenDataFrame_S* pData_frame, E_CANOpenEventType eEvent_type)
{
	struct CommComp_turntable__s_card_HT* pCard;

	DEBUG_ASSERT_PARAM(NULL != pUnique, "null pointer");
	pCard = ContainerOf(pUnique, struct CommComp_turntable__s_card_HT, _sUnique_CANopen_comp);
	switch (eEvent_type)
	{
		case CANOPENEVENTTYPE_RECVED_DATAFRAME:;
			{
				E_CANOpenFuncID eFuncID;

				DEBUG_ASSERT_PARAM(NULL != pData_frame, "null pointer");
				eFuncID = CANOpen_GetFuncIDByCOBID(pData_frame->uHead.sStand.bCOBID);
				switch (eFuncID)
				{
					case CANOPENFUNCID_SEND_PDO1:
						pCard->wStatusWord = pData_frame->uData.sU16.bData0;

						if (((pCard->wStatusWord & 0x2) || (0 != pCard->wError)) && CheckTimerOut(&pCard->_sErrorReqTime))
						{
							E_CANChannel              eChannel;
							CANOpenCommSDOMsg_S const sSDOMsg = { 0x2074, 0x01, CANCOMMSDOACCESS_UPLOAD, 0, 0 };

							ResetTimer(&pCard->_sErrorReqTime, ELECTRIC_ERROR_REQ_CYCLE);
							pCard->_sUnique_CANopen_comp.PUB_blFreeSDO = TRUE;
							eChannel                                   = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(&pCard->_sComp));
							CANOpenMaster_SendSDO(eChannel, pCard->_sUnique_CANopen_comp.PRO_bNodeID, &sSDOMsg);
						}

						pCard->nActAngle     = pData_frame->uData.sU16.bData1;
						pCard->nActSpd       = pData_frame->uData.sU16.bData2;
						pCard->nActValveVolt = pData_frame->uData.sU16.bData3;
						CANOpenCHBManager_RecvedMsg(&pCard->_sPDOManager);
						break;
					case CANOPENFUNCID_NMT_ERROR_CTRL:
						CANOpenCHBManager_RecvedMsg(&pCard->_sCHBManager);
						break;
					case CANOPENFUNCID_SEND_SDO:
						if (CANOPENNODESTATE_OPERATIONAL == pUnique->PRO_bNodeState)
						{
							ODID_T tODID;

							tODID = ((pData_frame->uData.sSDO.bODID_H << 8) | (pData_frame->uData.sSDO.bODID_L));
							switch (tODID)
							{
								case 0x2074:
									if (0x01 == pData_frame->uData.sSDO.bODSubIndex)
									{
										pCard->wError = pData_frame->uData.sSDO.bData & 0xFF;
									}
									break;
								default:
									break;
							}
						}
					default:
						break;
				}
			}
			break;
		case CANOPENEVENTTYPE_CHANGE_NODESTATE:
		{
			WORD wLinkState;

			wLinkState = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);
			if (CANOPENNODESTATE_OPERATIONAL == pUnique->PRO_bNodeState)
			{
				wLinkState |= (WORD)1 << pCard->_iRemote_index;
				SCommComp_InitCustomFunc(&pCard->_sComp, CommComp_turntable__s_card_HT_CANopen_custom_operation);
				ResetTimer(&pCard->_sPDOSendTime, pCard->_iComm_cycle);
			}
			else
			{
				wLinkState &= ~((WORD)1 << pCard->_iRemote_index);
				SCommComp_InitCustomFunc(&pCard->_sComp, NULL);
			}
			DB_SetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE, wLinkState);
			CANOpenCHBManager_Reset(&pCard->_sCHBManager);
			CANOpenCHBManager_Reset(&pCard->_sPDOManager);
		}
		break;
		default:
			DEBUG_ASSERT_WARN(false, "case error");
			break;
	}
}

/**
 * [CommComp_turntable__s_card_hydraulic_CANopen_custom_operation description]
 * @param pComp [description]
 * @param eType [description]
 */
void CommComp_turntable__s_card_hydraulic_CANopen_custom_operation(struct CommComp_S* const pComp, E_CustomFuncType eType)
{
	struct CommComp_turntable__s_card_hydraulic* pCard;
	E_CANChannel                                 eChannel;

	eChannel = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(pComp));
	pCard    = ContainerOf(pComp, struct CommComp_turntable__s_card_hydraulic, _sComp);
	DEBUG_ASSERT_ILLEGAL(CANOPENNODESTATE_OPERATIONAL == pCard->_sUnique_CANopen_comp.PRO_bNodeState, "state error.");
	switch (eType)
	{
		case CUSTOMFUNCTYPE_FAST:
			if (CheckTimerOut(&pCard->_sPDOSendTime))
			{
				ResetTimer(&pCard->_sPDOSendTime, pCard->_iComm_cycle);
				if ((unsigned)-1 != pCard->_iPF_channel)
				{
					extern WORD            g_wCommXPLC_backup;
					extern WORD            g_wHydraulicSpdSetpoint;
					CANOpenDataFrameData_U uData_frame;
					DBIndex_T              iIndex_press; //, iIndex_spd;

					iIndex_press = DBINDEX_M28_MONITOR_COMM_MB_CHL1_SET_PRESS_SCALE + pCard->_iPF_channel * 2;
					// iIndex_spd   = DBINDEX_M28_MONITOR_COMM_MB_CHL1_SET_FLOW + pCard->_iPF_channel * 2;

					uData_frame.sU16.bData0 = g_wCommXPLC_backup;       // STAT.RSVOPER + 1.
					uData_frame.sU16.bData1 = DB_GetWORD(iIndex_press); // HYDR.OUT_VALUE.
					// uData_frame.sU16.bData2 = DB_GetWORD(iIndex_press + 1); // HYDR.OUT_VALUE + 1. // CX mask 20190820 for HT rotation
					//uData_frame.sU16.bData2 = _IQ12(0.001 * DB_GetWORD(iIndex_spd)); // HYDR.OUT_VALUE + 1.
					uData_frame.sU16.bData2 = _IQ12(0.001 * g_wHydraulicSpdSetpoint); // HYDR.OUT_VALUE + 1.
					CANOpenMaster_SendPDO(eChannel, pCard->_sUnique_CANopen_comp.PRO_bNodeID, CANOPENFUNCID_RECV_PDO1, 6, &uData_frame);
				}
			}
			break;
		case CUSTOMFUNCTYPE_SLOW:
		{
			E_CANOpenCHBCheck eCheck;

			eCheck = CANOpenCHBManager_Check(&pCard->_sCHBManager);
			if (CANOPENCHBCHECK_IDLE != eCheck)
			{
				switch (eCheck)
				{
					case CANOPENCHBCHECK_CONFLICT:
						Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_REMOTEIO_NODEIDCONFLICT, pCard->_sUnique_CANopen_comp.PRO_bNodeID);
					// do case of CANOPENCHBCHECK_OFFLINE.
					case CANOPENCHBCHECK_OFFLINE:
						CANOpenMaster_ResetComm(&pCard->_sUnique_CANopen_comp);
						break;
					default:
						CANOpenMaster_SendNodeGruading(eChannel, pCard->_sUnique_CANopen_comp.PRO_bNodeID);
						break;
				}
			}
			eCheck = CANOpenCHBManager_Check(&pCard->_sPDOManager);
			if (CANOPENCHBCHECK_OFFLINE == eCheck)
			{
				CANOpenMaster_ResetComm(&pCard->_sUnique_CANopen_comp);
				break;
			}
		}
		break;
		default:
			break;
	}
}

void CommComp_turntable__s_card_HT_CANopen_custom_operation(struct CommComp_S* const pComp, E_CustomFuncType eType)
{
	struct CommComp_turntable__s_card_HT* pCard;
	E_CANChannel                          eChannel;

	eChannel = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(pComp));
	pCard    = ContainerOf(pComp, struct CommComp_turntable__s_card_HT, _sComp);
	DEBUG_ASSERT_ILLEGAL(CANOPENNODESTATE_OPERATIONAL == pCard->_sUnique_CANopen_comp.PRO_bNodeState, "state error.");
	switch (eType)
	{
		case CUSTOMFUNCTYPE_FAST:
			if (CheckTimerOut(&pCard->_sPDOSendTime))
			{
				ResetTimer(&pCard->_sPDOSendTime, pCard->_iComm_cycle);
				//if ((unsigned)-1 != pCard->_iPF_channel)
				{
					extern WORD            g_wCommXPLC_backup;
					extern WORD            g_wHydraulicSpdSetpoint;
					CANOpenDataFrameData_U uData_frame;

					uData_frame.sU16.bData0 = g_wCommXPLC_backup;      // STAT.RSVOPER + 1.
					uData_frame.sU16.bData1 = g_wHydraulicSpdSetpoint; // HYDR.OUT_VALUE.
					CANOpenMaster_SendPDO(eChannel, pCard->_sUnique_CANopen_comp.PRO_bNodeID, CANOPENFUNCID_RECV_PDO1, 6, &uData_frame);
				}
			}
			break;
		case CUSTOMFUNCTYPE_SLOW:
		{
			E_CANOpenCHBCheck eCheck;

			eCheck = CANOpenCHBManager_Check(&pCard->_sCHBManager);
			if (CANOPENCHBCHECK_IDLE != eCheck)
			{
				switch (eCheck)
				{
					case CANOPENCHBCHECK_CONFLICT:
						Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_REMOTEIO_NODEIDCONFLICT, pCard->_sUnique_CANopen_comp.PRO_bNodeID);
					// do case of CANOPENCHBCHECK_OFFLINE.
					case CANOPENCHBCHECK_OFFLINE:
						CANOpenMaster_ResetComm(&pCard->_sUnique_CANopen_comp);
						break;
					default:
						break;
				}
			}
			eCheck = CANOpenCHBManager_Check(&pCard->_sPDOManager);
			if (CANOPENCHBCHECK_OFFLINE == eCheck)
			{
				CANOpenMaster_ResetComm(&pCard->_sUnique_CANopen_comp);
				break;
			}
		}
		break;
		default:
			break;
	}
}

/**
 * [CommComp_turntable__s_card_hydraulic_init description]
 * @param pCard [description]
 */
void CommComp_turntable__s_card_electric_init(struct CommComp_turntable__s_card_electric* pCard)
{
	unsigned i;

	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	SCommComp_Init(&pCard->_sComp);
	SCommComp_InitEventCallBack(&pCard->_sComp, CommComp_turntable__s_card_electric_comp_event_callback);

	SCANOpenCompUnique_Init(&pCard->_sUnique_CANopen_comp);
	SCANOpenCompUnique_InitEventCallBack(&pCard->_sUnique_CANopen_comp, CommComp_turntable__s_card_electric_CANopen_event_callback);
	SCANOpenCompUnique_InitConfigInfo(&pCard->_sUnique_CANopen_comp, CommComp_turntable__s_card_electric_CANopen_SDO_config_finish);

	CANOpenCHBManager_Init(&pCard->_sCHBManager);
	pCard->_sCHBManager.iLose_max     = ELECTRIC_LOST_MAX;
	pCard->_sCHBManager.iConflict_max = 5;
	pCard->_sCHBManager.iTick         = ELECTRIC_HEARTBEAT_CYCLE;

	for (i = 0; i < (sizeof(pCard->_aPDOManager) / sizeof(*pCard->_aPDOManager)); ++i)
	{
		CANOpenCHBManager_Init(&pCard->_aPDOManager[i]);
		pCard->_aPDOManager[i].iLose_max     = 50;
		pCard->_aPDOManager[i].iConflict_max = 50;
		//pCard->_aPDOManager[i].iTick         = ELECTRIC_PDO_CYCLE; // hankin 20171031 mask.
	}
	ResetTimer(&pCard->_sPDOSendTime, 0);
	ResetTimer(&pCard->_sErrorReqTime, 0); // hankin 20171031 add.

	SCommSubComp_Init(&pCard->_sSubcomp);
	SCANOpenSubCompUnique_Init(&pCard->_sUnique_CANopen_subcomp);

	pCard->_iRemote_index    = (unsigned)-1;
	pCard->_iTurntable_index = (unsigned)-1;
	pCard->_iPF_channel      = (unsigned)-1;
	pCard->_iComm_cycle      = 0;

	pCard->i40DA00 = 0;
	pCard->i40DB00 = 0;
	pCard->i40DC00 = 0;
	pCard->i202500 = 0;

	// pCard->i604000 = 0;
	// pCard->i420800 = 0;
	// pCard->i60FF00 = 0;

	pCard->i604100 = 0;
	pCard->i420900 = 0;
	pCard->i414200 = 0;
	pCard->f200100 = 0;

	SCommComp_AddSubComp(&pCard->_sComp, &pCard->_sSubcomp);
}

/**
 * [CommComp_turntable__s_card_electric_add description]
 * @param  pCard            [description]
 * @param  iRemote_index    [description]
 * @param  iTurntable_index [description]
 * @param  eComm_type       [description]
 * @param  iNode_ID         [description]
 * @param  iComm_cycle      [description]
 * @return                  [description]
 */
bool CommComp_turntable__s_card_electric_add(struct CommComp_turntable__s_card_electric* pCard, unsigned iRemote_index, unsigned iTurntable_index, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle)
{
	WORD i;

	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	switch (eComm_type)
	{
		case COMMTYPE_CAN1:
		case COMMTYPE_CAN2:
			SCommComp_SetCommType(&pCard->_sComp, eComm_type);
			SCANOpenCompUnique_SetNodeID(&pCard->_sUnique_CANopen_comp, iNode_ID);
			for (i = 0; i < (sizeof(pCard->_aPDOManager) / sizeof(*pCard->_aPDOManager)); ++i)
				pCard->_aPDOManager[i].iTick = iComm_cycle;
			pCard->_iComm_cycle      = iComm_cycle;
			pCard->_iRemote_index    = iRemote_index;
			pCard->_iTurntable_index = iTurntable_index;
			break;
		default:
			DEBUG_ASSERT_WARN(true, "case error");
			return false;
	}
	return true;
}

/**
 * [CommComp_turntable__s_card_electric_del description]
 * @param pCard [description]
 */
inline void CommComp_turntable__s_card_electric_del(struct CommComp_turntable__s_card_electric* pCard)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	SCommComp_SetCommType(&pCard->_sComp, COMMTYPE_SUM);
	pCard->_iRemote_index    = (unsigned)-1;
	pCard->_iTurntable_index = (unsigned)-1;
	pCard->_iPF_channel      = (unsigned)-1;
}

/**
 * [CommComp_turntable__s_card_electric_set_PF_channel description]
 * @param pCard       [description]
 * @param iPF_channel [description]
 */
inline void CommComp_turntable__s_card_electric_set_PF_channel(struct CommComp_turntable__s_card_electric* pCard, unsigned iPF_channel)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	--iPF_channel;
	if (iPF_channel > 0xE)
	{
		iPF_channel = (unsigned)-1;
	}
	pCard->_iPF_channel = iPF_channel;
}

/**
 * [CommComp_turntable__s_card_electric_comp_event_callback description]
 * @param pComp  [description]
 * @param pEvent [description]
 */
void CommComp_turntable__s_card_electric_comp_event_callback(CommComp_S* pComp, CommEvent_U* pEvent)
{
	struct CommComp_turntable__s_card_electric* pCard;

	DEBUG_ASSERT_PARAM(NULL != pComp, "null pointer");
	DEBUG_ASSERT_PARAM(NULL != pEvent, "null pointer");
	pCard = ContainerOf(pComp, struct CommComp_turntable__s_card_electric, _sComp);
	switch (pEvent->eEvent)
	{
		case COMMEVENT_COMP_CHANGE_COMMTYPE:;
			{
				CommSubComp_S* psSubComp;

				psSubComp = pComp->PRI_psSubCompHead;
				switch (CommComp_GetCommType(pComp))
				{
					case COMMTYPE_CAN1:
					case COMMTYPE_CAN2:
						while (NULL != psSubComp)
						{
							psSubComp = psSubComp->PRI_psNextSubComp;
						}
						pComp->PRO_pvUnique = &pCard->_sUnique_CANopen_comp;
						CANOpenMaster_ResetComm(&pCard->_sUnique_CANopen_comp);
						break;
					case COMMTYPE_SUM:
						while (NULL != psSubComp)
						{
							psSubComp->PRO_pvUnique = NULL;
							psSubComp               = psSubComp->PRI_psNextSubComp;
						}
						pComp->PRO_pvUnique = NULL;
						SCommComp_InitCustomFunc(pComp, NULL);
						break;
					default:
						DEBUG_ASSERT_WARN(false, "case error");
						break;
				}
			}
			break;
		case COMMEVENT_SUBCOMP_ADD:
		case COMMEVENT_SUBCOMP_DEL:
			break;
		default:
			DEBUG_ASSERT_WARN(false, "case error");
			break;
	}
}

/**
 * [CommComp_turntable__s_card_electric_CANopen_SDO_config_finish description]
 * @param  psCompUnique [description]
 * @param  psRetSDOMsg  [description]
 * @param  wConfigIndex [description]
 * @return              [description]
 */
bool CommComp_turntable__s_card_electric_CANopen_SDO_config_finish(struct CANOpenCompUnique_S* psCompUnique, struct CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex)
{
	const CANOpenCommSDOMsg_S asSDOMsgConfigTab[] = {
		// { 0x1000, 0x00, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
		// { 0x1018, 0x01, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
		// { 0x1018, 0x02, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
		{ 0x100C, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, ELECTRIC_HEARTBEAT_CYCLE },
		{ 0x100D, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, ELECTRIC_LOST_MAX },
		{ 0x1017, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },

		{ 0x1400, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x200 },
		{ 0x1400, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 255 },
		{ 0x1600, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 3 },
		{ 0x1600, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x60400010 },
		{ 0x1600, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x60FF0020 },
		{ 0x1600, 0x03, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x42080010 },

		{ 0x1800, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x180 },
		{ 0x1800, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 255 },
		{ 0x1800, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
		{ 0x1A00, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 2 },
		{ 0x1A00, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x60410010 }, // status word.
		{ 0x1A00, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x42090010 }, // the turntable operating state of the host.

		{ 0x1801, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x280 },
		{ 0x1801, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 255 },
		{ 0x1801, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
		{ 0x1A01, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 2 },
		{ 0x1A01, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x41420020 }, // turntable angle.
		{ 0x1A01, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x20010020 }, //0x20010020 }, // turntable speed.

		{ 0x40DA, 0x00, CANCOMMSDOACCESS_UPLOAD, 0, 0 }, // for calculation speed.
		{ 0x40DB, 0x00, CANCOMMSDOACCESS_UPLOAD, 0, 0 }, // for calculation speed.
		{ 0x40DC, 0x00, CANCOMMSDOACCESS_UPLOAD, 0, 0 }, // for calculation angle.
	};

	DEBUG_ASSERT_PARAM(NULL != psRetSDOMsg, "null pointer");
	if (wConfigIndex < (sizeof(asSDOMsgConfigTab) / sizeof(*asSDOMsgConfigTab)))
	{
		*psRetSDOMsg = asSDOMsgConfigTab[wConfigIndex];
		switch (psRetSDOMsg->tODID)
		{
			case 0x1400:
			case 0x1800:
			case 0x1801:
				if (0x01 == psRetSDOMsg->bODSubIndex)
				{
					psRetSDOMsg->dwValue += psCompUnique->PRO_bNodeID;
				}
				else if (0x05 == psRetSDOMsg->bODSubIndex)
				{
					struct CommComp_turntable__s_card_electric* pElectric;

					pElectric            = ContainerOf(psCompUnique, struct CommComp_turntable__s_card_electric, _sUnique_CANopen_comp);
					psRetSDOMsg->dwValue = pElectric->_iComm_cycle;
				}
			default:
				break;
		}
		return false;
	}
	return true;
}

/**
 * [CommComp_turntable__s_card_electric_CANopen_event_callback description]
 * @param pUnique     [description]
 * @param pData_frame [description]
 * @param eEvent_type [description]
 */
void CommComp_turntable__s_card_electric_CANopen_event_callback(CANOpenCompUnique_S* pUnique, CANOpenDataFrame_S* pData_frame, E_CANOpenEventType eEvent_type)
{
	struct CommComp_turntable__s_card_electric* pCard;

	DEBUG_ASSERT_PARAM(NULL != pUnique, "null pointer");
	pCard = ContainerOf(pUnique, struct CommComp_turntable__s_card_electric, _sUnique_CANopen_comp);
	switch (eEvent_type)
	{
		case CANOPENEVENTTYPE_RECVED_DATAFRAME:;
			{
				E_CANOpenFuncID eFuncID;

				DEBUG_ASSERT_PARAM(NULL != pData_frame, "null pointer");
				eFuncID = CANOpen_GetFuncIDByCOBID(pData_frame->uHead.sStand.bCOBID);
				switch (eFuncID)
				{
					case CANOPENFUNCID_SEND_PDO1:
						pCard->i604100 = pData_frame->uData.sU16.bData0;
						if (((pCard->i604100 & 0x8) || (0 != pCard->i202500)) && CheckTimerOut(&pCard->_sErrorReqTime))
						{
							E_CANChannel              eChannel;
							CANOpenCommSDOMsg_S const sSDOMsg = { 0x2025, 0x00, CANCOMMSDOACCESS_UPLOAD, 0, 0 };

							ResetTimer(&pCard->_sErrorReqTime, ELECTRIC_ERROR_REQ_CYCLE);
							pCard->_sUnique_CANopen_comp.PUB_blFreeSDO = TRUE;
							eChannel                                   = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(&pCard->_sComp));
							CANOpenMaster_SendSDO(eChannel, pCard->_sUnique_CANopen_comp.PRO_bNodeID, &sSDOMsg);
						}
						pCard->i420900 = pData_frame->uData.sU16.bData1;
						CANOpenCHBManager_RecvedMsg(&pCard->_aPDOManager[0]);
						break;
					case CANOPENFUNCID_SEND_PDO2:;
						{
							FLOAT* p200100;

							pCard->i414200 = pData_frame->uData.sU32.bData0;
							p200100        = (FLOAT*)&pData_frame->uData.sU32.bData1;
							pCard->f200100 = *p200100;
							CANOpenCHBManager_RecvedMsg(&pCard->_aPDOManager[1]);
						}
						break;
					case CANOPENFUNCID_NMT_ERROR_CTRL:
						CANOpenCHBManager_RecvedMsg(&pCard->_sCHBManager);
						break;
					case CANOPENFUNCID_SEND_SDO:
						if (CANOPENNODESTATE_OPERATIONAL == pUnique->PRO_bNodeState)
						{
							ODID_T tODID;

							tODID = ((pData_frame->uData.sSDO.bODID_H << 8) | (pData_frame->uData.sSDO.bODID_L));
							switch (tODID)
							{
								case 0x2025:
									if (0x00 == pData_frame->uData.sSDO.bODSubIndex)
									{
										pCard->i202500 = pData_frame->uData.sSDO.bData;
									}
									break;
								default:
									break;
							}
						}
						else
						{
							ODID_T tODID;
							bool   bErr;
							WORD   wErrCode;

							bErr  = false;
							tODID = ((pData_frame->uData.sSDO.bODID_H << 8) | (pData_frame->uData.sSDO.bODID_L));
							switch (tODID)
							{
#if (true)
								case 0x1000:
									if ((0x00 == pData_frame->uData.sSDO.bODSubIndex) && (0x00100000 != pData_frame->uData.sSDO.bData))
									{
										bErr     = true;
										wErrCode = ERROR_ID_REMOTEIO_PICKERDEVICE_VERIFY_FAIL;
									}
									break;
								case 0x1018:
									if (((0x01 == pData_frame->uData.sSDO.bODSubIndex) && (0x000005BE != pData_frame->uData.sSDO.bData)) || ((0x02 == pData_frame->uData.sSDO.bODSubIndex) && (0x101802 != pData_frame->uData.sSDO.bData)))
									{
										bErr     = true;
										wErrCode = ERROR_ID_REMOTEIO_PICKERDEVICE_VERIFY_FAIL;
									}
									break;
#endif
								case 0x40DA:
									if (0x00 == pData_frame->uData.sSDO.bODSubIndex)
									{
										pCard->i40DA00 = pData_frame->uData.sSDO.bData;
									}
									break;
								case 0x40DB:
									if (0x00 == pData_frame->uData.sSDO.bODSubIndex)
									{
										pCard->i40DB00 = pData_frame->uData.sSDO.bData;
									}
									break;
								case 0x40DC:
									if (0x00 == pData_frame->uData.sSDO.bODSubIndex)
									{
										pCard->i40DC00 = pData_frame->uData.sSDO.bData;
									}
									break;
								default:
									break;
							}
							if (bErr)
							{
								Set_CommChipErrorDetail(ACTAXIS_MOLD, wErrCode, pUnique->PRO_bNodeID);
								CANOpenMaster_ResetComm(pUnique);
							}
						}
					default:
						break;
				}
			}
			break;
		case CANOPENEVENTTYPE_CHANGE_NODESTATE:
		{
			WORD wLinkState;

			wLinkState = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);
			if (CANOPENNODESTATE_OPERATIONAL == pUnique->PRO_bNodeState)
			{
				wLinkState |= (WORD)1 << pCard->_iRemote_index;
				SCommComp_InitCustomFunc(&pCard->_sComp, CommComp_turntable__s_card_electric_CANopen_custom_operation);
				ResetTimer(&pCard->_sPDOSendTime, pCard->_iComm_cycle);
			}
			else
			{
				wLinkState &= ~((WORD)1 << pCard->_iRemote_index);
				SCommComp_InitCustomFunc(&pCard->_sComp, NULL);
			}
			DB_SetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE, wLinkState);
			{
				unsigned i;

				CANOpenCHBManager_Reset(&pCard->_sCHBManager);
				for (i = 0; i < (sizeof(pCard->_aPDOManager) / sizeof(*pCard->_aPDOManager)); ++i)
					CANOpenCHBManager_Reset(&pCard->_aPDOManager[i]);
			}
		}
		break;
		default:
			DEBUG_ASSERT_WARN(false, "case error");
			break;
	}
}

/**
 * [CommComp_turntable__s_card_electric_CANopen_custom_operation description]
 * @param pComp [description]
 * @param eType [description]
 */
void CommComp_turntable__s_card_electric_CANopen_custom_operation(struct CommComp_S* const pComp, E_CustomFuncType eType)
{
	struct CommComp_turntable__s_card_electric* pCard;
	E_CANChannel                                eChannel;

	eChannel = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(pComp));
	pCard    = ContainerOf(pComp, struct CommComp_turntable__s_card_electric, _sComp);
	DEBUG_ASSERT_ILLEGAL(CANOPENNODESTATE_OPERATIONAL == pCard->_sUnique_CANopen_comp.PRO_bNodeState, "state error.");
	switch (eType)
	{
		case CUSTOMFUNCTYPE_FAST:
			if (CheckTimerOut(&pCard->_sPDOSendTime))
			{
				ResetTimer(&pCard->_sPDOSendTime, pCard->_iComm_cycle);
				if ((unsigned)-1 != pCard->_iPF_channel)
				{
					extern WORD            g_wCommXPLC_backup;
					extern WORD            g_wManualKey_ClrErrFlag;
					CANOpenDataFrameData_U uData_frame;
					DBIndex_T              iIndex_flow;
					DWORD                  iData;

					if (DB_GetWORD(DBINDEX_M28_MONITOR_COMM_MB_MOTORENABLE) & 1)
					{
						iData = 0xF;
					}
					else
					{
						iData = 0;
					}
					if (g_wCommXPLC_backup & 0x8000)
					{
						iData |= (WORD)1 << 4;
					}
					if (g_wManualKey_ClrErrFlag && (0 != pCard->i202500))
					{
						iData |= (WORD)1 << 7;
					}
					uData_frame.sU16.bData0 = iData;

					iIndex_flow             = DBINDEX_M28_MONITOR_COMM_MB_CHL1_SET_FLOW + pCard->_iPF_channel * 2;
					iData                   = DB_GetWORD(iIndex_flow);
					iData                   = _IQ24(iData * 0.001);
					uData_frame.sU16.bData1 = iData & 0xFFFF;
					uData_frame.sU16.bData2 = (iData >> 16) & 0xFFFF;
					uData_frame.sU16.bData3 = g_wCommXPLC_backup & 0x7FFF;
					CANOpenMaster_SendPDO(eChannel, pCard->_sUnique_CANopen_comp.PRO_bNodeID, CANOPENFUNCID_RECV_PDO1, 8, &uData_frame);
				}
			}
			break;
		case CUSTOMFUNCTYPE_SLOW:;
			{
				E_CANOpenCHBCheck eCheck;
				unsigned          i;

				eCheck = CANOpenCHBManager_Check(&pCard->_sCHBManager);
				if (CANOPENCHBCHECK_IDLE != eCheck)
				{
					switch (eCheck)
					{
						case CANOPENCHBCHECK_CONFLICT:
							Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_REMOTEIO_NODEIDCONFLICT, pCard->_sUnique_CANopen_comp.PRO_bNodeID);
						// do case of CANOPENCHBCHECK_OFFLINE.
						case CANOPENCHBCHECK_OFFLINE:
							CANOpenMaster_ResetComm(&pCard->_sUnique_CANopen_comp);
							break;
						default:
							CANOpenMaster_SendNodeGruading(eChannel, pCard->_sUnique_CANopen_comp.PRO_bNodeID);
							break;
					}
				}
				for (i = 0; i < (sizeof(pCard->_aPDOManager) / sizeof(*pCard->_aPDOManager)); ++i)
				{
					eCheck = CANOpenCHBManager_Check(&pCard->_aPDOManager[i]);
					if (CANOPENCHBCHECK_OFFLINE == eCheck)
					{
						CANOpenMaster_ResetComm(&pCard->_sUnique_CANopen_comp);
						break;
					}
				}
			}
			break;
		default:
			break;
	}
}

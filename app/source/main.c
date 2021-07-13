/*==============================================================================+
|  Function :                                                                   |
|  Task     :  Source File                                                      |
|-------------------------------------------------------------------------------|
|  Compile  :                                                                   |
|  Link     :                                                                   |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : Anders.Zhang                                                      |
|  Version  : V1.00                                                             |
|  Creation : 2013/01/07                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#include "Access.h"
#include "GeneralRW.h"
#include "CommComp_EtherCat.h"
#include "CommComp_PwmtrCard.h"
#include "CommComp_RemoteIO.h"
#include "CommComp_TemperCard.h"
#include "CommComp_turntable.h"
#include "CommComp_Driver.h"
#include "CommComp_HTRobot.h"
#include "Error_App.h"
#include "Task_Led.h"
#include "Task_Watch.h"
#include "common.h"
#include "eeprom.h"
#include "objdef.h"
#include "system.h"
#include "task.h"
#include "taskEtherCATMaster_app.h"
#include "taskTemper.h"
#include "task_ethercatmaster.h"
#include "taskio.h"
#include "tasknet.h"
#include "tasknet_PC.h"
#include "tasknet_master.h"
#include "version.h"
#include "version_kernel.h"
#include "comp_fifo.h"
#include "comp_fifo2.h"
#include "Rexroth.h"
#include "CommComp_CANOpenSlave.h"
#include "memory_manager.h"
#include "kernel/tftp/tftp_server.h"
/*==============================================================================+
|           Constant                                                            |
+==============================================================================*/

/*==============================================================================+
|           Type definition                                                     |
+==============================================================================*/

/*==============================================================================+
|           Macro definition                                                    |
+==============================================================================*/
/*==============================================================================+
|           Globle Variables                                                    |
+==============================================================================*/

/*==============================================================================+
|           Class declaration -                                                 |
+==============================================================================*/
/*------------------------------------------------------------------------------+
|           Helpers Implementation                                              |
+------------------------------------------------------------------------------*/
void        MainISR(void);
extern void RunTaskNet_HMI();
extern void RunTaskHMIFifoRead();

static void* Memcpy(void * s1, void const * s2, register size_t n);

static void  stack_init(DWORD* index);
static DWORD stack_scan(DWORD* index);
/*------------------------------------------------------------------------------+
|           Inner Globle Variable                                               |
+------------------------------------------------------------------------------*/
extern Uint16 RamFuncs_loadstart;
extern Uint16 RamFuncs_loadend;
extern Uint16 RamFuncs_runstart;
extern Uint16 RamFuncs_loadstart1;
extern Uint16 RamFuncs_loadend1;
extern Uint16 RamFuncs_runstart1;

DWORD         g_dwTickMainISR = 0;
DWORD         g_dwTickWhile   = 0;
CPULOAD       g_CPULoadWhile  = { 0, 0, 0, 0 };
volatile WORD u_bExecFlag     = FALSE;
//#define TEST_OUTRAM
#ifdef TEST_OUTRAM
DWORD  g_dwTestRAMError = 0;
DWORD  g_dwTestRAMData  = 0;
DWORD* g_pdwTestRAMAddr = 0;
#endif

DWORD u_dwStackScan;

//20210608 add
#pragma DATA_SECTION(heap, "heap");
static uintptr_t heap[1536 / sizeof(uintptr_t)];
/*------------------------------------------------------------------------------+
|           Constructor and destructor                                          |
+------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------+
|           Operations                                                          |
+------------------------------------------------------------------------------*/
int main()
{
	DWORD   i;
	SYSTEM* pBootSystem = (SYSTEM*)0x33FD00;
	//const SPI         Spi     = {SPI_BAUDRATE_10M, SPI_NULL};

    Memcpy(&RamFuncs_runstart, &RamFuncs_loadstart, &RamFuncs_loadend - &RamFuncs_loadstart);

    stack_init(&u_dwStackScan);
    //file_system_init();

	//======
	//      Step 1.
	//      Initialize System Control: PLL, WatchDog, enable Peripheral Clocks
	//======
	InitSysCtrl();
	//memset(&i, 0, (0x400 - (DWORD)&i));

	//======
	//      Step 2.
	//      Initalize GPIO
	//======
	InitGpio();
	InitXintf();
	//EALLOW;0x2FFE0220
	//GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;    // SPI_SEL0. RAM Debug Add, Anders 2014-12-05.
	//EDIS;
	//======
	//      Step 3.
	//      Clear all interrupts and initialize PIE vector table: Disable CPU interrupts
	//======
	DINT;

	//======
	//      Disable CPU interrupts and clear all CPU interrupt flags
	//======
	IER = 0x0000;
	IFR = 0x0000;

	//======
	//      Initialize the PIE control registers to their default state.
	//      The default state is all PIE interrupts disabled and flags
	//      are cleared.
	//======
	InitPieCtrl();

	//======
	//      Initialize the PIE vector table with pointers to the shell Interrupt
	//      Service Routines (ISR). cthis will populate the entire table.
	//======
	InitPieVectTable();

	//EALLOW;
	//PieVectTable.TINT0 = &cpu_timer0_isr;         // Interrupts that are used in cthis example
	//EDIS;

	// Section secureRamFuncs contains user defined code that runs from CSM secured RAM
	//memcpy(&RamFuncs_runstart,
	//    &RamFuncs_loadstart,
	//    &RamFuncs_loadend - &RamFuncs_loadstart);

	memcpy(&RamFuncs_runstart1,
		&RamFuncs_loadstart1,
		&RamFuncs_loadend1 - &RamFuncs_loadstart1);
	memory_manager_init(heap, sizeof(heap));

	InitFlash(); // Initialize the FLASH

	//======
	//      Step 4.
	//      Initialize timer
	//======
	//InitCpuTimers();                                      // For cthis example, only initialize the Cpu Timers
	//ConfigCpuTimer(&CpuTimer0, 150, CPUTIMER0_SLICE);     // Configure CPU-Timer 0 to interrupt every msecond:
	//IER |= M_INT1;                                        // Enable CPU INT1 which is connected to CPU-Timer 0
	//PieCtrlRegs.PIEIER1.bit.INTx7 = 1;                    // Enable TINT0 in the PIE: Group 1 interrupt 7

	//EALLOW;                                                           // cthis is needed to write to EALLOW protected registers
	//PieVectTable.T1OFINT = &cpu_timer0_isr;
	//EDIS;
	InitCpuTimers(); // For cthis example, only initialize the Cpu Timers
	                 //ConfigCpuTimer(&CpuTimer0, 150, CPUTIMER0_SLICE);
	                 //PieCtrlRegs.PIEIER1.bit.INTx7 = 1;                            // Enable TINT0 in the PIE: Group 1 interrupt 7                                             // Enable CPU INT2 for T1UFINT and INT3 for CAPINT3 and INT6 for SPIRXINTA Enable CPU INT1 for CPU-Timer 0

#if defined(D_HYB5_335) || defined(D_SMC335M1) || defined(D_SMC335M2)
	ConfigCpuTimer(&CpuTimer0, 150, CPUTIMER0_SLICE);
	EALLOW; // cthis is needed to write to EALLOW protected registers
	PieVectTable.TINT0 = &cpu_timer0_isr;
	EDIS;

	PieCtrlRegs.PIEIER1.all = M_INT7; // Enable PIE group 1 interrupt 7 for TINT0
	IER |= M_INT1;                    // Enable CPU INT2 for T1UFINT and INT3 for CAPINT3 and INT6 for SPIRXINTA Enable CPU INT1 for CPU-Timer 0

#elif defined(D_DSP2812)
	EvaRegs.GPTCONA.all         = 0; // Setup Timer 1 Registers (EV A) 20091119 Modified.
	EvaRegs.EVAIMRA.bit.T1UFINT = 1; // Waiting for enable flag set
	EvaRegs.EVAIFRA.bit.T1UFINT = 1; // Enable Underflow interrupt bits for GP timer 1

	EALLOW; // cthis is needed to write to EALLOW protected registers
	PieVectTable.T1UFINT = &cpu_timer0_isr;
	EDIS; // cthis is needed to disable write to EALLOW protected registers

	PieCtrlRegs.PIEIER2.all = M_INT6; // Enable PIE group 2 interrupt 6 for T1UFINT
	IER |= M_INT2;                    // Enable CPU INT2 for T1UFINT and INT3 for CAPINT3 and INT6 for SPIRXINTA Enable CPU INT1 for CPU-Timer 0
#endif
	//======
	//      Step 5.
	//      Enable global Interrupts and higher priority real-time debug events
	//======
	EINT; // Enable Global interrupt INTM
	ERTM; // Enable Global realtime interrupt DBGM

	//======
	//      Step 6.
	//      Initialize all the Device Peripherals
	//======
	//InitEv();                                                         // Init Ev for ADC conversion triggle
	//InitQEPA();                                                       // Init QEPA for Encoder
	//_InstallRS232();                                                  // Install RS232 device
	//WriteString_EEPROM(EEPADDR_MAC_START, g_awEepromMAC, 3);          // Test.
	ReadString_EEPROM(EEPADDR_MAC_START, g_awEepromMAC, 3); // Get MAC address.
	g_awEepromMAC[0] = 0xE000;                              // Anders 2016-3-26, Modify.

	for (i = 0; i < MAX_NETDEVICE; i++)
	{
		_InstallNetDevice(i); // Install Net device
	}
	//Enable_EEPROM_ax88796b(0);
	//g_wEEPROM0 = 0x1234;
	//WriteString_EEPROM_ax88796b(0, 5, &g_wEEPROM0, 1);
	//g_wEEPROM0 = 0;
	//ReadString_EEPROM_ax88796b(0, 5, &g_wEEPROM0, 1);

	//======
	//      Step 7.
	//      EthCAT By SPI.
	//======
	//_OpenSPI((SPI*)&Spi);

	//IER |= M_INT1;
	////External Interrupt Control Register
	//XIntruptRegs.XINT1CR.bit.ENABLE   = 1;        // 0    enable/disable
	//XIntruptRegs.XINT1CR.bit.POLARITY     = 0;        // 2    0 Interrupt generated on a falling edge (high-to-low transition)
	//                                              //      1 Interrupt generated on a rising edge (low-to-high transition)
	//XIntruptRegs.XINT2CR.bit.ENABLE   = 1;        // 0    enable/disable
	//XIntruptRegs.XINT2CR.bit.POLARITY     = 0;        // 2    pos/neg triggered
	//XIntruptRegs.XNMICR.bit.ENABLE        = 0;        // 0    enable/disable
	//XIntruptRegs.XNMICR.bit.SELECT        = 1;        // 1    Timer 1 or XNMI connected to INT13
	//XIntruptRegs.XNMICR.bit.POLARITY  = 0;        // 2    pos/neg triggered
	//IER |= M_INT13;

	//======
	//      Step 8.
	//      Create all tasks
	//======
	//Init_CompDB();                                                                              //{"THJ": {"Add": [{"Time": "2016-02-26", "Note": "test Task DB."}]}}
	Create_Task(&g_dwSystemUpTick);

	Create_TaskIO();
	//CreateTaskLed(RT_ALARM, 1000);              // LED task (Specify install devices Okey)
	//LED_OFF(LED_CPUHEART);
	Init_Led();

	//Init_Task_DB();                                                                             //{"THJ": {"Add": [{"Time": "2016-02-26", "Note": "test Task DB."}]}}
	Init_DB();
	Init_Version();
	WriteDB_DWORD(DBINDEX_M28_MONITOR_VERSION_BOOT_DATE, pBootSystem->adwSwVersion[0]); // Anders 2015-4-22, Add Boot Version.
	GetVersion_Kernel();
	tftp_server_init();

	CreateTaskNet();
	CreateTaskNet_PC();
	CreateTaskNet_master();
	// #ifdef  DEBUG_MCDB
	// MC_CreateTaskNet_PC();
	// #endif

	//CANOpen_Open(CANOPENCHANNAL_LOCAL);
	//CANOpen_Open(CANCHANNAL_CAN1);
	//CANOpen_Open(CANCHANNAL_CAN2);
	//CANOpenMaster_Init();
	Init_EtherCat();
	Init_Comm();
	// Init_CommCanopenSlave();//SHANLL_20210323
	//Create_Comm(COMMTYPE_LOCAL);
	Create_Comm(COMMTYPE_CAN1);
	Create_Comm(COMMTYPE_CAN2);
	Create_Comm(COMMTYPE_ETHERCAT);
	//Init_CommComp();
	//Init_CommLocal();
	//Init_CompAxisCard();
	//Init_Task_CompAxisCard();
	//Init_Task_CompPicker();

	Init_RemoteIO();
	Init_TemperCard();
	PwmtrCard_Init();
	CommComp_turntable_init();
	CommComp_driver_init();//SHANLL_20200312
	CommComp_Robot_init();//shanll add 20210429
	// CommComp_HTRobot_init();//SHANLL_20210323

	Create_Access();
	Create_GeneralRW();
	Create_Rexroth();

	//Create_TaskMO();
	//Create_TaskMoni();

	CreateTask_EthercatMaster(); // Anders 20110908.

	CreatTask_Temper(); // Create Task Temper
	Error_App_Init();
	Init_Task_Watch();

	CreatTaskEtherCATMaster_App();

	g_pFuncCritical = &MainISR; // Motion control task

	//#ifdef TEST_OUTRAM
	//LED_ON(LED_CPUHEART);
	//LED_ON(LED_ALARM);
	//for (i=0; i<(0x40000>>1); i++ )
	//{
	//  g_pdwTestRAMAddr = ((DWORD *)0x200000 + i);
	//  *g_pdwTestRAMAddr = (DWORD)g_pdwTestRAMAddr;                    // Write Data
	//
	//  //DSP28x_usDelay(1);
	//  g_dwTestRAMData = 0;
	//  g_dwTestRAMData = *g_pdwTestRAMAddr;                            // Read Data
	//  if ( (DWORD)g_pdwTestRAMAddr != g_dwTestRAMData)
	//      g_dwTestRAMError++;
	//
	//  if ( g_dwTestRAMError )
	//  {
	//      LED_OFF(LED_CPUHEART);
	//      LED_ON(LED_ALARM);
	//      while( 1 )
	//          ESTOP0;
	//  }
	//}
	//LED_OFF(LED_CPUHEART);
	//LED_OFF(LED_ALARM);
	//while( 1 );
	//#endif

	//======
	//      Step 9.
	//      Start cpu timer and start all tasks according to timer
	//======
	StartCpuTimer0(); // 150MHz CPU Freq, 1m second Period (in uSeconds)

#if defined(D_HYB5_335) || defined(D_SMC335M1) || defined(D_SMC335M2)
	StartCpuTimer1();

#elif defined(D_DSP2812)
	_OpenPWM(PWM_EVA, &pwmGEN);

#endif

	//======
	//      Step 10.
	//      Run all tasks
	//======

	// g_Card.EtherCATMaster.wAll = 1;
	// Reset_ECSlave(0, DEVICE_MT_HUB, FALSE);
	// Reset_ECSlave(0, DEVICE_HTJ6_OIL, FALSE);
	// SetAppSlaveCount(1);

	for (;;) // IDLE loop. Just sit and loop forever (optional):
	{
		//		static WORD  wExecStep         = 0;
		static DWORD dwLastTickMainISR = 0;

		CalcCPULoadStart(&g_CPULoadWhile);
		MainloopParseHMIData();//shanll add 20200512
		RunTaskNet_HMI();//shanll 20201210 move here
		if (g_Card_EtherCATMaster.wAll > 0)
		{
			DealData(ECMASTERDEVICEHANDLE);
			DealSwitch_EthercatMaster();//hankin 20201130
			DB_SetDWORD(DBINDEX_M28_MONITOR_DIAGNOSE_COMM_NET_TX_DATAGRAMS_COUNT, g_dwTxNetDatagramCount[DEVICEHANDLE_MASTER]);
		}

		DealData(DEVICEHANDLE_PC);
		RunTaskNet_PC();

		if (u_bExecFlag)
		{
			if (dwLastTickMainISR != g_dwTickMainISR)
			{
				dwLastTickMainISR = g_dwTickMainISR;

				// Anders 2016-10-21, add ECAT-Watch for HT.
				memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_ECAT_HT_OIL_STATUSWORD), &g_HT_AxisCard_Oil.outputs, sizeof(g_HT_AxisCard_Oil.outputs));
				memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_ECAT_HT_OIL_CONTROLWORD), &g_HT_AxisCard_Oil.inputs, sizeof(g_HT_AxisCard_Oil.inputs));

				memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_ECAT_HT_ELEINJ_STATUSWORD), &g_HT_AxisCard_EleInj.outputs.wStatusWord, sizeof(WORD));
				memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_ECAT_HT_ELEINJ_INJACTPOSITION), &g_HT_AxisCard_EleInj.outputs.wInjActPosiL, sizeof(g_HT_AxisCard_EleInj.outputs) - sizeof(WORD));
				memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_ECAT_HT_ELEINJ_CONTROLWORD), &g_HT_AxisCard_EleInj.inputs, sizeof(g_HT_AxisCard_EleInj.inputs));

				memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_ECAT_HT_PLUS1_INPUT_CONTROLWORD), &g_HT_AxisCard_Plus[0].inputs, sizeof(HTAXISCARD_PLUS_INPUTS));
				memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_ECAT_HT_PLUS1_OUTPUT_STATUSWORD), &g_HT_AxisCard_Plus[0].outputs, sizeof(HTAXISCARD_PLUS_OUTPUTS));
				memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_ECAT_HT_PLUS2_INPUT_CONTROLWORD), &g_HT_AxisCard_Plus[1].inputs, sizeof(HTAXISCARD_PLUS_INPUTS));
				memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_ECAT_HT_PLUS2_OUTPUT_STATUSWORD), &g_HT_AxisCard_Plus[1].outputs, sizeof(HTAXISCARD_PLUS_OUTPUTS));

				memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_ECAT_HT_PP1_INPUT_CONTROLWORD_E), &g_HT_AxisCard_PP[0].inputs,  sizeof(HTAXISCARD_PP_INPUTS));
				memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_ECAT_HT_PP1_OUTPUT_STATUSWORDL),  &g_HT_AxisCard_PP[0].outputs, sizeof(HTAXISCARD_PP_OUTPUTS));
				memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_ECAT_HT_PP2_INPUT_CONTROLWORD_E), &g_HT_AxisCard_PP[1].inputs,  sizeof(HTAXISCARD_PP_INPUTS));
				memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_ECAT_HT_PP2_OUTPUT_STATUSWORDL),  &g_HT_AxisCard_PP[1].outputs, sizeof(HTAXISCARD_PP_OUTPUTS));
				memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_ECAT_HT_PP3_INPUT_CONTROLWORD_E), &g_HT_AxisCard_PP[2].inputs,  sizeof(HTAXISCARD_PP_INPUTS));
				memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_ECAT_HT_PP3_OUTPUT_STATUSWORDL),  &g_HT_AxisCard_PP[2].outputs, sizeof(HTAXISCARD_PP_OUTPUTS));
				// Caculate CPU Loading(us). Anders 2017-4-14, add.
				DB_SetFLOAT(DBINDEX_LOCAL_AXISCARD_INTERFACE_MAX_CPU_LOADING, 0.006667 * g_dwTickWhile);
				DB_SetFLOAT(DBINDEX_LOCAL_AXISCARD_INTERFACE_CURRENT_CPU_LOADING, 0.006667 * g_CPULoad.dwCurrentCounter);

				//				Run_WhileloopTask(); // CX 20180308 changed here for loading too heavy to execute here costing much time when CMD doing(>100ms).

				// DealData(DEVICEHANDLE_PC);
				{
					DWORD result = stack_scan(&u_dwStackScan);
					if (0 != result) {
						g_sDB.M28.MONITOR.DIAGNOSE_DWORD_RESERVE1 = result;
					}
				}
			}
			Run_UnTimerintTask();

			Parse_Comm(COMMTYPE_CAN2);

			Run_SlowRun(COMMTYPE_ETHERCAT);

			u_bExecFlag = FALSE;
		}
		else
		{
			Run_Access();
			Run_GeneralRW();
			Run_Rexroth();
			Error_App_Run();
			Run_Task_Watch();
			Led_Service(100);

			// Run_WhileloopTask();    // CX 20180308 marked for loading too heavy to execute here costing much time when CMD doing(>100ms).
			//			if (!g_wHTJ6CardType) // Anders 2017-4-13, Add.
			//			{
			//				Run_SlowRun(COMMTYPE_LOCAL);
			//#if (HYBCARD_USED)
			//				Run_DriveAccess(); // Anders 2017-8-11, add.
			//#endif
			//			}

			Run_FastRun(COMMTYPE_CAN2);
			Run_SlowRun(COMMTYPE_CAN2);

			u_bExecFlag = TRUE; // Anders 2018-10-31, Add.
		}

		CalcCPULoadEnd(&g_CPULoadWhile);
		g_dwTickWhile = g_CPULoadWhile.dwCurrentCounter;
	}
}

// EVA interrupt
//======
//      1. MainISR executes one time every 1 mini-second.
//      2. Motion Control executes one time every MOTIONCAL_SLICE.
//      3. NetDeviceISR which polls net card for receiving and sending data executes
//         every 1 mini-second if possible or in preiod Motion Control doesn't work.
//      4. NetDeviceISR executes in main loop when all tasks is idle then RunTaskNet followed that.
//======

//WORD  g_wTestFlow = 0;
#ifdef _FAST_FUNC
#pragma CODE_SECTION(MainISR, "ramfuncs");
#endif
void MainISR(void)
{
	g_dwTickMainISR++;

	DealData(DEVICEHANDLE_MASTER);
	DB_SetDWORD(DBINDEX_M28_MONITOR_DIAGNOSE_COMM_NET_RX_DATAGRAMS_COUNT, g_dwRxNetDatagramCount[DEVICEHANDLE_MASTER]);

	if (g_wCycle5528J6 == 0) // Anders 2019-11-07, add.
	{
		if (g_dwTickMainISR & 1) // CheckDnTimerOut( &g_lRunECATTimer ) )
		{
			if (g_Card_EtherCATMaster.wAll > 0)
				RunTask_EthercatMaster(); // Run EtherCAT Master.
			RunTaskHMIFifoRead();
		}
		else
		{
			RunTaskNet_master();
		}
	}
	else
	{
		if (g_Card_EtherCATMaster.wAll > 0)
			RunTask_EthercatMaster(); // Run EtherCAT Master.

		if (0 != (g_dwTickMainISR & 1))
			RunTaskNet_master();
		else
		{
			RunTaskHMIFifoRead();
		}
	}

	if (!g_wHTJ6CardType) // Anders 2017-4-13, Add.
	{
		// if (g_dwTickMainISR & 1)
		// DealData(DEVICEHANDLE_MASTER);
		Run_TaskIO();
	}

	// CAN Master.
	if (g_sCommComp.apsCommCompHead[COMMTYPE_CAN1] != NULL) // Anders 2016-10-19, Mark for Rt.
	{
		_eCan_Tx(CAN_A);
		_eCan_Rx(CAN_A);
		//MC_processRxCanMessages();
		Parse_Comm(COMMTYPE_CAN1);
		Run_FastRun(COMMTYPE_CAN1);
		Run_SlowRun(COMMTYPE_CAN1);

		//if (!g_HTJ6CardConfig.all) // Anders 2017-4-13, Add. Anders 2017-8-11, mark.
		//    Run_DriveAccess();
	}
	// shanll 20210331 add for HTRobot as SlaveCan
	// if(g_sCanopenSlaveManager.bSlaveFuncEnable)
	// {
	// 	_eCan_Tx(CAN_A);
	// 	_eCan_Rx(CAN_A);
	// 	Parse_CANopenSlave(COMMTYPE_CAN1);
	// 	Run_CANOpenSlave(COMMTYPE_CAN1);
	// }

	if (!g_wHTJ6CardType) // Anders 2017-4-13, Add.
	{
		// Force Out IO. Anders 2015-8-14, Add.
		if (DB_GetDWORD(DBINDEX_LOCAL_AXISCARD_CONFIGURATION_ID) == 1)
		{
			Set_OutputGroup(1, DB_GetWORD(DBINDEX_LOCAL_AXISCARD_INTERFACE_IO_OUTPUT1));

			Write_DA(1, DB_GetWORD(DBINDEX_LOCAL_AXISCARD_INTERFACE_IO_DA1));
			Write_DA(2, DB_GetWORD(DBINDEX_LOCAL_AXISCARD_INTERFACE_IO_DA2));
			Write_DA(3, DB_GetWORD(DBINDEX_LOCAL_AXISCARD_INTERFACE_IO_DA3));
			Write_DA(4, DB_GetWORD(DBINDEX_LOCAL_AXISCARD_INTERFACE_IO_DA4));
		}

#ifdef TEST_55_28_NET
		if (g_wMasterReady & GetMoldsetState()) // g_wMasterReady ) // GetMoldsetState())  // test
			Set_Output(1);
#endif
	}

	Run_Led(); // Anders 2016-3-3, Add.

	//DealData(DEVICEHANDLE_PC);
	//DB_SetDWORD(DBINDEX_M28_MONITOR_DIAGNOSE_COMM_NET_RX_DATAGRAMS_COUNT, g_dwRxNetDatagramCount[DEVICEHANDLE_MASTER]);
	//DB_SetDWORD(DBINDEX_M28_MONITOR_DIAGNOSE_COMM_NET_TX_DATAGRAMS_COUNT, g_dwTxNetDatagramCount[DEVICEHANDLE_MASTER]);

	//RunTaskNet_PC();
}

/*------------------------------------------------------------------------------+
|           Helpers                                                             |
+------------------------------------------------------------------------------*/
//#pragma CODE_SECTION(memset_dword, "ramfuncs");
//#pragma CODE_SECTION(memset, "ramfuncs");
//
//void * memset_dword(void * mem, register DWORD ch, register size_t length)
//{
//	register DWORD * pMem;
//
//	pMem = mem;
//	while (length-- > 0)
//		*pMem++ = ch;
//	return mem;
//}
//
//void * memset(void * mem, register int ch, register size_t length)
//{
//	register DWORD xMem;
//	DWORD          dwSet;
//
//	if (0 == length)
//		return mem;
//
//	xMem = (DWORD) mem;
//	if (0 != (xMem & 1))
//	{
//		*(WORD *) xMem = ch;
//		++xMem;
//		--length;
//	}
//	dwSet = ((DWORD) ch << 16) | ch;
//	memset_dword((void *) xMem, dwSet, length >> 1);
//	if (0 != (length & 1))
//	{
//		xMem += length - 1;
//		*(WORD *) xMem = ch;
//	}
//	return mem;
//}
//
//#pragma CODE_SECTION(memcpy, "ramfuncs");
//void * memcpy(void * s1, void const * s2, register size_t n)
//{
//	register int *       pMem1;
//	register int const * pMem2;
//
//	pMem1 = s1;
//	pMem2 = s2;
//	while (n-- > 0)
//		*pMem1++ = *pMem2++;
//	return s1;
//}

static void * Memcpy(void * s1, void const * s2, register size_t n)
{
	register int *       pMem1;
	register int const * pMem2;

	pMem1 = s1;
	pMem2 = s2;
	while (n-- > 0)
		*pMem1++ = *pMem2++;
	return s1;
}

void stack_init(DWORD* index)
{
	extern char _stack[];
	extern void _STACK_SIZE;

	DWORD i;

	*index = (DWORD)&_STACK_SIZE;
	for (i = 32; i < *index; ++i) {
		_stack[i] = -1;
	}
}

DWORD stack_scan(DWORD* index)
{
	extern char _stack[];
	extern void _STACK_SIZE;

	if (*index == 0) {
		*index = (DWORD)&_STACK_SIZE;
	}
	if (-1 != _stack[--*index]) {
		DWORD result = *index + 1;
		*index = (DWORD)&_STACK_SIZE;
		return (result << 16) | (DWORD)&_STACK_SIZE;
	}
	return 0;
}

bool debug_assert(bool bAssert)
{
	while (!bAssert)
	{
		asm(" ESTOP0");
	}
	return bAssert;
}


void assert(bool bAssert)
{
	debug_assert(bAssert);
}

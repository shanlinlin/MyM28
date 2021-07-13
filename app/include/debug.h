/*=============================================================================+
|  Function :                                                                  |
|  Task     :                                                                  |
|------------------------------------------------------------------------------|
|  Compile  :                                                                  |
|  Link     :                                                                  |
|  Call     :                                                                  |
|------------------------------------------------------------------------------|
|  Author   : THJ                                                              |
|  Version  : V1.00                                                            |
|  Creation : 2016/01/20                                                       |
|  Revision :                                                                  |
+=============================================================================*/
#ifndef debug__
#define debug__
//------------------------------------------------------------------------------
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//##############################################################################
//
//      Definition
//
//##############################################################################
#ifndef DEBUG
#define DEBUG false
#endif
//------------------------------------------------------------------------------
#if (DEBUG)
#define DEBUG_CODE(def_Code) def_Code
#else
#define DEBUG_CODE(def_Code)
#endif


#define DEBUG_ASSERT_PARAM(def_bAssert, def_pMsg) DEBUG_CODE(do { debug__eassert((bool)(def_bAssert), #def_bAssert def_pMsg, __FILE__, __LINE__); } while (0))
#define DEBUG_ASSERT_ILLEGAL(def_bAssert, def_pMsg) DEBUG_CODE(do { debug__eassert((bool)(def_bAssert), #def_bAssert def_pMsg, __FILE__, __LINE__); } while (0))
#define DEBUG_ASSERT_WARN(def_bAssert, def_pMsg) //DEBUG_CODE(do { debug__eassert((bool)(def_bAssert), #def_bAssert def_pMsg, __FILE__, __LINE__); } while (0))
#define DEBUG_ASSERT(def_bAssert) DEBUG_CODE(do { debug__eassert((bool)(def_bAssert), #def_bAssert, __FILE__, __LINE__); } while (0))
//------------------------------------------------------------------------------
#define DEBUG_INIT_CHECK(def_Struct, def_Init)                             \
	DEBUG_CODE(do {                                                        \
		def_Struct sValue0;                                                \
		def_Struct sValue1;                                                \
		def_Init(&sValue1);                                                \
		memcpy(&sValue0, &sValue1, sizeof(def_Struct));                    \
		memset(&sValue1, 0, sizeof(def_Struct));                           \
		def_Init(&sValue1);                                                \
		DEBUG_ASSERT(0 == memcmp(&sValue0, &sValue1, sizeof(def_Struct))); \
	} while (0))

#define DEBUG_LEVEL_PARAM true // 参数级别：必要的参数.
#define DEBUG_LEVEL_ERROR true // 错误级别：写入错误的数据、写入数据到错误的地址、执行非法函数.
#define DEBUG_LEVEL_ALARM true // 警报级别：出现错误的数据、读取错误地址的数据、正常不会出现的情况.
#define DEBUG_LEVEL_TIPS true  // 提示级别：未实现的功能、已做好保护的.

// #define DEBUG_ASSERT(assert_) DEBUG_CODE(debug_assert(assert_))

#define EASSERT(level_, assert_, print_)          \
    DEBUG_CODE(do {                               \
        if (DEBUG_LEVEL_##level_ && !(assert_)) { \
            debug_assert(false);                  \
        }                                         \
    } while (0))


#ifndef DEBUG_LOG
#define DEBUG_LOG(type_, print_)                   \
    DEBUG_CODE(do {                                \
        if ((DEBUG_LEVEL_##type_) < DEBUG_LEVEL) { \
            printf("%s: %u %s, ", #type_,          \
                __LINE__, __FUNCTION__);           \
            printf print_;                         \
            putchar('\n');                         \
        }                                          \
    } while (0))
#endif
//##############################################################################
//
//      Enumeration
//
//##############################################################################
//##############################################################################
//
//      Type Definition
//
//##############################################################################
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
bool debug_assert(bool bAssert);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [debug_assert description]
 * @param  bAssert [description]
 * @return         [description]
 */
//inline bool debug_assert(bool bAssert)
//{
//	while (!bAssert)
//	{
//		asm(" ESTOP0");
//	}
//	return bAssert;
//}

/**
 * [debug__eassert description]
 * @param  bAssert [description]
 * @param  pMsg    [description]
 * @param  pFile   [description]
 * @param  iLine   [description]
 * @return         [description]
 */
//inline bool debug__eassert(bool bAssert, char const* pMsg, char const* pFile, unsigned iLine)
//{
//	//if (!bAssert)
//	//{
//	//	printf("%s, %u: [%s]\n", pFile, iLine, pMsg);
//	//}
//	return debug_assert(bAssert);
//}
//------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif

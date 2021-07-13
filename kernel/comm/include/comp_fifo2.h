#ifndef comp_fifo2__
#define comp_fifo2__

#include "debug.h"

typedef struct comp_fifo_s_manager {
    unsigned          _size;
    volatile unsigned _read;
    volatile unsigned _write;
    bool              _full;
} comp_fifo_s_manager;

typedef union tagFifoHMIMasterType {
	WORD all;
	struct
	{
		WORD bVersion : 8;
		WORD bCounter : 8;
	} bit;
} FifoHMIMasterType;

typedef struct FifoHMIHeader
{
	WORD        		wSTX;
	WORD        		wSize; //Package Size.
	FifoHMIMasterType 	Type;
	WORD        		wCMD;
	WORD        		wID;
	WORD        		wDataLen;
} FifoHMIHeader;

typedef struct HMI_tail{
    WORD   crc;
    WORD   ext;
} HMI_tail;

typedef struct fifo_HMI_buffer {
    WORD            fifo_Length;
    FifoHMIHeader   fifo_header;
    WORD            fifo_data[256];
    HMI_tail        fifo_tail;
} fifo_HMI_buffer;

typedef struct Fifo{
    comp_fifo_s_manager manager;
    fifo_HMI_buffer     fifo_buffer;
} Fifo;

static inline void     comp_fifo_s_manager_init(comp_fifo_s_manager* manager);
static inline void     comp_fifo_s_manager_init_size(comp_fifo_s_manager* manager, unsigned size);
static inline unsigned comp_fifo_s_manager_get_free(comp_fifo_s_manager* manager);
static inline unsigned comp_fifo_s_manager_get_exist(comp_fifo_s_manager* manager);
static inline unsigned comp_fifo_s_manager_get_read(comp_fifo_s_manager* manager);
static inline unsigned comp_fifo_s_manager_get_write(comp_fifo_s_manager* manager);
static inline void     comp_fifo_s_manager_set_read(comp_fifo_s_manager* manager, unsigned read);
static inline void     comp_fifo_s_manager_set_read_batch(comp_fifo_s_manager* manager, unsigned read, unsigned itemnum);
static inline void     comp_fifo_s_manager_set_write(comp_fifo_s_manager* manager, unsigned write);
static inline void     comp_fifo_s_manager_set_write_batch(comp_fifo_s_manager* manager, unsigned write, unsigned itemnum);

void comp_fifo_s_manager_init(comp_fifo_s_manager* manager)
{
    EASSERT(PARAM, NULL != manager, ("null pointer."));
    manager->_size  = 0;
    manager->_read  = 0;
    manager->_write = 0;
    manager->_full  = false;
}

void comp_fifo_s_manager_init_size(comp_fifo_s_manager* manager, unsigned size)
{
    EASSERT(PARAM, NULL != manager, ("null pointer."));
    manager->_size  = size;
    manager->_read  = 0;
    manager->_write = 0;
    manager->_full  = false;
}

unsigned comp_fifo_s_manager_get_free(comp_fifo_s_manager* manager)
{
    EASSERT(PARAM, NULL != manager, ("null pointer."));
    if(manager->_full == true)
        return 0;
    else if(manager->_read == manager->_write) {
        return manager->_size;
    }else{
        return (manager->_size - manager->_write + manager->_read) % manager->_size;
    }
}

unsigned comp_fifo_s_manager_get_exist(comp_fifo_s_manager* manager)
{
    EASSERT(PARAM, NULL != manager, ("null pointer."));
    if(manager->_full == true)
        return  manager->_size;
    else
        return manager->_size - comp_fifo_s_manager_get_free(manager);
}

unsigned comp_fifo_s_manager_get_read(comp_fifo_s_manager* manager)
{
    EASSERT(PARAM, NULL != manager, ("null pointer."));
    EASSERT(ERROR, comp_fifo_s_manager_get_exist(manager) > 0, ("null pointer."));
    return manager->_read;
}

unsigned comp_fifo_s_manager_get_write(comp_fifo_s_manager* manager)
{
    EASSERT(PARAM, NULL != manager, ("null pointer."));
    EASSERT(ERROR, comp_fifo_s_manager_get_free(manager) > 0, ("null pointer."));
    return manager->_write;
}

void comp_fifo_s_manager_set_read(comp_fifo_s_manager* manager, unsigned read)
{
    EASSERT(PARAM, NULL != manager, ("null pointer."));
    EASSERT(ERROR, read == manager->_read, ("null pointer."));
    EASSERT(ERROR, comp_fifo_s_manager_get_exist(manager) > 0, ("null pointer."));
    manager->_read = (++read < manager->_size) ? read : 0;
    manager->_full = false;
}

void comp_fifo_s_manager_set_read_batch(comp_fifo_s_manager* manager, unsigned read, unsigned itemnum)
{
    EASSERT(PARAM, NULL != manager, ("null pointer."));
    EASSERT(ERROR, read == manager->_read, ("null pointer."));
    EASSERT(ERROR, comp_fifo_s_manager_get_exist(manager) > 0, ("null pointer."));
    if(comp_fifo_s_manager_get_exist(manager) < itemnum)
        return;
    manager->_read = (read+itemnum < manager->_size) ? read+itemnum : 0;
    manager->_full = false;
}
void comp_fifo_s_manager_set_write(comp_fifo_s_manager* manager, unsigned write)
{
    EASSERT(PARAM, NULL != manager, ("null pointer."));
    EASSERT(ERROR, write == manager->_write, ("null pointer."));
    EASSERT(ERROR, comp_fifo_s_manager_get_free(manager) > 0, ("null pointer."));
    manager->_write = (++write < manager->_size) ? write : 0;
    manager->_full  = (manager->_write == manager->_read) ? true : false;
}

void comp_fifo_s_manager_set_write_batch(comp_fifo_s_manager* manager, unsigned write, unsigned itemnum)
{
    EASSERT(PARAM, NULL != manager, ("null pointer."));
    EASSERT(ERROR, write == manager->_write, ("null pointer."));
    EASSERT(ERROR, comp_fifo_s_manager_get_free(manager) > 0, ("null pointer."));
    if(comp_fifo_s_manager_get_free(manager) < itemnum)
        return;
    manager->_write = (write+itemnum < manager->_size) ? write+itemnum : 0;
    manager->_full  = (manager->_write == manager->_read) ? true : false;
}

#endif

#include "yc11xx_qspi.h"
#include "hal_flash.h"

#define QSPI_BUSYTIMEOUT (0x1000000)
#define DMA_WAIT_TIMEOUT (0x10000)

void qspi_get_lock(void)
{
    HWRITE_INLINE(mem_qspi_lock_flag_cm0, 1);// I'm interested
    HWRITE_INLINE(mem_qspi_lock_victim, QSPI_LOCK_VICTIM_CM0);// you go first
    
    while(((HREAD_INLINE(mem_qspi_lock_flag_respin)) != 0) 
        && ((HREAD_INLINE(mem_qspi_lock_victim)) == QSPI_LOCK_VICTIM_CM0));// wait
}
void qspi_free_lock(void)
{
    HWRITE_INLINE(mem_qspi_lock_flag_cm0, 0);
}


void __attribute__((noinline))   SetLockQSPI( )
{
		qspi_get_lock();
		HWRITE_INLINE(mem_m0_lock_qspi, 1);
}

void __attribute__((noinline))   SetReleaseQSPI( )
{
		qspi_free_lock();
		HWRITE_INLINE(mem_m0_lock_qspi, 0);	
}

void __attribute__((noinline)) prefetch(void *start_addr, void *end_addr)
{
 int addr;
 for(addr = (int)start_addr & ~0x1f;addr < (int)end_addr + 32;addr += 32)
  	PREFETCH_LINE(addr);
}

void qspi_dma_start(void *src,int srclen,void *dst,int dstlen)
{
		HWRITEW_INLINE(CORE_QSPI_TXADDR , (int)src);
		HWRITEW_INLINE(CORE_QSPI_TXLEN , srclen);
		HWRITEW_INLINE(CORE_QSPI_RXADDR , (int)dst);
		HWRITEW_INLINE(CORE_QSPI_RXLEN , dstlen);
		HWRITE_INLINE(CORE_DMA_START , 8);
}

uint8_t qspi_dma_wait(void)
{	
		int timeout = 0;
		while(!(HREAD_INLINE(CORE_DMA_STATUS) & 8))
		{
			timeout++;
			if(timeout>DMA_WAIT_TIMEOUT)
			{
				return ERROR;
			}
		}
		return SUCCESS;
} 

/*
 * @brief: qspi_write       (spi mode)
 * @param: tbuf data buffer
 * @param: len  data len
 * @return: ERROR ,SUCCESS
 */
uint8_t qspi_write(uint8_t *tbuf,uint32_t len)
{
		uint8_t ctrl = HREAD_INLINE(CORE_QSPI_CTRL);
		uint8_t delay = HREAD_INLINE(CORE_QSPI_DELAY);
		uint8_t ret = 0;
		HWRITE_INLINE(CORE_QSPI_CTRL, 0x44);
		HWRITE_INLINE(CORE_QSPI_DELAY,0x80);
    	qspi_dma_start(tbuf, len, 0, 0);
		ret = qspi_dma_wait();
		HWRITE_INLINE(CORE_QSPI_CTRL, ctrl);
		HWRITE_INLINE(CORE_QSPI_DELAY, delay);
		return ret;
}


/*
 * @brief:qspi_read_write  (spi mode)
 * @param:tbuf
 * @param:tlen
 * @param:rbuf
 * @param:rlen
 * @return: none
 */
uint8_t  qspi_read_write(uint8_t *tbuf,uint32_t tlen,uint8_t *rbuf,uint32_t rlen)
{
		uint8_t ctrl = HREAD_INLINE(CORE_QSPI_CTRL);
		uint8_t delay = HREAD_INLINE(CORE_QSPI_DELAY);
		uint8_t ret = 0;
		HWRITE_INLINE(CORE_QSPI_CTRL, 0x44);
		HWRITE_INLINE(CORE_QSPI_DELAY,0x80);
    	qspi_dma_start(tbuf, tlen, rbuf, rlen);
		ret = qspi_dma_wait();
		HWRITE_INLINE(CORE_QSPI_CTRL, ctrl);
		HWRITE_INLINE(CORE_QSPI_DELAY,delay);
		return ret;
}


/*
 * @brief: write flash cmd
 * @param: cmd  
 * @return: ERROR ,SUCCESS
 */
uint8_t  qspi_flash_cmd(uint8_t cmd)
{
    uint8_t tbuf[1];
	tbuf[0] = cmd;
    return qspi_write(tbuf, 1);
}

/*
 * @brief:read flash status1
 * @param:cmd
 * @return: ERROR ,SUCCESS
 */
uint8_t  qspi_flash_readstatus1(uint8_t *rbuf)
{
    uint8_t tbuf[1];
    tbuf[0] = W25X_READ_STATUS1;
    return qspi_read_write(tbuf, 1, rbuf, 1);
}

/*
 * @brief:read flash status2
 * @param:cmd
 * @return: ERROR ,SUCCESS
 */
uint8_t  qspi_flash_readstatus2(uint8_t *rbuf)
{
    uint8_t tbuf[1];
    tbuf[0] = W25X_READ_STATUS2;
    return qspi_read_write(tbuf, 1, rbuf, 1);
}


uint8_t  qspi_flash_waitfinish(void)
{
    uint32_t timeout = 0;
    uint8_t status1=1;
 
    while(status1 & 1)
    {
        qspi_flash_readstatus1(&status1);
        if(++timeout > QSPI_BUSYTIMEOUT)
        {
            return ERROR;
        }
    }
    return SUCCESS;
}

/*
 * @brief:write flash reg  reg[0] s7~0     reg[1] s15~0
 * @param:rxbuf
 * @return: ERROR ,SUCCESS
 */
uint8_t  _qspi_flash_writestatus(uint8_t *reg)
{
    uint8_t tbuf[3];
    tbuf[0] = W25X_WRITE_STATUS;
    tbuf[1] = reg[0];
    tbuf[2] = reg[1];
		
    if(ERROR == qspi_flash_cmd(W25X_WRITE_ENABLE))
        return ERROR;

    qspi_write(tbuf, 3);
		
	return qspi_flash_waitfinish();
}


/*
 * @brief:enable flash quad mode
 * @param:on 0:off  1:on
 * @return: ERROR ,SUCCESS
 */
 #if 0
uint8_t  qspi_flash_quadenable(uint8_t on)
{
    uint8_t reg[2] = {0};

    if(ERROR == qspi_flash_readstatus1(&reg[0]))
        return ERROR;
    
    if(ERROR == qspi_flash_readstatus2(&reg[1]))
        return ERROR;

    if(on)
        reg[1] |= 0x02; 
    else
        reg[1] &= 0xfd;

    return qspi_flash_writestatus(reg);
}
#endif
/*
 * @brief:flash write enable
 * @param:none
 * @return: ERROR ,SUCCESS
 */
uint8_t  qspi_flash_writeenable(void)
{
    return qspi_flash_cmd(W25X_WRITE_ENABLE);
}


/*
 * @brief:flash write disable
 * @param:none
 * @return: ERROR ,SUCCESS
 */
uint8_t  qspi_flash_writedisable(void)
{
    return qspi_flash_cmd(W25X_WRITE_DISABLE);
}

void  qspi_flash_precmd(uint8_t *buf,uint8_t cmd,uint32_t addr)
{
    buf[0] = W25X_PAGE_PROGRAM;
    buf[1] = addr >> 16;
    buf[2] = addr >> 8;
    buf[3] = addr;
}



/*
 * @brief: flash write  
 * @param: flash_addr
 * @param: len
 * @param: txbuf
 * @return: ERROR ,SUCCESS
 */
#define BLOCK_UNIT  (256)
uint8_t  _qspi_flash_write(uint32_t flash_addr,uint32_t len,uint8_t *tbuf)
{
    uint8_t buf[BLOCK_UNIT+4] = {0};
    uint32_t packnum = (len+BLOCK_UNIT-1)/BLOCK_UNIT;
    uint32_t packlen = 0;

    for(uint32_t i=0; i<packnum;i++)
    {   
        packlen = len > BLOCK_UNIT?BLOCK_UNIT:len;

        if(!flash_access_is_enabled()) { return ERROR; }

        qspi_flash_precmd(buf,W25X_PAGE_PROGRAM,flash_addr+i*BLOCK_UNIT);
        
        for(uint32_t j = 0;j<packlen;j++)
        {
            buf[4+j] =  tbuf[j+i*BLOCK_UNIT];
        }
        if(ERROR == qspi_flash_writeenable())
                return ERROR;
				
        if(ERROR == qspi_write(buf,packlen+4))
            return ERROR;
        
        if(ERROR == qspi_flash_waitfinish())
            return ERROR;

        len -= packlen;
    }

    return SUCCESS;
}


/*
 * @brief:chip erase
 * @param:none
 * @return: ERROR ,SUCCESS
 */
 #if 0
uint8_t  _qspi_flash_chiperase(void)
{
	if(ERROR == qspi_flash_writeenable())
        return ERROR;
		
    if(ERROR ==  qspi_flash_cmd(W25X_CHIP_ERASE))
        return ERROR;

    return qspi_flash_waitfinish();
}
#endif

/*
 * @brief:qspi_flash_erase
 * @param:cmd
 *          W25X_SECTOR_ERASE    4K
 *          W25X_BLOCK_ERASE32K 32k
 *          W25X_BLOCK_ERASE64K 64k
 * @param:flash_addr
 * @return: none
 */
uint8_t  _qspi_flash_erase(uint8_t cmd,uint32_t flash_addr)
{
    uint8_t tbuf[4] = {0};
    if(ERROR == qspi_flash_writeenable())
        return ERROR;

    if(!flash_access_is_enabled()) { return ERROR; }

    tbuf[0] = cmd;
    tbuf[1] = flash_addr >> 16;
    tbuf[2] = flash_addr >> 8;
    tbuf[3] = flash_addr;

    if(ERROR == qspi_write(tbuf,4))
        return ERROR;

    return qspi_flash_waitfinish();
}

/*
 * @brief: flash read
 * @param: mode
 *          W25X_FASTREAD_DUAL1
 *          W25X_FASTREAD_DUAL2
 *          W25X_FASTREAD_QUAD1
 *          W25X_FASTREAD_QUAD2
 *          W25X_FASTREAD_QUAD3
 * @param: flash_addr
 * @param: len
 * @param: rxbuf
 * @return: ERROR ,SUCCESS
 */
uint8_t  _qspi_flash_read(uint8_t mode,uint32_t flash_addr,uint32_t rlen,uint8_t *rbuf)
{
    uint8_t ret = 0;
    uint8_t tbuf[5] = {0};
    uint32_t tlen = 5;
	uint8_t ctrl =  HREAD_INLINE(CORE_QSPI_CTRL);
	uint8_t delay = HREAD_INLINE(CORE_QSPI_DELAY);

      tbuf[0] = mode;
	  tbuf[1] = flash_addr >> 16;
	  tbuf[2] = flash_addr >> 8;
	  tbuf[3] = flash_addr;
	  tbuf[4] = 0;

    switch(mode) {
		case W25X_FASTREAD_DUAL1 : 
			tlen = 4;
			HWRITEW_INLINE(CORE_QSPI_CTRL,QSPICFG_MODE_3B);
			break;
		case W25X_FASTREAD_DUAL2 : 
			HWRITEW_INLINE(CORE_QSPI_CTRL,QSPICFG_MODE_BB);
			break;
		case W25X_FASTREAD_QUAD1 : 
			tlen = 4;
			HWRITEW_INLINE(CORE_QSPI_CTRL,QSPICFG_MODE_6B);
			break;
		case W25X_FASTREAD_QUAD2 : 
			HWRITEW_INLINE(CORE_QSPI_CTRL,QSPICFG_MODE_EB);
			break;
		case W25X_FASTREAD_QUAD3 :
			HWRITEW_INLINE(CORE_QSPI_CTRL,QSPICFG_MODE_E7); 
			break;
		default :
            tlen = 4; 
			HWRITEW_INLINE(CORE_QSPI_CTRL,0x44);
			break;
	}
    qspi_dma_start(tbuf, tlen, rbuf, rlen);
	ret = qspi_dma_wait();
	HWRITE_INLINE(CORE_QSPI_CTRL, ctrl);
	HWRITE_INLINE(CORE_QSPI_DELAY, delay);
    return ret;
}

/*
 * @brief:read flash id
 * @param:cmd
 * @return: ERROR ,SUCCESS
 */
uint32_t  _qspi_flash_JEDECID(void)
{
    uint8_t tbuf[1];
	uint8_t rbuf[3];
	uint32_t ret;
    tbuf[0] = W25X_JEDEC_DEVICEID;
    qspi_read_write(tbuf, 1, rbuf, 3);
	ret = (rbuf[0]<<16) | (rbuf[1]<<8) | (rbuf[2]<<0);
	return ret;
}
/*
 * @brief:sector erase 4kB
 * @param:flash addr
 * @return: ERROR ,SUCCESS
 */
uint8_t  qspi_flash_sectorerase(uint32_t flash_addr)
{
	if(!flash_access_is_enabled()) { return ERROR; }
	prefetch(qspi_dma_start, qspi_flash_sectorerase);
    return _qspi_flash_erase(W25X_SECTOR_ERASE,flash_addr);
}

/*
 * @brief:block erase  32K
 * @param:flash addr
 * @return: ERROR ,SUCCESS
 */
 #if 0
uint8_t  qspi_flash_blockerase32k(uint32_t flash_addr)
{
	prefetch(qspi_dma_start, qspi_flash_blockerase32k);
    return _qspi_flash_erase(W25X_BLOCK_ERASE32K,flash_addr);
}
#endif
/*
 * @brief:block erase  64K
 * @param:flash addr
 * @return: ERROR ,SUCCESS
 */
 #if 0
uint8_t  qspi_flash_blockerase64k(uint32_t flash_addr)
{
	prefetch(qspi_dma_start, qspi_flash_blockerase64k);
    return _qspi_flash_erase(W25X_BLOCK_ERASE64K,flash_addr);
}

uint8_t  qspi_flash_chiperase(void)
{
	prefetch(qspi_dma_start, qspi_flash_chiperase);
	return _qspi_flash_chiperase();
}
#endif

uint8_t  qspi_flash_write(uint32_t flash_addr,uint32_t len,uint8_t *tbuf)
{
	if(!flash_access_is_enabled()) { return ERROR; }
	prefetch(qspi_dma_start, qspi_flash_write);
	return _qspi_flash_write(flash_addr,len,tbuf);
}


/*
 * @brief: flash read
 * @param: mode
 *          W25X_FASTREAD_DUAL1
 *          W25X_FASTREAD_DUAL2
 *          W25X_FASTREAD_QUAD1
 *          W25X_FASTREAD_QUAD2
 *          W25X_FASTREAD_QUAD3
 * @param: flash_addr
 * @param: len
 * @param: rxbuf
 * @return: ERROR ,SUCCESS
 */
uint8_t  qspi_flash_read(uint8_t mode,uint32_t flash_addr,uint32_t rlen,uint8_t *rbuf)
{
	prefetch(qspi_dma_start, qspi_flash_read);
	return _qspi_flash_read(mode,flash_addr,rlen,rbuf);
}

uint8_t qspi_flash_writestatus(uint8_t *reg)
{
	prefetch(qspi_dma_start, qspi_flash_writestatus);
    return _qspi_flash_writestatus (reg);	
}
/*
 * @brief:read flash id
 * @param:cmd
 * @return: ERROR ,SUCCESS
 */
uint32_t  qspi_flash_JEDECID(void)
{
    prefetch(qspi_dma_start, qspi_flash_JEDECID);
		return _qspi_flash_JEDECID();
}

#if 0
uint8_t  qspi_flash_blockerasepage(uint32_t flash_addr)
{
	prefetch(qspi_dma_start, qspi_flash_blockerasepage);
       return _qspi_flash_erase(FlashCMD_PageErase,flash_addr);
}
#endif


#define FLASH_TRANMIT_BUFFER (0x100)
// We don't suggest use this to work for big memory copy to xram
uint8_t  QSPI_ReadFlashDataToXRAM(uint32_t flash_addr,uint32_t rlen,uint8_t *rbuf)
{
	uint8_t ret, i, buckCnt;
	uint8_t tempBuffer[FLASH_TRANMIT_BUFFER];
	uint32_t reserveCnt;
	buckCnt = rlen / FLASH_TRANMIT_BUFFER;
	reserveCnt = rlen - (buckCnt * FLASH_TRANMIT_BUFFER);
	OS_ENTER_CRITICAL();
	SetLockQSPI( );
	for(i = 0; i < buckCnt; i ++)
	{
		ret = qspi_flash_read(W25X_FASTREAD_DUAL1, flash_addr + (i * FLASH_TRANMIT_BUFFER), FLASH_TRANMIT_BUFFER, tempBuffer);
		xmemcpy(rbuf + (i * FLASH_TRANMIT_BUFFER), tempBuffer, FLASH_TRANMIT_BUFFER);
	}
	if(reserveCnt != 0)
	{
		ret = qspi_flash_read(W25X_FASTREAD_DUAL1, flash_addr + (i * FLASH_TRANMIT_BUFFER), reserveCnt, tempBuffer);
		xmemcpy(rbuf + (i * FLASH_TRANMIT_BUFFER), tempBuffer, reserveCnt);
	}
	SetReleaseQSPI( );
	OS_EXIT_CRITICAL();

	return ret;
}
uint8_t  QSPI_ReadFlashData(uint32_t flash_addr,uint32_t rlen,uint8_t *rbuf)
{
	uint8_t ret;
	OS_ENTER_CRITICAL();
	SetLockQSPI( );
	ret = qspi_flash_read(W25X_FASTREAD_DUAL1, flash_addr, rlen, rbuf);
	SetReleaseQSPI( );
	OS_EXIT_CRITICAL();

	return ret;
}
uint8_t  QSPI_WriteFlashData(uint32_t flash_addr,uint32_t len,uint8_t *tbuf)
{
	uint8_t ret;

	if(!flash_access_is_enabled()) { return ERROR; }

	OS_ENTER_CRITICAL();
	SetLockQSPI( );
	ret = qspi_flash_write(flash_addr, len, tbuf);
	SetReleaseQSPI( );
	OS_EXIT_CRITICAL();

	return ret;
}
uint8_t  QSPI_SectorEraseFlash(uint32_t flash_addr)
{
	uint8_t ret;

	if(!flash_access_is_enabled()) { return ERROR; }

	OS_ENTER_CRITICAL();
	SetLockQSPI( );
	ret = qspi_flash_sectorerase(flash_addr);
	SetReleaseQSPI( );
	OS_EXIT_CRITICAL();

	return ret;
}

#if 0
uint8_t QSPI_EraseFlashPage(uint32_t flash_addr)
{
	uint8_t ret;
	OS_ENTER_CRITICAL();
	SetLockQSPI( );
	ret=qspi_flash_blockerasepage(flash_addr);
	SetReleaseQSPI( );
	OS_EXIT_CRITICAL();

	return ret;
}
#endif

uint8_t QSPI_WriteFlashStatus(uint8_t *reg)
{
	reg[0] &= 0x7f;
	reg[1] &= 0xfe;
	uint8_t ret;
	OS_ENTER_CRITICAL();
	SetLockQSPI( );
	ret=qspi_flash_writestatus(reg);
	SetReleaseQSPI( );
	OS_EXIT_CRITICAL();

	return ret;
}

uint8_t QSPI_WriteFlashStatus_AllLocked(void)
{
	uint8_t tbuf[2];
	tbuf[0] = 0x28;
	tbuf[1] = 0;
	if(QSPI_WriteFlashStatus(tbuf))
		return 1;
	else
		return 0;
}

uint8_t QSPI_WriteFlashStatus_AllUnLocked(void)
{
	uint8_t tbuf[2];
	tbuf[0] = 0;
	tbuf[1] = 0;
	if(QSPI_WriteFlashStatus(tbuf))
		return 1;
	else
		return 0;
}



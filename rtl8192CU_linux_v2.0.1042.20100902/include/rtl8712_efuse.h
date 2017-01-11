#ifndef __RTL8712_EFUSE_H__
#define __RTL8712_EFUSE_H__

#include <drv_conf.h>
#include <osdep_service.h>


#define _REPEAT_THRESHOLD_	3

#define	EFUSE_MAX_PGPKT_SIZE   	9 //header+ 2* 4 words (BYTES)
#define	EFUSE_PGPKT_DATA_SIZE 	8 //BYTES sizeof(u8)*8
#define	EFUSE_MAX_PGPKT_OFFSET	16
		
#define	EFUSE_PGPKG_MAX_WORDS 	4
#define	EFUSE_REPROG_THRESHOLD	3

#define	EFUSE_PG_STATE_HEADER	0x01
#define	EFUSE_PG_STATE_DATA		0x20
#define	EFUSE_MAX_PHYSICAL_SIZE	512
#define	EFUSE_MAX_LOGICAL_SIZE	128

#define GET_EFUSE_OFFSET(header)	((header & 0xF0) >> 4)
#define GET_EFUSE_WORD_EN(header)	(header & 0x0F)
#define MAKE_EFUSE_HEADER(offset, word_en)	(((offset & 0x0F) << 4) | (word_en & 0x0F))
//------------------------------------------------------------------------------
typedef struct PG_PKT_STRUCT{
	u8 offset;
	u8 word_en;
	u8 data[EFUSE_PGPKT_DATA_SIZE];
} PGPKT_STRUCT,*PPGPKT_STRUCT;
//------------------------------------------------------------------------------
extern u8 	rtw_efuse_reg_init(_adapter *padapter);
extern void 	rtw_efuse_reg_uninit(_adapter *padapter);
extern u16 	rtw_efuse_get_current_phy_size(_adapter *padapter);
extern int 	rtw_efuse_get_max_phy_size(_adapter *padapter);

extern u8 	rtw_efuse_pg_packet_read(_adapter *padapter, u8 offset, u8 *data);
extern u8 	rtw_efuse_pg_packet_write(_adapter *padapter, const u8 offset, const u8 word_en, const u8 *data);
extern u8 	rtw_efuse_access(_adapter *padapter, u8 bRead, u16 start_addr, u16 cnts, u8 *data);
extern u8	rtw_efuse_map_read(_adapter *padapter, u16 addr, u16 cnts, u8 *data);
extern u8	rtw_efuse_map_write(_adapter *padapter, u16 addr, u16 cnts, u8 *data);

u8 efuse_one_byte_read(_adapter *padapter, u16 addr, u8 *data);

#endif

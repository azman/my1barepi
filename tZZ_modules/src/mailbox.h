/*----------------------------------------------------------------------------*/
#ifndef __MY1MAILBOX_H__
#define __MY1MAILBOX_H__
/*----------------------------------------------------------------------------*/
#include "raspi.h"
#include "boot.h"
/*----------------------------------------------------------------------------*/
#define MAIL_CHANNEL_MASK 0x0000000F
#define MAIL_CH_POWER 0x00000000
#define MAIL_CH_FBUFF 0x00000001
#define MAIL_CH_VUART 0x00000002
#define MAIL_CH_VCHIQ 0x00000003
#define MAIL_CH_LEDS  0x00000004
#define MAIL_CH_BUTTS 0x00000005
#define MAIL_CH_TOUCH 0x00000006
#define MAIL_CH_NOUSE 0x00000007
#define MAIL_CH_TAGAV 0x00000008
#define MAIL_CH_TAGVA 0x00000009
/*----------------------------------------------------------------------------*/
/**
0: Power management
1: Framebuffer
2: Virtual UART
3: VCHIQ
4: LEDs
5: Buttons
6: Touch screen
7: <NOT USED?>
8: Property tags (ARM -> VC)
9: Property tags (VC -> ARM)
**/
/*----------------------------------------------------------------------------*/
unsigned int mailbox_read(unsigned int channel);
void mailbox_write(unsigned int channel,unsigned int value);
/*----------------------------------------------------------------------------*/
#define TAGS_STATUS_REQUEST 0x00000000
#define TAGS_STATUS_SUCCESS 0x80000000
#define TAGS_STATUS_FAILURE 0x80000001
/*----------------------------------------------------------------------------*/
#define TAGS_END 				0x00000000
#define TAGS_MASK_SET			0x00008000
#define TAGS_REQUESTS			0x00000000
#define TAGS_RESPONSE			0x80000000
/*----------------------------------------------------------------------------*/
/** number of 32-bit buffer allocated for tags response */
#define TAGS_RESPONSE_SIZE		2
/*----------------------------------------------------------------------------*/
/** VideoCore */
#define TAGS_FIRMWARE_REVISION	0x00000001
/*----------------------------------------------------------------------------*/
#define TAGS_SET_CURSOR_INFO	(0x00000010|TAGS_MASK_SET)
#define TAGS_SET_CURSOR_STATE	(0x00000011|TAGS_MASK_SET)
/*----------------------------------------------------------------------------*/
/** Hardware */
#define TAGS_BOARD_MODEL		0x00010001
#define TAGS_BOARD_REVISION		0x00010002
#define TAGS_BOARD_MAC_ADDR		0x00010003
#define TAGS_BOARD_SERIAL		0x00010004
#define TAGS_ARM_MEMORY			0x00010005
#define TAGS_VC_MEMORY			0x00010006
#define TAGS_CLOCKS				0x00010007
/*----------------------------------------------------------------------------*/
#define TAGS_POWER_STATE		0x00020001
#define TAGS_TIMING				0x00020002
/*----------------------------------------------------------------------------*/
#define TAGS_SET_POWER_STATE	(TAGS_POWER_STATE|TAGS_MASK_SET)
/*----------------------------------------------------------------------------*/
#define TAGS_CLOCK_STATE		0x00030001
#define TAGS_CLOCK_RATE			0x00030002
#define TAGS_VOLTAGE			0x00030003
#define TAGS_MAX_CLOCK_RATE		0x00030004
#define TAGS_MAX_VOLTAGE		0x00030005
#define TAGS_TEMPERATURE		0x00030006
#define TAGS_MIN_CLOCK_RATE		0x00030007
#define TAGS_MIN_VOLTAGE		0x00030008
#define TAGS_TURBO				0x00030009
#define TAGS_MAX_TEMPERATURE	0x0003000a
#define TAGS_STC				0x0003000b
#define TAGS_ALLOCATE_MEMORY	0x0003000c
#define TAGS_LOCK_MEMORY		0x0003000d
#define TAGS_UNLOCK_MEMORY		0x0003000e
#define TAGS_RELEASE_MEMORY		0x0003000f
#define TAGS_EXECUTE_CODE		0x00030010
#define TAGS_EXECUTE_QPU		0x00030011
#define TAGS_ENABLE_QPU			0x00030012
#define TAGS_X_RES_MEM_HANDLE	0x00030014
#define TAGS_EDID_BLOCK			0x00030020
#define TAGS_CUSTOMER_OTP		0x00030021
#define TAGS_DOMAIN_STATE		0x00030030
#define TAGS_GPIO_STATE			0x00030041
#define TAGS_GPIO_CONFIG		0x00030043
/*----------------------------------------------------------------------------*/
#define TAGS_SET_CLOCK_STATE	(TAGS_CLOCK_STATE|TAGS_MASK_SET)
#define TAGS_SET_CLOCK_RATE		(TAGS_CLOCK_RATE|TAGS_MASK_SET)
#define TAGS_SET_VOLTAGE		(TAGS_VOLTAGE|TAGS_MASK_SET)
#define TAGS_SET_TURBO			(TAGS_TURBO|TAGS_MASK_SET)
#define TAGS_SET_CUSTOMER_OTP	(TAGS_CUSTOMER_OTP|TAGS_MASK_SET)
#define TAGS_SET_DOMAIN_STATE	(TAGS_DOMAIN_STATE|TAGS_MASK_SET)
#define TAGS_SET_GPIO_STATE		(TAGS_GPIO_STATE|TAGS_MASK_SET)
#define TAGS_SET_SDHOST_CLOCK	(0x00038042|TAGS_MASK_SET)
#define TAGS_SET_GPIO_CONFIG	(TAGS_GPIO_CONFIG|TAGS_MASK_SET)
/*----------------------------------------------------------------------------*/
#define TAGS_FB_ALLOCATE		0x00040001
#define TAGS_FB_BLANK			0x00040002
#define TAGS_FB_PHYS_DIMS		0x00040003
#define TAGS_FB_VIRT_DIMS		0x00040004
#define TAGS_FB_DEPTH			0x00040005
#define TAGS_FB_PIXEL_ORDER		0x00040006
#define TAGS_FB_ALPHA_MODE		0x00040007
#define TAGS_FB_PITCH			0x00040008
#define TAGS_FB_VIROFFSET		0x00040009
#define TAGS_FB_OVERSCAN		0x0004000a
#define TAGS_FB_PALETTE			0x0004000b
#define TAGS_FB_TOUCHBUF		0x0004000f
#define TAGS_FB_GPIOVIRTBUF		0x00040010
/*----------------------------------------------------------------------------*/
#define TAGS_FBT_PHYS_DIMS		0x00044003
#define TAGS_FBT_VIRT_DIMS		0x00044004
#define TAGS_FBT_DEPTH			0x00044005
#define TAGS_FBT_PIXEL_ORDER	0x00044006
#define TAGS_FBT_ALPHA_MODE		0x00044007
#define TAGS_FBT_VIROFFSET		0x00044009
#define TAGS_FBT_OVERSCAN		0x0004400a
#define TAGS_FBT_PALETTE		0x0004400b
#define TAGS_FBT_VSYNC			0x0004400e
/*----------------------------------------------------------------------------*/
#define TAGS_FB_RELEASE			(TAGS_FB_ALLOCATE|TAGS_MASK_SET)
#define TAGS_FB_SET_PHYS_DIMS	(TAGS_FB_PHYS_DIMS|TAGS_MASK_SET)
#define TAGS_FB_SET_VIRT_DIMS	(TAGS_FB_VIRT_DIMS|TAGS_MASK_SET)
#define TAGS_FB_SET_DEPTH		(TAGS_FB_DEPTH|TAGS_MASK_SET)
#define TAGS_FB_SET_PIXEL_ORDER	(TAGS_FB_PIXEL_ORDER|TAGS_MASK_SET)
#define TAGS_FB_SET_ALPHA_MODE	(TAGS_FBT_ALPHA_MODE|TAGS_MASK_SET)
#define TAGS_FB_SET_VIROFFSET	(TAGS_FB_VIROFFSET|TAGS_MASK_SET)
#define TAGS_FB_SET_OVERSCAN	(TAGS_FB_OVERSCAN|TAGS_MASK_SET)
#define TAGS_FB_SET_PALETTE		(TAGS_FB_PALETTE|TAGS_MASK_SET)
#define TAGS_FB_SET_TOUCHBUF	(0x0004801f|TAGS_MASK_SET)
#define TAGS_FB_SET_GPIOVIRTBUF	(0x00048020|TAGS_MASK_SET)
#define TAGS_FB_SET_VSYNC		(0x0004800e|TAGS_MASK_SET)
#define TAGS_FB_SET_BACKLIGHT	(0x0004800f|TAGS_MASK_SET)
#define TAGS_VCHIQ_INIT			(0x00048010|TAGS_MASK_SET)
/*----------------------------------------------------------------------------*/
#define TAGS_COMMAND_LINE		0x00050001
#define TAGS_DMA_CHANNELS		0x00060001
/*----------------------------------------------------------------------------*/
typedef struct _tags_head_t {
	unsigned int tags_id;
	unsigned int vbuf_size; /* usually 8-bytes */
	unsigned int req_res; /* req:0x00000000, res: 0x80000000 | value length */
	unsigned int vbuffer[2]; /* most tag response is 8-bytes long */
}
tags_head_t;
/*----------------------------------------------------------------------------*/
#define INFO_STATUS_OK 0
#define INFO_STATUS_UNKNOWN_ERROR_ 1
#define INFO_STATUS_INVALID_BUFFER 2
#define INFO_STATUS_REQUEST_FAILED 3
#define INFO_STATUS_REQUEST_ERROR_ 4
#define INFO_STATUS_READING_TAGS 5
/*----------------------------------------------------------------------------*/
typedef struct _tags_info_t {
	unsigned int info_status;
	unsigned int test, temp;
	volatile unsigned int *buff;
	/* hardware/board info */
	unsigned int vc_revision;
	unsigned int board_model;
	unsigned int board_revision;
	unsigned int board_mac_addrl; /* 6-bytes actually! */
	unsigned int board_mac_addrh;
	unsigned int board_serial_l;
	unsigned int board_serial_h;
	unsigned int memory_arm_base;
	unsigned int memory_arm_size;
	unsigned int memory_vc_base;
	unsigned int memory_vc_size;
	/* framebuffer info */
	unsigned int fb_width, fb_height;
	unsigned int fb_vwidth, fb_vheight;
	unsigned int fb_depth, fb_pixel_order;
	unsigned int fb_alpha_mode, fb_pitch;
	unsigned int fb_vx_offset, fb_vy_offset;
}
tags_info_t;
/*----------------------------------------------------------------------------*/
unsigned int* mailbox_get_board_info(tags_info_t* info);
unsigned int* mailbox_get_video_info(tags_info_t* info);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/

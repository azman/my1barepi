/*----------------------------------------------------------------------------*/
#include "raspi.h"
/*----------------------------------------------------------------------------*/
#define MAILBOX_BASE (PMAP_BASE|MAILBOX_OFFSET)
/*----------------------------------------------------------------------------*/
#include "boot.h"
#include "mailbox.h"
#include "barrier.h"
/*----------------------------------------------------------------------------*/
#define MAIL0_BASE    (MAILBOX_BASE+0x00)
#define MAIL0_READ    (MAILBOX_BASE+0x00)
#define MAIL0_POLL    (MAILBOX_BASE+0x10)
#define MAIL0_SEND_ID (MAILBOX_BASE+0x14)
#define MAIL0_STATUS  (MAILBOX_BASE+0x18)
#define MAIL0_CONFIG  (MAILBOX_BASE+0x1C)
#define MAIL0_WRITE   (MAILBOX_BASE+0x20)
/* MAIL0_WRITE IS ACTUALLY MAIL1_BASE/MAIL1_READ? */
#define MAIL1_BASE    (MAILBOX_BASE+0x20)
#define MAIL1_READ    (MAILBOX_BASE+0x20)
#define MAIL1_POLL    (MAILBOX_BASE+0x30)
#define MAIL1_SEND_ID (MAILBOX_BASE+0x34)
#define MAIL1_STATUS  (MAILBOX_BASE+0x38)
#define MAIL1_CONFIG  (MAILBOX_BASE+0x3C)
/**
 * Mailbox (MB) 0: VC -> ARM, MB 1: ARM->VC
 * - write to MB1, read from MB0!
**/
/*----------------------------------------------------------------------------*/
#define MAIL_STATUS_FULL  0x80000000
#define MAIL_STATUS_EMPTY 0x40000000
/*----------------------------------------------------------------------------*/
void mailbox_init(void)
{
	/* nothing to do! */
}
/*----------------------------------------------------------------------------*/
unsigned int mailbox_read(unsigned int channel)
{
	unsigned int value;
	while (1)
	{
		/* wait if mailbox is empty */
		while (get32(MAIL0_STATUS)&MAIL_STATUS_EMPTY);
		/* get value@channel */
		value = get32(MAIL0_BASE);
		/* check if the expected channel */
		if ((value&MAIL_CHANNEL_MASK)==channel) break;
		/* if not, data lost? should we store it somewhere? */
	}
	/* return address only */
	value &= ~MAIL_CHANNEL_MASK;
	return value;
}
/*----------------------------------------------------------------------------*/
void mailbox_write(unsigned int channel,unsigned int value)
{
	/* merge value/channel data */
	value &= ~MAIL_CHANNEL_MASK;
	value |= (channel&MAIL_CHANNEL_MASK);
	/* wait if mailbox is full */
	while (get32(MAIL1_STATUS)&MAIL_STATUS_FULL);
	/* read-write barrier */
	memory_barrier();
	/* send it to MB1! */
	put32(MAIL1_BASE,value);
}
/*----------------------------------------------------------------------------*/
/* for a 1kb mailbox buffer size */
#define MAIL_BUFFER_SIZE 256
/*----------------------------------------------------------------------------*/
volatile unsigned int mbbuff[MAIL_BUFFER_SIZE] __attribute__((aligned(16)));
/*----------------------------------------------------------------------------*/
int tags_init(volatile unsigned int* buff)
{
	int size = 1;
	buff[size++] = TAGS_STATUS_REQUEST;
	buff[size++] = TAGS_END;
	buff[0] = size*sizeof(unsigned int);
	return size;
}
/*----------------------------------------------------------------------------*/
int tags_insert(volatile unsigned int* buff, int size,
	unsigned int tags_id, int tags_buff_count)
{
	size--; /* override previous tags_end! */
	buff[size++] = tags_id;
	buff[size++] = tags_buff_count*sizeof(unsigned int);
	buff[size++] = TAGS_REQUESTS;
	while (tags_buff_count>0)
	{
		buff[size++] = 0x00000000;
		tags_buff_count--;
	}
	/* place terminating tags */
	buff[size++] = TAGS_END;
	/* update size */
	buff[0] = size*sizeof(unsigned int);
	return size;
}
/*----------------------------------------------------------------------------*/
int tags_isinfo(volatile unsigned int* buff, int size,
	unsigned int tags_id, int tags_needed, volatile tags_head_t** pptag)
{
	tags_head_t* ptag = (tags_head_t*)&buff[size];
	unsigned int test = ptag->req_res&TAGS_RESPONSE;
	unsigned int temp = ptag->req_res&~TAGS_RESPONSE;
	int vbufsize = tags_needed*sizeof(unsigned int);
	size += (ptag->vbuf_size/sizeof(unsigned int))+3;
	if (ptag->tags_id!=tags_id||ptag->vbuf_size<vbufsize||!test||temp>vbufsize)
		*pptag = 0x0;
	else
		*pptag = ptag;
	return size;
}
/*----------------------------------------------------------------------------*/
unsigned int* mailbox_get_board_info(tags_info_t* info)
{
	volatile tags_head_t *ptag;
	unsigned int temp = (unsigned int)mbbuff, test;
	int size, read;
	/* for DEBUG! */
	info->buff = mbbuff;
	/* configure buffer for request */
	size = tags_init(mbbuff);
	/* tag to request firmware revision */
	size = tags_insert(mbbuff,size,TAGS_FIRMWARE_REVISION,TAGS_RESPONSE_SIZE);
	/* tag to request board model */
	size = tags_insert(mbbuff,size,TAGS_BOARD_MODEL,TAGS_RESPONSE_SIZE);
	/* tag to request board revision */
	size = tags_insert(mbbuff,size,TAGS_BOARD_REVISION,TAGS_RESPONSE_SIZE);
	/* tag to request board mac addr */
	size = tags_insert(mbbuff,size,TAGS_BOARD_MAC_ADDR,TAGS_RESPONSE_SIZE);
	/* tag to request board serial num */
	size = tags_insert(mbbuff,size,TAGS_BOARD_SERIAL,TAGS_RESPONSE_SIZE);
	/* tag to request arm memory */
	size = tags_insert(mbbuff,size,TAGS_ARM_MEMORY,TAGS_RESPONSE_SIZE);
	/* tag to request videocore memory */
	size = tags_insert(mbbuff,size,TAGS_VC_MEMORY,TAGS_RESPONSE_SIZE);
	/* prepare address */
	temp |= VC_MMU_MAP;
	/* mail it! */
	memory_barrier();
	mailbox_write(MAIL_CH_TAGAV,temp);
	memory_barrier();
	test = mailbox_read(MAIL_CH_TAGAV);
	memory_barrier();
	/* validate response */
	if (test!=temp)
	{
		info->test = test;
		info->temp = temp;
		info->info_status = INFO_STATUS_INVALID_BUFFER;
		return 0x0;
	}
	/* DEBUG */
	info->test = mbbuff[0];
	info->temp = mbbuff[1];
	read = 2;
	/* get request status */
	switch(info->temp)
	{
		case TAGS_STATUS_SUCCESS:
			info->info_status = INFO_STATUS_READING_TAGS;
			break;
		case TAGS_STATUS_FAILURE:
			info->info_status = INFO_STATUS_REQUEST_FAILED;
			return 0x0;
		default:
			info->info_status = INFO_STATUS_REQUEST_ERROR_;
			return 0x0;
	}
	/* get firmware revision */
	read = tags_isinfo(mbbuff,read,TAGS_FIRMWARE_REVISION,1,&ptag);
	if (!ptag) return 0x0;
	info->vc_revision = ptag->vbuffer[0];
	/* get board model */
	read = tags_isinfo(mbbuff,read,TAGS_BOARD_MODEL,1,&ptag);
	if (!ptag) return 0x0;
	info->board_model = ptag->vbuffer[0];
	/* get board revision */
	read = tags_isinfo(mbbuff,read,TAGS_BOARD_REVISION,1,&ptag);
	if (!ptag) return 0x0;
	info->board_revision = ptag->vbuffer[0];
	/* get board mac addr */
	read = tags_isinfo(mbbuff,read,TAGS_BOARD_MAC_ADDR,2,&ptag);
	if (!ptag) return 0x0;
	/** need to convert network byte order to little endian! */
	info->board_mac_addrh = ptag->vbuffer[0];
	info->board_mac_addrl = ptag->vbuffer[1];
	/* get board serial num */
	read = tags_isinfo(mbbuff,read,TAGS_BOARD_SERIAL,2,&ptag);
	if (!ptag) return 0x0;
	info->board_serial_l = ptag->vbuffer[0];
	info->board_serial_h = ptag->vbuffer[1];
	/* get arm memory */
	read = tags_isinfo(mbbuff,read,TAGS_ARM_MEMORY,2,&ptag);
	if (!ptag) return 0x0;
	info->memory_arm_base = ptag->vbuffer[0];
	info->memory_arm_size = ptag->vbuffer[1];
	/* get vc memory */
	read = tags_isinfo(mbbuff,read,TAGS_VC_MEMORY,2,&ptag);
	if (!ptag) return 0x0;
	info->memory_vc_base = ptag->vbuffer[0];
	info->memory_vc_size = ptag->vbuffer[1];
	/* return pointer to buffer on success */
	info->info_status = INFO_STATUS_OK;
	return (unsigned int*)mbbuff;
}
/*----------------------------------------------------------------------------*/
unsigned int* mailbox_get_video_info(tags_info_t* info)
{
	volatile tags_head_t *ptag;
	unsigned int temp = (unsigned int)mbbuff, test;
	int size, read;
	/* for DEBUG! */
	info->buff = mbbuff;
	/* configure buffer for request */
	size = tags_init(mbbuff);
	/* tag to request physical display dimension */
	size = tags_insert(mbbuff,size,TAGS_FB_PHYS_DIMS,TAGS_RESPONSE_SIZE);
	/* tag to request virtual display dimension */
	size = tags_insert(mbbuff,size,TAGS_FB_VIRT_DIMS,TAGS_RESPONSE_SIZE);
	/* tag to request depth (bits-per-pixel) */
	size = tags_insert(mbbuff,size,TAGS_FB_DEPTH,TAGS_RESPONSE_SIZE);
	/* tag to request pixel order */
	size = tags_insert(mbbuff,size,TAGS_FB_PIXEL_ORDER,TAGS_RESPONSE_SIZE);
	/* tag to request alpha mode */
	size = tags_insert(mbbuff,size,TAGS_FB_ALPHA_MODE,TAGS_RESPONSE_SIZE);
	/* tag to request pitch */
	size = tags_insert(mbbuff,size,TAGS_FB_PITCH,TAGS_RESPONSE_SIZE);
	/* tag to request virtual offset */
	size = tags_insert(mbbuff,size,TAGS_FB_VIROFFSET,TAGS_RESPONSE_SIZE);
	/* prepare address */
	temp |= VC_MMU_MAP;
	/* mail it! */
	memory_barrier();
	mailbox_write(MAIL_CH_TAGAV,temp);
	memory_barrier();
	test = mailbox_read(MAIL_CH_TAGAV);
	memory_barrier();
	/* validate response */
	if (test!=temp)
	{
		info->test = test;
		info->temp = temp;
		info->info_status = INFO_STATUS_INVALID_BUFFER;
		return 0x0;
	}
	/* DEBUG */
	info->test = mbbuff[0];
	info->temp = mbbuff[1];
	read = 2;
	/* get request status */
	switch(info->temp)
	{
		case TAGS_STATUS_SUCCESS:
			info->info_status = INFO_STATUS_READING_TAGS;
			break;
		case TAGS_STATUS_FAILURE:
			info->info_status = INFO_STATUS_REQUEST_FAILED;
			return 0x0;
		default:
			info->info_status = INFO_STATUS_REQUEST_ERROR_;
			return 0x0;
	}
	/* get physical dimension */
	read = tags_isinfo(mbbuff,read,TAGS_FB_PHYS_DIMS,2,&ptag);
	if (!ptag) return 0x0;
	info->fb_width = ptag->vbuffer[0];
	info->fb_height = ptag->vbuffer[1];
	/* get virtual dimension */
	read = tags_isinfo(mbbuff,read,TAGS_FB_VIRT_DIMS,2,&ptag);
	if (!ptag) return 0x0;
	info->fb_vwidth = ptag->vbuffer[0];
	info->fb_vheight = ptag->vbuffer[1];
	/* get depth */
	read = tags_isinfo(mbbuff,read,TAGS_FB_DEPTH,1,&ptag);
	if (!ptag) return 0x0;
	info->fb_depth = ptag->vbuffer[0];
	/* get pixel order */
	read = tags_isinfo(mbbuff,read,TAGS_FB_PIXEL_ORDER,1,&ptag);
	if (!ptag) return 0x0;
	info->fb_pixel_order = ptag->vbuffer[0]; /** 0x0:BGR , 0x1:RGB */
	/* get apha mode */
	read = tags_isinfo(mbbuff,read,TAGS_FB_ALPHA_MODE,1,&ptag);
	if (!ptag) return 0x0;
	/** 0x0:enabled(0=opaque) , 0x1:reversed(0=transparent), 0x2:ignored */
	info->fb_alpha_mode = ptag->vbuffer[0];
	/* get pitch */
	read = tags_isinfo(mbbuff,read,TAGS_FB_PITCH,1,&ptag);
	if (!ptag) return 0x0;
	info->fb_pitch = ptag->vbuffer[0];
	/* get virtual offsets */
	read = tags_isinfo(mbbuff,read,TAGS_FB_VIROFFSET,2,&ptag);
	if (!ptag) return 0x0;
	info->fb_vx_offset = ptag->vbuffer[0];
	info->fb_vy_offset = ptag->vbuffer[1];
	/* return pointer to buffer on success */
	info->info_status = INFO_STATUS_OK;
	return (unsigned int*)mbbuff;
}
/*----------------------------------------------------------------------------*/

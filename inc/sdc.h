
#ifndef SDC_H
#define SDC_H

#include <stdint.h>
#include <sys/ioctl.h>

#include "hi_nnie.h"
#include "hi_type.h"


#ifdef __cplusplus
extern "C" {
#endif




/**
类型转定义
**/

typedef           char			CHAR;
typedef  unsigned char			UCHAR;
typedef signed char				INT8;
typedef unsigned char			UINT8;
typedef signed short			INT16;
typedef unsigned short			UINT16;
typedef signed int				INT32;
typedef unsigned int			UINT32;
typedef signed long long		INT64;
typedef unsigned long long		UINT64;


#define ERR -1
#define TRUE 1
#define FALSE 0
#define OK 0


#define SRVFS_FT_BUFFER 2
#define SRVFS_BUFF_SETWRSIZE _IO(SRVFS_FT_BUFFER,0x11)
#define SRVFS_BUFF_GETWRSIZE _IOR(SRVFS_FT_BUFFER,0x12,unsigned int)
#define SRVFS_BUFF_SETRDSIZE _IO(SRVFS_FT_BUFFER,0x13)
#define SRVFS_BUFF_GETRDSIZE _IOR(SRVFS_FT_BUFFER,0x14,unsigned int)
#define SRVFS_BUFF_GETMSGSIZE _IOR(SRVFS_FT_BUFFER,0x15,unsigned int)


#define SRVFS_FT_PHYMEM 5

typedef struct sdc_mem_stru
{
    void* addr_phy;
    void* addr_virt;
    unsigned int size;
}sdc_mem_s;

#define SRVFS_PHYMEM_MMAP _IOR(SRVFS_FT_PHYMEM,0x00,sdc_mem_s)
#define SRVFS_PHYMEM_MMAPCACHED _IOR(SRVFS_FT_PHYMEM,0x01,sdc_mem_s)
#define SRVFS_PHYMEM_MUNMAP _IOW(SRVFS_FT_PHYMEM,0x02,sdc_mem_s)
#define SRVFS_PHYMEM_CACHEFLUSH _IOW(SRVFS_FT_PHYMEM,0x03,sdc_mem_s)

/*
* SHM CACHE
*/
#define SDC_SHM_CACHE_DEV "/dev/cache"

typedef struct SDC_SHM_CACHE_Stru
{
    void* addr_virt;
    unsigned long addr_phy;
    unsigned int size;
    unsigned int cookie;
	int ttl;
}SDC_SHM_CACHE_S;

#define SDC_CACHE_ALLOC _IOR(SDC_FT_CACHE,0x00, SDC_SHM_CACHE_S)

#define SDC_FT_CACHE 7
#define SDC_CACHE_GETPHY _IOR(SDC_FT_CACHE,0x00, SDC_SHM_CACHE_S)
#define SDC_CACHE_MMAP _IOR(SDC_FT_CACHE,0x01, SDC_SHM_CACHE_S)
/* 消息头的数据结构定义
*/
#define SDC_VERSION 0x5331

#define SDC_METHOD_CREATE 1
#define SDC_METHOD_GET 2
#define SDC_METHOD_UPDATE 3
#define SDC_METHOD_DELETE 4

#define SDC_CODE_200	200
#define SDC_CODE_400	400
#define SDC_CODE_401	401
#define SDC_CODE_403	403
#define SDC_CODE_500	500

typedef struct sdc_common_head_stru
{
        uint16_t        version;
        uint8_t         url_ver;
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        uint8_t         method: 7;
        uint8_t         response: 1;
#elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
        uint8_t         response: 1;
        uint8_t         method: 7;
#else
#error "unknown __BYTE_ORDER__"
#endif
#else
#error "don't define __BYTE_ORDER__ or __ORDER_LITTLE_ENDIAN__ or __ORDER_BIG_ENDIAN__"
#endif
        uint16_t        url;
        uint16_t        code;
        uint16_t        head_length;
        uint16_t        trans_id;
        uint32_t        content_length;
}sdc_common_head_s;

typedef struct sdc_extend_head_stru
{
        uint16_t type;
        uint16_t length;
        uint32_t reserve;
}sdc_extend_head_s;

#define sdc_extend_head_length(extend_head) (((extend_head)->length + 7) & ~7)

#define sdc_extend_head_next(extend_head) ((sdc_extend_head_s*)((char*)extend_head + sdc_extend_head_length(extend_head)))

#define sdc_extend_head_first(common_head) ((sdc_extend_head_s*)(common_head + 1))

#define sdc_for_each_extend_head(common_head, extend_head) \
	for( extend_head = sdc_extend_head_first(common_head); (char*)extend_head - (char*)common_head < common_head->head_length; extend_head = sdc_extend_head_next(extend_head))

/**
* video.iaas.sdc服务的数据结构定义
*/

/**
* YUV Channel的数据格式定义
*/

#define SDC_URL_YUV_CHANNEL 	0x00

#define SDC_YVU_420SP 		0x00

typedef struct sdc_yuv_channel_param_stru
{
	uint32_t channel;
	uint32_t width;
	uint32_t height;
	uint32_t fps;
	uint32_t on_off;
	uint32_t format; // YUV_420SP 当前发的临时版本没有这个字段，正式版本补充进去
}sdc_yuv_channel_param_s;

typedef struct sdc_resolution_stru
{
	uint32_t width;
	uint32_t height;
}sdc_resolution_s;

typedef struct sdc_yuv_channel_info_stru
{
	sdc_yuv_channel_param_s param;
	sdc_resolution_s max_resolution;
	uint32_t is_snap_channel;
	uint32_t src_id;
	uint32_t subscribe_cnt;
	uint32_t nResolutionModitfy;/*此通道分辨率是否支持修改，0不支持修改，1支持修改*/
}sdc_yuv_channel_info_s;

#define SDC_URL_YUV_DATA		0x01

#define SDC_HEAD_YUV_SYNC 		0x00
#define SDC_HEAD_YUV_CACHED_COUNT_MAX 	0x01
#define SDC_HEAD_YUV_PARAM_MASK 	0x02

#define SDC_HEAD_YUV_CACHED_COUNT 	0x10
#define SDC_HEAD_YUV_PARAM_SNAP 	0x13


typedef struct sdc_yuv_frame_stru
{
	uint64_t addr_phy;
	uint64_t addr_virt;
    uint32_t size;
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	uint32_t format; // YUV_420SP
	uint32_t reserve;
	uint32_t cookie[4];
}sdc_yuv_frame_s;

typedef struct sdc_yuv_data_stru
{
	uint32_t channel;
	uint32_t reserve;
	uint64_t pts;
	uint64_t pts_sys;
	sdc_yuv_frame_s frame;
}sdc_yuv_data_s;

typedef struct sdc_chan_query_stru
{
	uint32_t channel;
}sdc_chan_query_s;

#define SDC_URL_VENC_DATA	0x03

#define SDC_HEAD_VENC_CACHED_COUNT_MAX	0x01
#define SDC_HEAD_VENC_CACHED_COUNT	0x10

#define SDC_VENC_FRAME_I	0x00
#define SDC_VENC_FRAME_P	0x01
#define SDC_VENC_FRAME_B	0x02

typedef struct sdc_venc_frame_stru
{
	uint64_t addr_phy;
	uint64_t addr_virt;
	uint64_t size;
	uint32_t width;
	uint32_t height;
	uint32_t format;
	uint32_t frame_type;
	uint64_t cookie[8];
}sdc_venc_frame_s;

typedef struct sdc_venc_data_stru
{
	uint32_t channel;
	uint32_t reserve;
	uint64_t ulpts;
	uint64_t pts_sys;
	sdc_venc_frame_s frame;
}sdc_venc_data_s;

#define SDC_URL_YUV_SNAP		0x04

struct sdc_yuv_snap_param_stru
{
	uint32_t id;
	uint32_t num;
	uint32_t interval_msec;
}sdc_yuv_snap_param_s;

#define SDC_URL_RED_LIGHT_ENHANCED	0x05

typedef struct sdc_region_stru
{
	uint32_t x;
	uint32_t y;
	uint32_t w;
	uint32_t h;
}sdc_region_s;

struct sdc_red_light_enhanced_param_stru
{
	uint32_t level;
	uint32_t num;
	uint32_t uImgWidth;  /*图像的宽度，用来计算比例*/
    uint32_t uImgHeight;  /*图像的高度，用来计算比例*/
	sdc_region_s regions[0];
}sdc_red_light_enhanced_param_s;

/**
* codec.iaas.sdc 服务接口数据结构定义
*/

#define SDC_URL_ENCODED_JPEG	0x00

typedef struct sdc_osd_stru
{
	uint8_t format[128];
	uint32_t reserve;
	uint32_t content_len;
	uint8_t content[2048];
}sdc_osd_s;

typedef struct sdc_osd_region_stru
{
	sdc_region_s region;
	sdc_osd_s osd;
}sdc_osd_region_s;

typedef struct sdc_encode_jpeg_param_stru
{
	uint16_t qf;
	uint16_t osd_region_cnt;
	uint32_t reserve;
	sdc_region_s region;
	sdc_yuv_frame_s frame;
	sdc_osd_region_s osd_region[0];
}sdc_encode_jpeg_param_s;

typedef struct sdc_jpeg_frame_stru
{
	uint64_t addr_phy;
	uint64_t addr_virt;
	uint32_t size;
	uint32_t reserve;
	uint32_t cookie[4];
}sdc_jpeg_frame_s;

#define SDC_URL_DECODED_YUV	0x01

#define SDC_HEAD_DECODED_YUV_ACCEPT_TYPE	0x00

#define SDC_BGR888	0x10
#define SDC_RGB888	0x11


#define SDC_URL_OSD_BOX_HEIGHT	0x02

typedef struct sdc_osd_box_stru
{
	uint32_t width;
	sdc_osd_s osd;
}sdc_osd_box_s;

#define SDC_URL_COMBINED_IMAGE	0x03
#define SDC_HEAD_COMBINED_CONTENT_TYPE	0x00
#define SDC_HEAD_COMBINED_JPEG_QF	0x01
#define SDC_HEAD_COMBINED_OSD	0x12 

typedef struct sdc_combined_yuv_stru
{
	sdc_region_s origin_region;
	sdc_region_s combined_region;
	sdc_yuv_frame_s frame;	
}sdc_combined_yuv_s;

typedef struct sdc_combined_yuv_param_stru
{
	uint32_t width;
	uint32_t height;
	uint32_t yuv_cnt;
	uint32_t reserve;
	sdc_combined_yuv_s yuv[0];
}sdc_combined_yuv_param_s;

/**
* utils.iaas.sdc服务接口数据结构定义
*/

#define SDC_URL_HARDWARE_ID	100

typedef struct sdc_hardware_id_stru
{
	char id[33];
}sdc_hardware_id_s;

#define SDC_URL_MMZ	101

#define SDC_HEAD_MMZ_CACHED	0x00

typedef struct sdc_mmz_alloc_stru
{
	uint64_t addr_phy;
	uint64_t addr_virt;
	uint32_t size;
	uint32_t reserve;
	uint32_t cookie[4];
}sdc_mmz_alloc_s;



/**
* appmgr.paas.sdc服务接口数据结构定义
*/

#define SDC_URL_APP_WATCHDOG 200

typedef struct appdog_op_req_stru
{
    int32_t watchdog_time;
}appdog_op_req_s;

/** 
* event.paas.sdc服务接口数据结果定义 
*/



#define event_paas_name "/mnt/srvfs/event.paas.sdc"

#define SDC_URL_PAAS_EVENTD_EVENT 0
#define SDC_HEAD_SHM_CACHED_EVENT 0xFFFF
typedef struct paas_event_stru
{
        char publisher[16];   //发送事件的服务标识，调测使用
        char name[16];      //事件唯一标识，建议同域名定义避免冲突
        uint64_t src_timestamp;  //发生时的时间，单位毫秒（CLOCK_MONOTONIC时间）
        uint64_t tran_timestamp; //服务转发的时间，单位毫秒（CLOCK_MONOTONIC时间）
        uint32_t id;        //建议同IP地址一样管理，不同前缀对应事件分类，方便分类订阅。
        uint32_t length;   //事件内容的长度.
        char data[0];
}paas_event_s;

typedef struct paas_shm_cached_event_stru
{
    uint64_t addr_phy;
    uint32_t size;
    uint32_t cookie;
}paas_shm_cached_event_s;

typedef struct paas_event_filter_stru
{
        char subscriber[16]; //订阅者的标识, 调测使用
        char name[16];
        char filter[256];
}paas_event_filter_s;

#define SDC_URL_PAAS_EVENTD_SUBSCRIBE_STAT 0X01

typedef struct paas_event_subscribe_stat_stru
{
        char name[16];
        uint64_t cnt;
        uint64_t fail_cnt;
}paas_event_subscribe_stat_s;

typedef struct LABEL_EVENT_DATA_STRU
{
	paas_event_s base;
	char* data;
}LABEL_EVENT_DATA_S;


#define SDC_URL_PAAS_EVENTD_PUBLISH_STAT 0X02

typedef struct paas_event_publish_stat_stru
{
        char name[16];
        uint32_t subscriber_cnt;
        uint64_t cnt;
        uint64_t trans_cnt; //成功转发的次数
        uint64_t fail_cnt; //转发失败的次数
}paas_event_publish_stat_s;



/**
*
* algorithm.iaas.sdc
*/
#define SDC_URL_NNIE_MODEL 0x00

#define SDC_URL_NNIE_FORWARD 0x01
#define SDC_URL_NNIE_FORWARD_BBOX 0x02
#define MAX_MODULE_PATH 100
#define NNIE_MODEL_CONTENT_MMZ 0
#define NNIE_MODEL_CONTENT_FILE 1
#define NNIE_NNIE_MODEL_OP 1

#define SDC_HEAD_NNIE_MODEL_CONTENT_TYPE 1


typedef struct sdc_nnie_forward_ctrl_stru
{
	uint32_t netseg_id;
	uint32_t max_batch_num;
	uint32_t max_bbox_num;
	uint32_t reserve;
}sdc_nnie_forward_ctrl_s;

typedef struct sdc_nnie_forward_stru
{
	SVP_NNIE_MODEL_S model;
	sdc_nnie_forward_ctrl_s forward_ctrl;
	SVP_SRC_BLOB_S astSrc[16];
	SVP_DST_BLOB_S astDst[16];
}sdc_nnie_forward_s;

typedef struct sdc_nnie_forward_withbox_ctrl_stru
{
	uint32_t proposal_num; 
	uint32_t netseg_id; 
	uint32_t max_batch_num;
	uint32_t max_bbox_num; 
}sdc_nnie_forward_withbox_ctrl_s;
typedef struct sdc_nnie_forward_withbox_stru
{
	SVP_NNIE_MODEL_S model; 
	sdc_nnie_forward_withbox_ctrl_s forward_ctl; 
	SVP_SRC_BLOB_S astSrc[16]; 
	SVP_SRC_BLOB_S astBbox[16]; 
	SVP_DST_BLOB_S astDst[16]; 
}sdc_nnie_forward_withbox_s;



#ifdef __cplusplus
}
#endif

#endif


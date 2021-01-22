/******************************************************************************

                  版权所有 (C), 2019-2029, SDC OS 开源软件小组所有

 ******************************************************************************
  文 件 名   : sdc_os_api.c
  版 本 号   : 初稿
  作    者   : jelly
  生成日期   : 2019年6月8日
  最近修改   :
  功能描述   : SDC OS基本服务化接口操作命令
  函数列表   :
  修改历史   :
  1.日    期   : 2019年6月8日
    作    者   : jelly
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

#include <inttypes.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <math.h>

#include "sdc_os_api.h"
#include "sample_comm_svp.h"
#include "sample_comm_ive.h"




int fd_video = -1;
int fd_codec = -1;
int fd_utils = -1;
int fd_algorithm = -1;
int fd_event = -1;
int fd_cache = -1;
unsigned int trans_id = 0;

/*ssd para*/
extern SAMPLE_SVP_NNIE_MODEL_S s_stYoloModel;
extern SAMPLE_SVP_NNIE_PARAM_S s_stYoloNnieParam;
extern SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S s_stYolov3SoftwareParam;
extern SAMPLE_SVP_NNIE_CFG_S   stNnieCfg_SDC;


extern HI_S32 SAMPLE_SVP_NNIE_Yolov3_GetResult(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,
    SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftwareParam);


/*****************************************************************************
 函 数 名  : SDC_ServiceCreate
 功能描述  : 打开服务文件句柄
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年6月8日
    作    者   : jelly
    修改内容   : 新生成函数

*****************************************************************************/
int SDC_ServiceCreate(void)
{
	fd_video = open("/mnt/srvfs/video.iaas.sdc",O_RDWR);
	if(fd_video < 0) goto video_fail;
	fd_codec = open("/mnt/srvfs/codec.iaas.sdc",O_RDWR);
	if(fd_codec < 0) goto codec_fail;
	fd_utils = open("/mnt/srvfs/utils.iaas.sdc",O_RDWR);
	if(fd_utils < 0) goto config_fail;
	fd_algorithm = open("/mnt/srvfs/algorithm.iaas.sdc",O_RDWR);
	if(fd_algorithm < 0) goto algorithm_fail;

	fd_event = open("/mnt/srvfs/event.paas.sdc",O_RDWR);
	if(fd_event < 0) goto event_fail;
	fd_cache = open("/dev/cache",O_RDWR);
	if(fd_cache < 0) goto cache_fail;
	
	return 0;

cache_fail:
	fprintf(stderr, "open /dev/cache fail in SDC_ServiceCreate!\r\n");
	close(fd_event);
	fd_event = -1;
event_fail:
	fprintf(stderr, "open event fail in SDC_ServiceCreate!\r\n");
	close(fd_algorithm);
	fd_algorithm = -1;
algorithm_fail:
	fprintf(stderr, "errno:%d open algorithm.iaas.sdc fail！\r\n", errno);
	close(fd_utils);
	fd_utils = -1;
config_fail:
	close(fd_codec);
	fprintf(stderr, "errno:%d open utils.iaas.sdc fail！\r\n", errno);
	fd_codec = -1;
codec_fail:
	close(fd_video);
	fprintf(stderr, "errno:%d open codec.iaas.sdc fail！\r\n", errno);
	fd_video = -1;
video_fail:
	fprintf(stderr, "errno:%d open video.iaas.sdc fail！\r\n", errno);
	return errno;
}

/*****************************************************************************
 函 数 名  : SDC_GetHardWareId
 功能描述  : 获取硬件ID
 输入参数  : sdc_hardware_id_s *pstHardWareParas
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年6月8日
    作    者   : jelly
    修改内容   : 新生成函数

*****************************************************************************/
int SDC_GetHardWareId(sdc_hardware_id_s *pstHardWareParas)
{
	sdc_common_head_s head;	
	struct iovec iov[2] = 
    {
		[0] = {.iov_base = &head, .iov_len = sizeof(head) },
		[1] = {.iov_base = pstHardWareParas->id, .iov_len = sizeof(sdc_hardware_id_s) },
	};
	ssize_t retn;

	memset(&head,0,sizeof(head));
	head.version = SDC_VERSION;
	head.url = SDC_URL_HARDWARE_ID;
	head.method = SDC_METHOD_GET;
	head.trans_id = ++trans_id;
	head.head_length = sizeof(head);

	retn = write(fd_utils, &head, sizeof(head));
	if(retn != sizeof(head)) 
    {
		fprintf(stderr, "write to config.iaas.sdc fail: %m\n");
		return ERR;
	}

	retn = readv(fd_utils,iov,2);
	if(retn == ERR) 
    {
		fprintf(stderr, "read from config.iaas.sdc fail!\n");
		return ERR;
	}

	if(head.code != SDC_CODE_200) 
    {
		fprintf(stderr, "read from config.iaas.sdc fail: code = %u\n",head.code);
		return ERR;
	}

	if(head.trans_id != trans_id) 
    {
		fprintf(stderr, "read from config.iaas.sdc fail: wrong trans-id= %u, expected = %u\n",head.trans_id,trans_id);
	}
	
    return OK;
}



/*****************************************************************************
 函 数 名  : SDC_YuvChnAttrSet
 功能描述  : 设置YUV通道参数
 输入参数  : int fd  
                           int uiYuvChnId
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年6月8日
    作    者   : jelly
    修改内容   : 新生成函数

*****************************************************************************/
int SDC_YuvChnAttrSet(int fd, int uiYuvChnId)
{
	sdc_yuv_channel_param_s param = 
    {
		.channel = uiYuvChnId,
		.width = 416,
		.height = 416,
		.fps = 10,
		.on_off = 1,
		.format = SDC_YVU_420SP,
	};

	sdc_common_head_s head = 
    {
		.version = SDC_VERSION, //0x5331
		.url = SDC_URL_YUV_CHANNEL, //0x00
		.method = SDC_METHOD_UPDATE, //0x02
		.content_length = sizeof(param),
		.head_length = sizeof(head),
	};
	struct iovec iov[2] = { {.iov_base = &head, .iov_len = sizeof(head)},
		{.iov_base = &param, .iov_len = sizeof(param) }};
	int nret;

	nret = writev(fd, iov, 2);
	if(nret < 0)
	{
	   fprintf(stdout,"writev fail yuv_channel_set,response:%d,url:%d,code:%d,method:%d\n",head.response,head.url,head.code, head.method);
	   return errno;
	}

	fprintf(stdout,"yuv_channel_set write succeed \n");

	nret = read(fd,&head, sizeof(head));
	if(nret < 0 || head.code != SDC_CODE_200)
	{
	   fprintf(stdout,"read fail response:%d,url:%d,code:%d, method:%d\n",head.response,head.url,head.code, head.method);
	   return errno;
	}

	fprintf(stdout,"yuv_channel_set read succeed content_length:%d \n", head.content_length);
	
	return OK;
}



/*****************************************************************************
 函 数 名  : SDC_YuvChnAttrGet
 功能描述  : 查询YUV逻辑通道信息
 输入参数  : int fd  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年6月8日
    作    者   : jelly
    修改内容   : 新生成函数

*****************************************************************************/
int SDC_YuvChnAttrGet(int fd)
{
	int nret,i;
	char buf[1024] = { 0 };
	sdc_yuv_channel_info_s* info;
	sdc_common_head_s* head = (sdc_common_head_s*)buf;
	sdc_chan_query_s* param;

	/** query all channels' info */
	head->version = SDC_VERSION; //0x5331
	head->url = SDC_URL_YUV_CHANNEL; //0x00
	head->method = SDC_METHOD_GET; //0x02
	head->head_length = sizeof(*head);

	param = (sdc_chan_query_s*)(buf+head->head_length);
    param->channel = 0;/*通道ID为0时查询的是所有的逻辑通道*/
    head->content_length = sizeof(sdc_chan_query_s);

	head->content_length = 0;
    
	nret = write(fd, head, head->head_length);
	if(nret < 0)
	{
	   fprintf(stdout,"writev fail yuv_channel_get,response:%d,url:%d,code:%d,method:%d\n",head->response,head->url,head->code, head->method);
	   return errno;
	}
	fprintf(stdout,"yuv_channel_get write succeed \n");

	nret = read(fd,buf,sizeof(buf));
	if(nret < 0 || head->code != SDC_CODE_200)
	{
	   fprintf(stdout,"read fail response:%d,url:%d,code:%d, method:%d\n",head->response,head->url,head->code, head->method);
	   return errno;
	}
	
	fprintf(stdout,"yuv_channel_get read succeed content_length:%d \n", head->content_length);


	info = (sdc_yuv_channel_info_s*)&buf[head->head_length];
	for(i = 0; i < head->content_length / sizeof(*info); ++i, ++info) 
	{
		fprintf(stdout,"channel info:is_snap_channel:%d,src_id:%d,subscribe_cnt:%d,width:%d,height:%d,channel:%d,fps:%d,on_off:%d\n",
			info->is_snap_channel, info->src_id, info->subscribe_cnt, info->max_resolution.width, 
			info->max_resolution.height, info->param.channel, info->param.fps, info->param.on_off);
	}
    return 0;
}


void SDC_YuvDataFree(int fd,  sdc_yuv_data_s *yuv_data)
{
	int nret;
	char buf[1024] = { 0 };
	sdc_common_head_s* head = (sdc_common_head_s*) buf;

    /** free yuv_data */

    head->version = SDC_VERSION;
	head->url = SDC_URL_YUV_DATA;
	head->method = SDC_METHOD_DELETE;
	head->head_length = sizeof(sdc_common_head_s);    
    head->response = head->code = 0;
    head->content_length = sizeof(sdc_yuv_data_s); 
    
    memcpy(&buf[head->head_length],yuv_data,sizeof(sdc_yuv_data_s));
    
    nret = write(fd, buf, head->head_length + sizeof(sdc_yuv_data_s)); 
    if(nret < 0)
    {
        fprintf(stdout,"writev fail yuv_frame_get,response:%d,url:%d,code:%d,method:%d\n",
        head->response,head->url,head->code, head->method);
    }

    return;
}


int SDC_YuvDataReq(int fd, int extendheadflag, unsigned int uiChnId, unsigned int uiMaxUsedBufNum)
{
	int nret;
	char buf[1024] = { 0 };
	sdc_common_head_s* head = (sdc_common_head_s*) buf;
	sdc_extend_head_s* extend_head;
	uint32_t* channel;

	head->version = SDC_VERSION;
	head->url = SDC_URL_YUV_DATA;
	head->method = SDC_METHOD_GET;
	head->head_length = sizeof(*head);

	fprintf(stdout,"yuv_frame_get:extendheadflag:%d\n",extendheadflag);

    if (0x01 == (extendheadflag&0x01))
	{
	    extend_head = (sdc_extend_head_s*)&buf[head->head_length];
		extend_head->type = SDC_HEAD_YUV_SYNC;
		extend_head->length = sizeof(*extend_head);
		extend_head->reserve = 0;
		head->head_length += sdc_extend_head_length(extend_head);
	}

	if (0x02 == (extendheadflag&0x02))
	{
		extend_head = (sdc_extend_head_s*)&buf[head->head_length];
		extend_head->type = SDC_HEAD_YUV_CACHED_COUNT_MAX;
		extend_head->length = sizeof(*extend_head);
		extend_head->reserve = uiMaxUsedBufNum;
	    
		head->head_length += sdc_extend_head_length(extend_head);
	}

	if (0x04 == (extendheadflag&0x04))
	{
		extend_head = (sdc_extend_head_s*)&buf[head->head_length];
		extend_head->type = SDC_HEAD_YUV_PARAM_MASK;
		extend_head->length = sizeof(*extend_head);
		extend_head->reserve = 8;

		head->head_length += sdc_extend_head_length(extend_head);
	}

	channel = (uint32_t*)&buf[head->head_length];
	
    channel[0] = uiChnId;          
    head->content_length = 1 * sizeof(uint32_t);
        
	nret = write(fd, head, head->head_length + head->content_length);
	if(nret < 0)
	{
	   fprintf(stdout,"writev fail yuv_frame_get,response:%d,url:%d,code:%d,method:%d\n",
            head->response,head->url,head->code, head->method);
	   return ERR;
	}

	nret = read(fd, buf,sizeof(buf));
	if(nret < 0)
	{
	   fprintf(stdout,"read fail yuv_frame_get,response:%d,url:%d,code:%d,method:%d\n",
            head->response,head->url,head->code, head->method);
	   return ERR;
	}
    
    return OK;    
}
/*****************************************************************************
 函 数 名  : SDC_MemAlloc
 功能描述  : 申请内存函数
 输入参数  : int fd                
                           unsigned int size         
                           sdc_mmz_alloc_s* mmz  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年6月8日
    作    者   : jelly
    修改内容   : 新生成函数

*****************************************************************************/
int SDC_MemAlloc(int fd, unsigned int size, int uiCacheFlag, sdc_mmz_alloc_s* pstMemParas)
{

    //fprintf(stdout,"enter into mmz_alloc_cached:size:%d\n",size);
	sdc_common_head_s head = {
		.version = SDC_VERSION,
		.url = SDC_URL_MMZ,
		.method = SDC_METHOD_CREATE,
		.head_length = sizeof(head),// + sizeof(cached_head),
		.content_length = sizeof(size),
	};

	struct iovec iov[] = {
		{ (void*)&head, sizeof(head) },
		//{ (void*)&cached_head, sizeof(cached_head) },
		{ &size, sizeof(size) }
	};

    
	int nret = writev(fd, iov, 2);
	if(nret < 0) return errno;
	//fprintf(stdout,"mmz_alloc_cached:1\n");

	iov[1].iov_base = pstMemParas;
	iov[1].iov_len = sizeof(*pstMemParas);

	nret = readv(fd, iov,2);
	if(nret < 0) 
    {
        fprintf(stdout,"mmz_alloc_cached:1\n");
        return errno;
    }
	//fprintf(stdout,"mmz_alloc_cached:2\n");

	if(head.code != SDC_CODE_200 || head.head_length != sizeof(head) || head.content_length != sizeof(*pstMemParas)) 
    {
        fprintf(stdout,"mmz_alloc_cached:2,size:%d\n", pstMemParas->size);
        return EIO;       
    }
    //fprintf(stdout,"mmz succeed mmz_alloc_cached:size:%d\n",size);
	return size;
}

/*****************************************************************************
 函 数 名  : SDC_MemFree
 功能描述  : 释放申请的内存
 输入参数  : int fd                
             sdc_mmz_alloc_s* mmz  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年6月8日
    作    者   : jelly
    修改内容   : 新生成函数

*****************************************************************************/
void SDC_MemFree(int fd, sdc_mmz_alloc_s* pstMemParas)
{
	sdc_common_head_s head = 
    {
		.version = SDC_VERSION,
		.url = SDC_URL_MMZ,
		.method = SDC_METHOD_DELETE,
		.head_length = sizeof(head),
		.content_length = sizeof(*pstMemParas),
	};

	struct iovec iov[] = {
		{ (void*)&head, sizeof(head) },
		{ pstMemParas, sizeof(*pstMemParas) }
	};

	(void)writev(fd, iov, sizeof(iov)/sizeof(iov[0]));
    return ;
}


void SDC_DisplayYuvData(sdc_yuv_data_s* yuv_data)
{
    fprintf(stdout,"SDC_DisplayYuvData,channel:%d,reserve:%d,pts:%ld,pts_sys:%ld,addr_phy:0x%lx,addr_virt:0x%lx,size:%d\n",
		yuv_data->channel, yuv_data->reserve,yuv_data->pts,yuv_data->pts_sys,
		yuv_data->frame.addr_phy,yuv_data->frame.addr_virt,yuv_data->frame.size);
    return;
}


void SDC_DisplayExtendHead(sdc_extend_head_s* extendhead)
{
	fprintf(stdout, "*******SDC_DisplayExtendHead********:type=%u, length=%u, reserve=%u\n", 
        extendhead->type,extendhead->length,extendhead->reserve);
    return;
}

int SDC_LoadModel(unsigned int uiLoadMode, char *pucModelFileName, SVP_NNIE_MODEL_S *pstModel)
{
    int s32Ret = 0;
    int ret = 0;
    sdc_extend_head_s* extend_head;
    char buf[1024] = {0};
    sdc_common_head_s *phead = (sdc_common_head_s *)buf;
    unsigned int uFileSize;
    sdc_mmz_alloc_s stMmzAddr;
    if ((NULL == pstModel) || (NULL == pucModelFileName))
    {
        fprintf(stdout,"Err in SDC_LoadModel, pstModel or pucModelFileName is null\n");
        return -1;
    }
    
    fprintf(stdout,"Load model, pucModelFileName:%s!\n", pucModelFileName);

    struct rsp_strcut 
    {
        sdc_common_head_s head;
        SVP_NNIE_MODEL_S model;
    }rsp_strcut_tmp;
    
    struct iovec iov[2] = 
    {
        [0] = { .iov_base = buf, .iov_len = sizeof(sdc_common_head_s) + sizeof(sdc_extend_head_s)},
        [1] = { .iov_len = MAX_MODULE_PATH}
    };
    //memset(&head, 0, sizeof(head));
    phead->version = SDC_VERSION;
    phead->url = SDC_URL_NNIE_MODEL;
    phead->method = SDC_METHOD_CREATE;
    phead->head_length = sizeof(sdc_common_head_s);
    phead->content_length = MAX_MODULE_PATH;
    /*模式 0，不带扩展头，默认内存方式加载*/
    if (uiLoadMode == 0)
    {
        FILE *fp = fopen(pucModelFileName, "rb");
        if(fp == NULL)
        {
            fprintf(stdout,"modelfile fopen %s fail!\n", pucModelFileName);
            return -1;
        }
        ret = fseek(fp,0L,SEEK_END);
        if(ret != 0)
        {
            fprintf(stdout,"check nnie file SEEK_END, fseek fail.");
            fclose(fp);
            return -1;
        }
        
        uFileSize = ftell(fp);
        ret = fseek(fp,0L,SEEK_SET);
        if(0 != ret)
        {
            fprintf(stdout,"check nnie file SEEK_SET, fseek fail.");
            fclose(fp);
            return -1;
        }
        
        stMmzAddr.size = uFileSize;
        ret = SDC_MemAlloc(fd_utils, uFileSize, 0, &stMmzAddr); // param 2: 0 no cache, 1 cache
        if(ret != stMmzAddr.size)
        {
            fprintf(stdout,"SDC_MmzAlloc ret %d, readsize %d", ret, stMmzAddr.size);
            return -1;
        }
        
        ret = fread((HI_VOID*)(uintptr_t)stMmzAddr.addr_virt, 1, stMmzAddr.size, fp);
        if(ret != stMmzAddr.size)
        {
            fprintf(stdout,"filesize %d, readsize %d", ret, stMmzAddr.size);
            return -1;
        }
        /*用户执行调用算法程序对传入文件进行解码*/
        if(SDC_ModelDecript(&stMmzAddr))
        {
            fprintf(stdout,"SDC_ModelDecript Fail!");
            return -1; 
        }
        iov[1].iov_base = &stMmzAddr; 
        iov[0].iov_len = sizeof(sdc_common_head_s);
    }

    else if (uiLoadMode == 1)/*模式 1，带扩展头，扩展头参数指定为内存方式加载*/
    {
        FILE *fp = fopen(pucModelFileName, "rb");
        if(fp == NULL)
        {
            fprintf(stdout,"modelfile fopen %s fail!\n", pucModelFileName);
            return -1;
        }
        ret = fseek(fp,0L,SEEK_END);
        if(ret != 0)
        {
            fprintf(stdout,"check nnie file SEEK_END, fseek fail.");
            fclose(fp);
            return -1;
        }
        
        uFileSize = ftell(fp);
        ret = fseek(fp,0L,SEEK_SET);
        if(0 != ret)
        {
            fprintf(stdout,"check nnie file SEEK_SET, fseek fail.");
            fclose(fp);
            return -1;
        }

        stMmzAddr.size = uFileSize;
        ret = SDC_MemAlloc(fd_utils, uFileSize, 0, &stMmzAddr); // param 2: 0 no cache, 1 cache
        if(ret != stMmzAddr.size)
        {
            fprintf(stdout,"SDC_MmzAlloc ret %d, readsize %d", ret, stMmzAddr.size);
            return -1;
        }
        ret = fread((HI_VOID*)(uintptr_t)stMmzAddr.addr_virt, 1, stMmzAddr.size, fp);
        if(ret != stMmzAddr.size)
        {
            fprintf(stdout,"filesize %d, readsize %d", ret, stMmzAddr.size);
            return -1;
        }
        /*用户执行调用算法程序对传入文件进行解码*/
        if(SDC_ModelDecript(&stMmzAddr))
        {
            fprintf(stdout,"SDC_ModelDecript Fail!");
            return -1;
        }
        extend_head = (sdc_extend_head_s *)&buf[phead->head_length];
        extend_head->type = 1;//NNIE_NNIE_MODEL_OP
        extend_head->length = sizeof(*extend_head);
        extend_head->reserve = 0;/*0 或者不带是内存方式，1 是文件名方式*/
        phead->head_length += sizeof(sdc_extend_head_s);
        iov[1].iov_base = &stMmzAddr; 
    }
    else /*模式 2，带扩展头，扩展头参数指定为文件名方式加载*/
    {
        extend_head = (sdc_extend_head_s *)&buf[phead->head_length];
        extend_head->type = 1;//NNIE_NNIE_MODEL_OP
        extend_head->length = sizeof(*extend_head);
        extend_head->reserve = 1;/*0 或者不带是内存方式，1 是文件名方式*/
        phead->head_length += sizeof(sdc_extend_head_s);
        iov[1].iov_base = pucModelFileName;//pcModelName; 
    }
    s32Ret = writev(fd_algorithm, iov, 2);
    if (s32Ret < 0)
    {
        fprintf(stdout,"creat nnie,write to algorithm.iaas.sdc fail: %m\n");
    }
    
    /*模型加载后立即释放*/
    //if (uiLoadMode < 2)mmz_free(fd_config, &stMmzAddr);
    
    s32Ret = read(fd_algorithm, &rsp_strcut_tmp, sizeof(rsp_strcut_tmp));
    if(s32Ret == -1)
    {
        fprintf(stdout,"get_channel_data fail: %m\n");
        return -1;
    }
    if(s32Ret > sizeof(rsp_strcut_tmp))
    {
        fprintf(stdout,"get_channel_data truncated, data len: %d > %zu\n", s32Ret, sizeof(rsp_strcut_tmp));
        return -1;
    }
    if (s32Ret < 0 || rsp_strcut_tmp.head.code != SDC_CODE_200 || rsp_strcut_tmp.head.content_length <= 0)
    {
        fprintf(stdout,"get nnie create response, read from algorithm.iaas.sdc fail,s32Ret:%d, code=%d,length=%d\n", 
            s32Ret, rsp_strcut_tmp.head.code, rsp_strcut_tmp.head.content_length);
    }
    else 
    {
        s_stYoloModel.stModel = rsp_strcut_tmp.model;
        memcpy(pstModel, &rsp_strcut_tmp.model,sizeof(SVP_NNIE_MODEL_S));
    }
    
    return OK;
}


int SDC_UnLoadModel(SVP_NNIE_MODEL_S *pstModel)
{
    int nRet = -1;
    if (NULL != pstModel)
    {
        sdc_common_head_s head;
        struct iovec iov[2] = {
        [0] = {.iov_base = &head , .iov_len = sizeof(head)},
        [1] = {.iov_base = pstModel, .iov_len = 
        sizeof(SVP_NNIE_MODEL_S)}};
        // fill head struct 
        memset(&head, 0, sizeof(head));
        head.version = SDC_VERSION;
        head.url = SDC_URL_NNIE_MODEL;
        head.method = SDC_METHOD_DELETE;
        head.head_length = sizeof(head);
        head.content_length = sizeof(SVP_NNIE_MODEL_S);
        nRet = writev(fd_algorithm, iov, sizeof(iov)/sizeof(iov[0]));
        if (nRet < 0)
        {
            fprintf(stdout,"Errin SDC_UnLoadModel:failed to unload nnie module!\n");
        } 
    }
    else
    {
        fprintf(stdout,"Err in SDC_UnLoadModel:module pointer is NULL!\n");
    }
    
    return 0;
}

int SDC_Nnie_Forward(sdc_nnie_forward_s *p_sdc_nnie_forward)
{
    int nRet;
    sdc_common_head_s rsp_head;
    sdc_common_head_s head;
    struct iovec iov[2] = 
    {
        [0] = {.iov_base = &head, .iov_len = sizeof(head)},
        [1] = {.iov_base = p_sdc_nnie_forward, .iov_len = sizeof(*p_sdc_nnie_forward)}
    };
    // fill head struct 
    memset(&head, 0, sizeof(head));
    head.version = SDC_VERSION;
    head.url = SDC_URL_NNIE_FORWARD;
    head.method = SDC_METHOD_GET;
    head.head_length = sizeof(head);
    head.content_length = sizeof(*p_sdc_nnie_forward);

    // write request
    nRet = writev(fd_algorithm, iov, sizeof(iov)/sizeof(iov[0]));
    if (nRet < 0)
    {
        fprintf(stdout,"Error:failed to write info to NNIE Forward,nRet:%d!\n",nRet);
        return ERR;
    }
    // read response
    iov[0].iov_base = &rsp_head;
    iov[0].iov_len = sizeof(rsp_head);
    nRet = readv(fd_algorithm, iov, 1);
    if (rsp_head.code != SDC_CODE_200 || nRet < 0)
    {
        fprintf(stdout,"Error:failed to read info from NNIE Forward,nRet:%d,rsp_head.code:%d!\n",
            nRet, rsp_head.code);
        return ERR;
    } 
    return OK;
}


int SDC_Nnie_Forward_Withbox(sdc_nnie_forward_withbox_s *p_sdc_nnie_forward_withbox)
{
    int nRet;
    sdc_common_head_s rsp_head;
    sdc_common_head_s head;
    struct iovec iov[2] = 
    {
        [0] = {.iov_base = &head , .iov_len = sizeof(head)},
        [1] = {.iov_base = p_sdc_nnie_forward_withbox, .iov_len = sizeof(*p_sdc_nnie_forward_withbox)}
    };
    
    // fill head struct 
    memset(&head, 0, sizeof(head));
    head.version = SDC_VERSION;
    head.url = SDC_URL_NNIE_FORWARD_BBOX;
    head.method = SDC_METHOD_GET;
    head.head_length = sizeof(head);
    head.content_length = sizeof(*p_sdc_nnie_forward_withbox);
    
    // write request
    nRet = writev(fd_algorithm, iov, sizeof(iov)/sizeof(iov[0]));
    if (nRet < 0)
    {
        fprintf(stdout,"Error:failed to write info to NNIE Forward_With_Box!\n");
        return ERR;
    }
    // read response
    iov[0].iov_base = &rsp_head;
    iov[0].iov_len = sizeof(rsp_head);
    nRet = readv(fd_algorithm, iov, 1);
    
    if (rsp_head.code != SDC_CODE_200 || nRet < 0)
    {
        fprintf(stdout,"Error:failed to read info from NNIE Forward_With_Box!\n");
        return ERR;
    }
    return OK;

}

INT64 AllocRGBMmz(VW_YUV_FRAME_S *subImage)
{
    HI_S32 ulRet = 0;
    void *rgbPhyAddr = NULL;
    void *rgbVitAddr = NULL;

    sdc_mmz_alloc_s stMmzAddr;

    stMmzAddr.size = subImage->uWidth * subImage->uHeight * 3;
    ulRet = SDC_MemAlloc(fd_utils, stMmzAddr.size, 1,&stMmzAddr); // param 2: 0 no cache, 1 cache
    if(ulRet != stMmzAddr.size)
    {
        fprintf(stdout,"SDC_MemAlloc ret %d, readsize %d", ulRet, stMmzAddr.size);
        return -1;
    }

    rgbPhyAddr = (void *)stMmzAddr.addr_phy;
    rgbVitAddr = (void *)stMmzAddr.addr_virt;
        
    for (int j = 0; j < 3; j++)
    {
        subImage->ulPhyAddr[j]  = (UINT64)(rgbPhyAddr) + (UINT64)j * subImage->uWidth * subImage->uHeight;
        subImage->ulVirAddr[j]  = (UINT64)(rgbVitAddr) + (UINT64)j * subImage->uWidth * subImage->uHeight;
        subImage->uStride[j]    = subImage->uWidth;
    }
    subImage->pYuvImgAddr = (CHAR *)(rgbVitAddr);
    
    return 0;
}


int SDC_ModelDecript(sdc_mmz_alloc_s *pstMmzAddr)
{
    if(pstMmzAddr == NULL)
    {
        printf("Err in SDC_ModelDecript, pstMmzAddr is null\n");
        return ERR;
    }    
    return OK;
}

static HI_S32 SDC_SVP_NNIE_Detection_GetResult(SVP_BLOB_S *pstDstScore,
    SVP_BLOB_S *pstDstRoi, SVP_BLOB_S *pstClassRoiNum, SDC_SSD_RESULT_S *pstResult)
{
    HI_U32 i = 0, j = 0;
    HI_U32 u32RoiNumBias = 0;
    HI_U32 u32ScoreBias = 0;
    HI_U32 u32BboxBias = 0;
    HI_FLOAT f32Score = 0.0f;
    HI_S32* ps32Score = (HI_S32*)pstDstScore->u64VirAddr;
    HI_S32* ps32Roi = (HI_S32*)pstDstRoi->u64VirAddr;
    HI_S32* ps32ClassRoiNum = (HI_S32*)pstClassRoiNum->u64VirAddr;
    HI_U32 u32ClassNum = pstClassRoiNum->unShape.stWhc.u32Width;
    // printf("\nu32ClassNum===%d\n",u32ClassNum);
    HI_S32 s32XMin = 0,s32YMin= 0,s32XMax = 0,s32YMax = 0;

	HI_S32 MaxObjectNum = pstResult->numOfObject;
	pstResult->numOfObject = 0;

    u32RoiNumBias += ps32ClassRoiNum[0];
    for (i = 1; i < u32ClassNum; i++)
    {
        u32ScoreBias = u32RoiNumBias;
        u32BboxBias = u32RoiNumBias * SAMPLE_SVP_NNIE_COORDI_NUM;
        /*if the confidence score greater than result threshold, the result will be printed*/
        if((HI_FLOAT)ps32Score[u32ScoreBias] / SAMPLE_SVP_NNIE_QUANT_BASE >=
            pstResult->thresh && ps32ClassRoiNum[i]!=0)
        {
 //           SAMPLE_SVP_TRACE_INFO("==== The %dth class box info====\n", i);
        }
        for (j = 0; j < (HI_U32)ps32ClassRoiNum[i]; j++)
        {
            f32Score = (HI_FLOAT)ps32Score[u32ScoreBias + j] / SAMPLE_SVP_NNIE_QUANT_BASE;
            if (f32Score < pstResult->thresh)
            {
                break;
            }
            s32XMin = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM];
            s32YMin = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 1];
            s32XMax = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 2];
            s32YMax = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 3];
            //SAMPLE_SVP_TRACE_INFO("%d %d %d %d %f\n", s32XMin, s32YMin, s32XMax, s32YMax, f32Score);
			pstResult->pObjInfo[pstResult->numOfObject].class = i;
			pstResult->pObjInfo[pstResult->numOfObject].confidence = f32Score;
			pstResult->pObjInfo[pstResult->numOfObject].x_left = s32XMin;
			pstResult->pObjInfo[pstResult->numOfObject].y_top = s32YMin;
			pstResult->pObjInfo[pstResult->numOfObject].x_right = s32XMax;
			pstResult->pObjInfo[pstResult->numOfObject].y_bottom = s32YMax;
			pstResult->pObjInfo[pstResult->numOfObject].w = s32XMax - s32XMin; 
			pstResult->pObjInfo[pstResult->numOfObject].h = s32YMax - s32YMin;
			pstResult->numOfObject += 1;
			if(pstResult->numOfObject == MaxObjectNum)
				return HI_SUCCESS;
        }
        u32RoiNumBias += ps32ClassRoiNum[i];
    }
    return HI_SUCCESS;
}



/*
*Fulsh cached
*/
HI_S32 SDC_FlushCache(HI_U64 u64PhyAddr, HI_VOID *pvVirAddr, HI_U32 u32Size)
{
	HI_S32 s32Ret = HI_SUCCESS;
    sdc_mem_s sdc_mem_addr;
    sdc_mem_addr.addr_phy = (void *)u64PhyAddr;
    sdc_mem_addr.addr_virt = pvVirAddr;
    sdc_mem_addr.size = u32Size;

    s32Ret = ioctl(fd_utils, SRVFS_PHYMEM_CACHEFLUSH,&sdc_mem_addr);

	return s32Ret;
}



/******************************************************************************
* function : Fill Src Data
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_FillSrcData(SAMPLE_SVP_NNIE_CFG_S* pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S* pstInputDataIdx)
{
    FILE* fp = NULL;
    HI_U32 i =0, j = 0, n = 0;
    HI_U32 u32Height = 0, u32Width = 0, u32Chn = 0, u32Stride = 0, u32Dim = 0;
    HI_U32 u32VarSize = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8*pu8PicAddr = NULL;
    HI_U32*pu32StepAddr = NULL;
    HI_U32 u32SegIdx = pstInputDataIdx->u32SegIdx;
    HI_U32 u32NodeIdx = pstInputDataIdx->u32NodeIdx;
    HI_U32 u32TotalStepNum = 0;
	HI_U8 *pu8BGR = HI_NULL;
	HI_U8 *pu8YUV = HI_NULL;
	HI_BOOL bRBG2BGR = HI_TRUE;
	
	// RBG => BGR
	#define B_BASE_OFFSET (1*u32Stride*u32Height)
	#define G_BASE_OFFSET (2*u32Stride*u32Height)
	#define R_BASE_OFFSET (0*u32Stride*u32Height)
	HI_U8 *p_B_data = HI_NULL; 
	HI_U8 *p_G_data = HI_NULL;
	HI_U8 *p_R_data = HI_NULL;
								
								
    /*open file*/
    if (NULL != pstNnieCfg->pszPic)
    {
        fp = fopen(pstNnieCfg->pszPic,"rb");
        SAMPLE_SVP_CHECK_EXPR_RET(NULL == fp,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error, open file failed!\n");
    }
	else if(NULL != pstNnieCfg->pszBGR)
    {
	    pu8BGR = (HI_U8*)pstNnieCfg->pszBGR; 
	    //printf("pu8BGR = %p\n", pu8BGR);	
	}
	else if(NULL != pstNnieCfg->pszYUV)
    {
	    pu8YUV = (HI_U8*)pstNnieCfg->pszYUV; 
	    //printf("pu8YUV = %p\n", pu8YUV);	
	}

    /*get data size*/
    if(SVP_BLOB_TYPE_U8 <= pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType &&
        SVP_BLOB_TYPE_YVU422SP >= pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
    {
        u32VarSize = sizeof(HI_U8);
    }
    else
    {
        u32VarSize = sizeof(HI_U32);
    }

    /*fill src data*/
    if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
    {
        u32Dim = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stSeq.u32Dim;
        u32Stride = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Stride;
        pu32StepAddr = (HI_U32*)(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stSeq.u64VirAddrStep);
        pu8PicAddr = (HI_U8*)(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr);
        for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
        {
            for(i = 0;i < *(pu32StepAddr+n); i++)
            {
                s32Ret = fread(pu8PicAddr,u32Dim*u32VarSize,1,fp);
                SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,Read image file failed!\n");
                pu8PicAddr += u32Stride;
            }
            u32TotalStepNum += *(pu32StepAddr+n);
        }
        SDC_FlushCache(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64PhyAddr,
            (HI_VOID *) pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr,
            u32TotalStepNum*u32Stride);
    }
    else
    {
        u32Height = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Height;
        u32Width = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Width;
        u32Chn = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Chn;
        u32Stride = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Stride;
        pu8PicAddr = (HI_U8*)(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr);
        if(SVP_BLOB_TYPE_YVU420SP== pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
        {
            for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
            {
                for(i = 0; i < u32Chn*u32Height/2; i++)
                {
					if(fp)
					{
						s32Ret = fread(pu8PicAddr,u32Width*u32VarSize,1,fp);
					}
					else
					{
						//printf("pu8YUV current = %p\n", pu8YUV);
						memcpy_s(pu8PicAddr, u32Width*u32VarSize, pu8YUV, u32Width*u32VarSize);
						//printf("pu8YUV current2 = %p\n", pu8YUV);
						pu8YUV += 304;
					}
                    //SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,Read image file failed!\n");
					//printf("u32Stride = %d\n", u32Stride);
                    pu8PicAddr += u32Stride;
                }
            }
        }
        else if(SVP_BLOB_TYPE_YVU422SP== pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType)
        {
            for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
            {
                for(i = 0; i < u32Height*2; i++)
                {
                    s32Ret = fread(pu8PicAddr,u32Width*u32VarSize,1,fp);
                    SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,Read image file failed!\n");
                    pu8PicAddr += u32Stride;
                }
            }
        }
        else
        {
			if(bRBG2BGR) // RBG => BGR
			{
				p_B_data = pu8BGR + B_BASE_OFFSET;
				p_G_data = pu8BGR + G_BASE_OFFSET;
				p_R_data = pu8BGR + R_BASE_OFFSET;
			}
            for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++)
            {
                for(i = 0;i < u32Chn; i++)
                {
                    for(j = 0; j < u32Height; j++)
                    {
						if(HI_NULL != fp)
						{  
							s32Ret = fread(pu8PicAddr,u32Width*u32VarSize,1,fp);
							SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,Read image file failed!\n");
						}
						else if(HI_NULL != pstNnieCfg->pszBGR)
						{
							//printf("u32Width*u32VarSize = %d\n", u32Width*u32VarSize);
							if(bRBG2BGR) // RBG => BGR
							{
							    if(u32Chn == 0) //copy B
								{
									memcpy_s(pu8PicAddr, u32Width*u32VarSize, p_B_data, u32Width*u32VarSize);  
									p_B_data += u32Stride;
								}
								else if(u32Chn == 1) //copy G
								{
									memcpy_s(pu8PicAddr, u32Width*u32VarSize, p_G_data, u32Width*u32VarSize);  
									p_G_data += u32Stride;
								}
								else // copy R
								{
									memcpy_s(pu8PicAddr, u32Width*u32VarSize, p_R_data, u32Width*u32VarSize);  
									p_R_data += u32Stride;
								}
							}
							else
							{
								memcpy_s(pu8PicAddr, u32Width*u32VarSize, pu8BGR, u32Width*u32VarSize);  
								pu8BGR += u32Stride;
							}
						}
						//pu8BGR += u32Width*u32VarSize;
						//printf("u32Stride = %d\n", u32Stride);
						pu8PicAddr += u32Stride;
                    }
                }
            }
        }
        SDC_FlushCache(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64PhyAddr,
            (HI_VOID *) pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr,
            pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num*u32Chn*u32Height*u32Stride);
    }
    if(fp != HI_NULL)
        fclose(fp);
    return HI_SUCCESS;

FAIL:
    if(fp != HI_NULL)
        fclose(fp);
    fclose(fp);
    return HI_FAILURE;
}




/******************************************************************************
* function : NNIE Forward
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Forward(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S* pstInputDataIdx,
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S* pstProcSegIdx,HI_BOOL bInstant)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0, j = 0;
    //HI_BOOL bFinish = HI_FALSE;
    //SVP_NNIE_HANDLE hSvpNnieHandle = 0;
    // HI_U32 u32TotalStepNum = 0;
    sdc_nnie_forward_s sdc_nnie_forward;

	#if 0
    SDC_FlushCache(pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u64PhyAddr,
        (HI_VOID *) pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u64VirAddr,
        pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u32Size);
	#endif

    /*set input blob according to node name*/
    if(pstInputDataIdx->u32SegIdx != pstProcSegIdx->u32SegIdx)
    {
        for(i = 0; i < pstNnieParam->pstModel->astSeg[pstProcSegIdx->u32SegIdx].u16SrcNum; i++)
        {
            for(j = 0; j < pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].u16DstNum; j++)
            {
                if(0 == strncmp(pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].astDstNode[j].szName,
                    pstNnieParam->pstModel->astSeg[pstProcSegIdx->u32SegIdx].astSrcNode[i].szName,
                    SVP_NNIE_NODE_NAME_LEN))
                {
                    pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc[i] =
                        pstNnieParam->astSegData[pstInputDataIdx->u32SegIdx].astDst[j];
                    break;
                }
            }
            SAMPLE_SVP_CHECK_EXPR_RET((j == pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].u16DstNum),
                HI_FAILURE,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,can't find %d-th seg's %d-th src blob!\n",
                pstProcSegIdx->u32SegIdx,i);
        }
    }

    /*NNIE_Forward*/


    
    memcpy(&sdc_nnie_forward.model, pstNnieParam->pstModel,  sizeof(SVP_NNIE_MODEL_S));
    sdc_nnie_forward.forward_ctrl.max_batch_num = 1;
	sdc_nnie_forward.forward_ctrl.max_bbox_num = 0;/*此处需要根据算法模型的ROI个数决定，max_bbox_num = max_roi_num(ROI个数)*/
    sdc_nnie_forward.forward_ctrl.netseg_id = pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].u32NetSegId;
    
    memcpy(sdc_nnie_forward.astSrc, pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc,  16*sizeof(SVP_DST_BLOB_S));
    memcpy(sdc_nnie_forward.astDst, pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst,  16*sizeof(SVP_DST_BLOB_S));



/*目前NNIE的服务化接口有一个BUG，最大max_bbox_num有限制，最新版本会修复使用服务化接口*/
#if 1
    s32Ret = SDC_Nnie_Forward(&sdc_nnie_forward);
    if(s32Ret != OK) 
    {
		fprintf(stderr, "Err in SDC_Nnie_Forward, s32Ret: %d\n", s32Ret);
		return ERR;
	}
#else
    
    /*NNIE_Forward*/
    s32Ret = HI_MPI_SVP_NNIE_Forward(&hSvpNnieHandle,
        pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc,
        pstNnieParam->pstModel, pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst,
        &pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx], bInstant);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,HI_MPI_SVP_NNIE_Forward failed!\n");

    if(bInstant)
    {
        /*Wait NNIE finish*/
        while(HI_ERR_SVP_NNIE_QUERY_TIMEOUT == (s32Ret = HI_MPI_SVP_NNIE_Query(pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].enNnieId,
            hSvpNnieHandle, &bFinish, HI_TRUE)))
        {
            usleep(100);
            SAMPLE_SVP_TRACE(SAMPLE_SVP_ERR_LEVEL_INFO,
                "HI_MPI_SVP_NNIE_Query Query timeout!\n");
        }
    }


    //bFinish = HI_FALSE;
    for(i = 0; i < pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].u32DstNum; i++)
    {
        if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].enType)
        {
            for(j = 0; j < pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num; j++)
            {
                u32TotalStepNum += *((HI_U32*)(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stSeq.u64VirAddrStep)+j);
            }
            SDC_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                (HI_VOID *) pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr,
                u32TotalStepNum*pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);

        }
        else
        {

            SDC_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                (HI_VOID *) pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr,
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Chn*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Height*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);
        }
    }

#endif

    return s32Ret;
}




int SDC_SVP_ForwardBGR(HI_CHAR *pcSrcBGR, SDC_SSD_RESULT_S *pstResult, SDC_SSD_INPUT_SIZE_S InputSize)
{
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};

    /*Set configuration parameter*/
    stNnieCfg_SDC.pszPic = HI_NULL;
    stNnieCfg_SDC.pszYUV = HI_NULL;
    stNnieCfg_SDC.pszBGR = pcSrcBGR;

    /*Fill src data*/
  //  SAMPLE_SVP_TRACE_INFO("Ssd start!\n");
    stInputDataIdx.u32SegIdx = 0;
    stInputDataIdx.u32NodeIdx = 0;
    s32Ret = SAMPLE_SVP_NNIE_FillSrcData(&stNnieCfg_SDC,&s_stYoloNnieParam,&stInputDataIdx);

    stProcSegIdx.u32SegIdx = 0;
    s32Ret = SAMPLE_SVP_NNIE_Forward(&s_stYoloNnieParam,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
   // printf("SAMPLE_SVP_NNIE_Forward s32Ret = 0x%x\n", s32Ret);

    /*software process*/
    /*if user has changed net struct, please make sure SAMPLE_SVP_NNIE_Ssd_GetResult
     function's input datas are correct*/
    // s32Ret = SAMPLE_SVP_NNIE_Ssd_GetResult(&s_stSsdNnieParam,&s_stSsdSoftwareParam);
    s32Ret = SAMPLE_SVP_NNIE_Yolov3_GetResult(&s_stYoloNnieParam,&s_stYolov3SoftwareParam);
    // printf("SAMPLE_SVP_NNIE_Ssd_GetResult s32Ret = %d\n", s32Ret);
    // printf("\nnumclass====%d\n",s_stYolov3SoftwareParam.stClassRoiNum.unShape.stWhc.u32Width);
    

    /*print result, this sample has 21 classes:
     class 0:background     class 1:plane           class 2:bicycle
     class 3:bird           class 4:boat            class 5:bottle
     class 6:bus            class 7:car             class 8:cat
     class 9:chair          class10:cow             class11:diningtable
     class 12:dog           class13:horse           class14:motorbike
     class 15:person        class16:pottedplant     class17:sheep
     class 18:sofa          class19:train           class20:tvmonitor*/

    // HI_S32* ps32ClassRoiNum = (HI_S32*)s_stYolov3SoftwareParam.stClassRoiNum.u64PhyAddr;
    // HI_S32* ps32ClassRoiNum = (HI_S32*)s_stYolov3SoftwareParam.stClassRoiNum.u64PhyAddr;
    // printf("\naaa ===%d\n",*(s_stYolov3SoftwareParam.stClassRoiNum.u64PhyAddr-1));
    // printf("\naaa ===%d\n",ps32ClassRoiNum[2]);

    SDC_SVP_NNIE_Detection_GetResult(&s_stYolov3SoftwareParam.stDstScore,
        &s_stYolov3SoftwareParam.stDstRoi, &s_stYolov3SoftwareParam.stClassRoiNum, pstResult);
	return s32Ret;
}




/******************************************************************************
* function : Yolov3 software deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Yolov3_SoftwareDeinit(SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam, int fd)
{
    HI_S32 s32Ret = HI_SUCCESS;
	sdc_mmz_alloc_s stMemParas;
    SAMPLE_SVP_CHECK_EXPR_RET(NULL== pstSoftWareParam,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error, pstSoftWareParam can't be NULL!\n");
    if(0!=pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr && 0!=pstSoftWareParam->stGetResultTmpBuf.u64VirAddr)
    {
		stMemParas.addr_phy = pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr;
		stMemParas.addr_virt = pstSoftWareParam->stGetResultTmpBuf.u64VirAddr;

        SDC_MemFree(fd, &stMemParas);
		
        pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr = 0;
        pstSoftWareParam->stGetResultTmpBuf.u64VirAddr = 0;
        pstSoftWareParam->stDstRoi.u64PhyAddr = 0;
        pstSoftWareParam->stDstRoi.u64VirAddr = 0;
        pstSoftWareParam->stDstScore.u64PhyAddr = 0;
        pstSoftWareParam->stDstScore.u64VirAddr = 0;
        pstSoftWareParam->stClassRoiNum.u64PhyAddr = 0;
        pstSoftWareParam->stClassRoiNum.u64VirAddr = 0;
    }
    return s32Ret;
}


/*****************************************************************************
*   Prototype    : SDC_NNIE_ParamDeinit
*   Description  : Deinit NNIE parameters
*   Input        : SAMPLE_SVP_NNIE_PARAM_S        *pstNnieParam     NNIE Parameter
*                  SAMPLE_SVP_NNIE_SOFTWARE_MEM_S *pstSoftWareMem   software mem
*
*
*
*
*   Output       :
*   Return Value :  HI_S32,HI_SUCCESS:Success,Other:failure
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-20
*           Author       :
*           Modification : Create
*
*****************************************************************************/
HI_S32 SDC_NNIE_ParamDeinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, int fd)
{
    sdc_mmz_alloc_s stMemParas;
	SAMPLE_SVP_CHECK_EXPR_RET(NULL == pstNnieParam,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error, pstNnieParam can't be NULL!\n");

	#if 0
	if(0!=pstNnieParam->stTaskBuf.u64PhyAddr && 0!=pstNnieParam->stTaskBuf.u64VirAddr)
	{
		SAMPLE_SVP_MMZ_FREE(pstNnieParam->stTaskBuf.u64PhyAddr,pstNnieParam->stTaskBuf.u64VirAddr);
		pstNnieParam->stTaskBuf.u64PhyAddr = 0;
		pstNnieParam->stTaskBuf.u64VirAddr = 0;
	}
	#endif
	
    if(0!=pstNnieParam->stStepBuf.u64PhyAddr && 0!=pstNnieParam->stStepBuf.u64VirAddr)
	{
		stMemParas.addr_phy = pstNnieParam->stStepBuf.u64PhyAddr;
		stMemParas.addr_virt = pstNnieParam->stStepBuf.u64VirAddr;

        SDC_MemFree(fd, &stMemParas);
		
		pstNnieParam->stStepBuf.u64PhyAddr = 0;
		pstNnieParam->stStepBuf.u64VirAddr = 0;
	}
	return HI_SUCCESS;
}


/******************************************************************************
* function : Yolov3 Deinit
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Yolov3_Deinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
    SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam,SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel, int fd)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*hardware deinit*/
    if(pstNnieParam!=NULL)
    {
        s32Ret = SDC_NNIE_ParamDeinit(pstNnieParam, fd);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error,SDC_NNIE_ParamDeinit failed!\n");
    }
    /*software deinit*/
    if(pstSoftWareParam!=NULL)
    {
        s32Ret = SAMPLE_SVP_NNIE_Yolov3_SoftwareDeinit(pstSoftWareParam, fd);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error,SAMPLE_SVP_NNIE_Yolov3_SoftwareDeinit failed!\n");
    }
    /*model deinit*/
    if(pstNnieModel!=NULL)
    {
        s32Ret = SDC_UnLoadModel(&pstNnieModel->stModel);
        if (s32Ret != OK)
        {
            fprintf(stdout,"Err in SDC_UnLoadModel\n");
        }
    }
    return s32Ret;
}



/*****************************************************************************
* Prototype :   SAMPLE_SVP_NNIE_Yolov3_GetResultTmpBuf
* Description : this function is used to Get Yolov3 GetResult tmp buffer size
* Input :      SAMPLE_SVP_NNIE_PARAM_S*               pstNnieParam     [IN]  the pointer to YOLOV3 NNIE parameter
*              SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S*   pstSoftwareParam [IN]  the pointer to YOLOV3 software parameter
*
*
*
*
* Output :
* Return Value : HI_U32: tmp buffer size.
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-11-10
* Author :
* Modification : Create
*
*****************************************************************************/
HI_U32 SAMPLE_SVP_NNIE_Yolov3_GetResultTmpBuf(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,
    SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftwareParam)
{
    HI_U32 u32TotalSize = 0;
    HI_U32 u32AssistStackSize = 0;
    HI_U32 u32TotalBboxNum = 0;
    HI_U32 u32TotalBboxSize = 0;
    HI_U32 u32DstBlobSize = 0;
    HI_U32 u32MaxBlobSize = 0;
    HI_U32 i = 0;

    for(i = 0; i < pstNnieParam->pstModel->astSeg[0].u16DstNum; i++)
    {
        u32DstBlobSize = pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Width*sizeof(HI_U32)*
            pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Height*
            pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Chn;
        if(u32MaxBlobSize < u32DstBlobSize)
        {
            u32MaxBlobSize = u32DstBlobSize;
        }
        u32TotalBboxNum += pstSoftwareParam->au32GridNumWidth[i]*pstSoftwareParam->au32GridNumHeight[i]*
            pstSoftwareParam->u32BboxNumEachGrid;
    }
    u32AssistStackSize = u32TotalBboxNum*sizeof(SAMPLE_SVP_NNIE_STACK_S);
    u32TotalBboxSize = u32TotalBboxNum*sizeof(SAMPLE_SVP_NNIE_YOLOV3_BBOX_S);
    u32TotalSize += (u32MaxBlobSize+u32AssistStackSize+u32TotalBboxSize);

    return u32TotalSize;
}



/******************************************************************************
* function : Yolov3 software para init
******************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_Yolov3_SoftwareInit(int fd, SAMPLE_SVP_NNIE_CFG_S* pstCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ClassNum = 0;
    HI_U32 u32TotalSize = 0;
    HI_U32 u32DstRoiSize = 0;
    HI_U32 u32DstScoreSize = 0;
    HI_U32 u32ClassRoiNumSize = 0;
    HI_U32 u32TmpBufTotalSize = 0;
    HI_U64 u64PhyAddr = 0;
    HI_U8* pu8VirAddr = NULL;
    sdc_mmz_alloc_s stMemParas;
	

    pstSoftWareParam->u32OriImHeight = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height;
    pstSoftWareParam->u32OriImWidth = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width;
    pstSoftWareParam->u32BboxNumEachGrid = 3;
    //pstSoftWareParam->u32ClassNum = 80;
    pstSoftWareParam->u32ClassNum = 1;
    pstSoftWareParam->au32GridNumHeight[0] = 13;
    pstSoftWareParam->au32GridNumHeight[1] = 26;
    pstSoftWareParam->au32GridNumHeight[2] = 52;
    pstSoftWareParam->au32GridNumWidth[0] = 13;
    pstSoftWareParam->au32GridNumWidth[1] = 26;
    pstSoftWareParam->au32GridNumWidth[2] = 52;
    pstSoftWareParam->u32NmsThresh = (HI_U32)(0.3f*SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->u32ConfThresh = (HI_U32)(0.5f*SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->u32MaxRoiNum = 10;
    pstSoftWareParam->af32Bias[0][0] = 116;
    pstSoftWareParam->af32Bias[0][1] = 90;
    pstSoftWareParam->af32Bias[0][2] = 156;
    pstSoftWareParam->af32Bias[0][3] = 198;
    pstSoftWareParam->af32Bias[0][4] = 373;
    pstSoftWareParam->af32Bias[0][5] = 326;
    pstSoftWareParam->af32Bias[1][0] = 30;
    pstSoftWareParam->af32Bias[1][1] = 61;
    pstSoftWareParam->af32Bias[1][2] = 62;
    pstSoftWareParam->af32Bias[1][3] = 45;
    pstSoftWareParam->af32Bias[1][4] = 59;
    pstSoftWareParam->af32Bias[1][5] = 119;
    pstSoftWareParam->af32Bias[2][0] = 10;
    pstSoftWareParam->af32Bias[2][1] = 13;
    pstSoftWareParam->af32Bias[2][2] = 16;
    pstSoftWareParam->af32Bias[2][3] = 30;
    pstSoftWareParam->af32Bias[2][4] = 33;
    pstSoftWareParam->af32Bias[2][5] = 23;

    /*Malloc assist buffer memory*/
    u32ClassNum = pstSoftWareParam->u32ClassNum+1;

    SAMPLE_SVP_CHECK_EXPR_RET(SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM != pstNnieParam->pstModel->astSeg[0].u16DstNum,
        HI_FAILURE,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,pstNnieParam->pstModel->astSeg[0].u16DstNum(%d) should be %d!\n",
        pstNnieParam->pstModel->astSeg[0].u16DstNum,SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM);
    u32TmpBufTotalSize = SAMPLE_SVP_NNIE_Yolov3_GetResultTmpBuf(pstNnieParam,pstSoftWareParam);
    u32DstRoiSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstSoftWareParam->u32MaxRoiNum*sizeof(HI_U32)*SAMPLE_SVP_NNIE_COORDI_NUM);
    u32DstScoreSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstSoftWareParam->u32MaxRoiNum*sizeof(HI_U32));
    u32ClassRoiNumSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    u32TotalSize = u32TotalSize+u32DstRoiSize+u32DstScoreSize+u32ClassRoiNumSize+u32TmpBufTotalSize;

    s32Ret = SDC_MemAlloc(fd,u32TotalSize,1,&stMemParas);
    u64PhyAddr = stMemParas.addr_phy;
    pu8VirAddr = (void *)stMemParas.addr_virt;
    u32TotalSize = stMemParas.size;

	
	//s32Ret = SAMPLE_COMM_SVP_MallocCached("SAMPLE_YOLOV3_INIT",NULL,(HI_U64*)&u64PhyAddr,
        //(void**)&pu8VirAddr,u32TotalSize);
    SAMPLE_SVP_CHECK_EXPR_RET(u32TotalSize != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,Malloc memory failed!\n");
    memset(pu8VirAddr,0, u32TotalSize);
    SDC_FlushCache(u64PhyAddr,(void*)pu8VirAddr,u32TotalSize);

   /*set each tmp buffer addr*/
    pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr = u64PhyAddr;
    pstSoftWareParam->stGetResultTmpBuf.u64VirAddr = (HI_U64)(pu8VirAddr);

    /*set result blob*/
    pstSoftWareParam->stDstRoi.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstRoi.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize;
    pstSoftWareParam->stDstRoi.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize);
    pstSoftWareParam->stDstRoi.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*
        pstSoftWareParam->u32MaxRoiNum*sizeof(HI_U32)*SAMPLE_SVP_NNIE_COORDI_NUM);
    pstSoftWareParam->stDstRoi.u32Num = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Width = u32ClassNum*
        pstSoftWareParam->u32MaxRoiNum*SAMPLE_SVP_NNIE_COORDI_NUM;

    pstSoftWareParam->stDstScore.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstScore.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize+u32DstRoiSize;
    pstSoftWareParam->stDstScore.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize+u32DstRoiSize);
    pstSoftWareParam->stDstScore.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*
        pstSoftWareParam->u32MaxRoiNum*sizeof(HI_U32));
    pstSoftWareParam->stDstScore.u32Num = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Width = u32ClassNum*pstSoftWareParam->u32MaxRoiNum;

    pstSoftWareParam->stClassRoiNum.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stClassRoiNum.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize+
        u32DstRoiSize+u32DstScoreSize;
    pstSoftWareParam->stClassRoiNum.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize+
        u32DstRoiSize+u32DstScoreSize);
    pstSoftWareParam->stClassRoiNum.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    pstSoftWareParam->stClassRoiNum.u32Num = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Width = u32ClassNum;

    return HI_SUCCESS;
}




/*****************************************************************************
*   Prototype    : SAMPLE_SVP_NNIE_GetBlobMemSize
*   Description  : Get blob mem size
*   Input        : SVP_NNIE_NODE_S astNnieNode[]   NNIE Node
*                  HI_U32          u32NodeNum      Node num
*                  HI_U32          astBlob[]       blob struct
*                  HI_U32          u32Align        stride align type
*                  HI_U32          *pu32TotalSize  Total size
*                  HI_U32          au32BlobSize[]  blob size
*
*
*
*
*   Output       :
*   Return Value : VOID
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-20
*           Author       :
*           Modification : Create
*
*****************************************************************************/
static void SAMPLE_SVP_NNIE_GetBlobMemSize(SVP_NNIE_NODE_S astNnieNode[], HI_U32 u32NodeNum,
	HI_U32 u32TotalStep,SVP_BLOB_S astBlob[], HI_U32 u32Align, HI_U32* pu32TotalSize,HI_U32 au32BlobSize[])
{
	HI_U32 i = 0;
	HI_U32 u32Size = 0;
	HI_U32 u32Stride = 0;

	for(i = 0; i < u32NodeNum; i++)
	{
		if(SVP_BLOB_TYPE_S32== astNnieNode[i].enType||SVP_BLOB_TYPE_VEC_S32== astNnieNode[i].enType||
            SVP_BLOB_TYPE_SEQ_S32== astNnieNode[i].enType)
		{
			u32Size = sizeof(HI_U32);
		}
		else
		{
			u32Size = sizeof(HI_U8);
		}
        if(SVP_BLOB_TYPE_SEQ_S32 == astNnieNode[i].enType)
        {
            if(SAMPLE_SVP_NNIE_ALIGN_16 == u32Align)
    		{
    			u32Stride = SAMPLE_SVP_NNIE_ALIGN16(astNnieNode[i].unShape.u32Dim*u32Size);
    		}
    		else
    		{
    			u32Stride = SAMPLE_SVP_NNIE_ALIGN32(astNnieNode[i].unShape.u32Dim*u32Size);
    		}
            au32BlobSize[i] = u32TotalStep*u32Stride;
        }
        else
        {
            if(SAMPLE_SVP_NNIE_ALIGN_16 == u32Align)
    		{
    			u32Stride = SAMPLE_SVP_NNIE_ALIGN16(astNnieNode[i].unShape.stWhc.u32Width*u32Size);
    		}
    		else
    		{
    			u32Stride = SAMPLE_SVP_NNIE_ALIGN32(astNnieNode[i].unShape.stWhc.u32Width*u32Size);
    		}
    		au32BlobSize[i] = astBlob[i].u32Num*u32Stride*astNnieNode[i].unShape.stWhc.u32Height*
    			astNnieNode[i].unShape.stWhc.u32Chn;
        }
		*pu32TotalSize += au32BlobSize[i];
	    astBlob[i].u32Stride = u32Stride;
	}
}

/*****************************************************************************
*   Prototype    : SAMPLE_SVP_NNIE_GetTaskAndBlobBufSize
*   Description  : Get taskinfo and blob memory size
*   Input        : SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam     NNIE parameter
* 	                HI_U32                  *pu32TaskInfoSize Task info size
*                  HI_U32                  *pu32TmpBufSize    Tmp buffer size
*                  SAMPLE_SVP_NNIE_BLOB_SIZE_S  astBlobSize[] each seg input and output blob mem size
*                  HI_U32                  *pu32TotalSize     Total mem size
*
*
*   Output       :
*   Return Value : HI_S32,HI_SUCCESS:Success,Other:failure
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-20
*           Author       :
*           Modification : Create
*
*****************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_GetTaskAndBlobBufSize(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,HI_U32*pu32TotalTaskBufSize, HI_U32*pu32TmpBufSize,
    SAMPLE_SVP_NNIE_BLOB_SIZE_S astBlobSize[],HI_U32*pu32TotalSize)
{
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 i = 0, j = 0;
    HI_U32 u32TotalStep = 0;

	#if 0
	/*Get each seg's task buf size*/
	s32Ret = HI_MPI_SVP_NNIE_GetTskBufSize(pstNnieCfg->u32MaxInputNum, pstNnieCfg->u32MaxRoiNum,
		pstNnieParam->pstModel, pstNnieParam->au32TaskBufSize,pstNnieParam->pstModel->u32NetSegNum);
	SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error,HI_MPI_SVP_NNIE_GetTaskSize failed!\n");

    /*Get total task buf size*/
	*pu32TotalTaskBufSize = 0;
	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++)
	{
		*pu32TotalTaskBufSize += pstNnieParam->au32TaskBufSize[i];
	}

	/*Get tmp buf size*/
	*pu32TmpBufSize = pstNnieParam->pstModel->u32TmpBufSize;
	*pu32TotalSize += *pu32TotalTaskBufSize + *pu32TmpBufSize;
	#endif

	/*calculate Blob mem size*/
	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++)
	{
        if(SVP_NNIE_NET_TYPE_RECURRENT == pstNnieParam->pstModel->astSeg[i].enNetType)
        {
            for(j = 0; j < pstNnieParam->astSegData[i].astSrc[0].u32Num; j++)
            {
                u32TotalStep += *((HI_S32*)pstNnieParam->astSegData[i].astSrc[0].unShape.stSeq.u64VirAddrStep+j);
            }
        }
		/*the first seg's Src Blob mem size, other seg's src blobs from the output blobs of
		those segs before it or from software output results*/
		if(i == 0)
		{
			SAMPLE_SVP_NNIE_GetBlobMemSize(&(pstNnieParam->pstModel->astSeg[i].astSrcNode[0]),
				pstNnieParam->pstModel->astSeg[i].u16SrcNum,u32TotalStep,&(pstNnieParam->astSegData[i].astSrc[0]),
				SAMPLE_SVP_NNIE_ALIGN_16, pu32TotalSize, &(astBlobSize[i].au32SrcSize[0]));
		}

		/*Get each seg's Dst Blob mem size*/
		SAMPLE_SVP_NNIE_GetBlobMemSize(&(pstNnieParam->pstModel->astSeg[i].astDstNode[0]),
			pstNnieParam->pstModel->astSeg[i].u16DstNum,u32TotalStep,&(pstNnieParam->astSegData[i].astDst[0]),
			SAMPLE_SVP_NNIE_ALIGN_16, pu32TotalSize, &(astBlobSize[i].au32DstSize[0]));
	}
	return s32Ret;
}

/*****************************************************************************
*   Prototype    : SAMPLE_SVP_NNIE_FillForwardInfo
*   Description  : fill NNIE forward ctrl information
*   Input        : SAMPLE_SVP_NNIE_CFG_S   *pstNnieCfg       NNIE configure info
* 	               SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam     NNIE parameter
*
*
*
*   Output       :
*   Return Value : HI_S32,HI_SUCCESS:Success,Other:failure
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-20
*           Author       :
*           Modification : Create
*
*****************************************************************************/
static HI_S32 SAMPLE_SVP_NNIE_FillForwardInfo(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam)
{
	HI_U32 i = 0, j = 0;
	HI_U32 u32Offset = 0;
	HI_U32 u32Num = 0;

	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++)
	{
        /*fill forwardCtrl info*/
		if(SVP_NNIE_NET_TYPE_ROI == pstNnieParam->pstModel->astSeg[i].enNetType)
		{
			pstNnieParam->astForwardWithBboxCtrl[i].enNnieId = pstNnieCfg->aenNnieCoreId[i];
			pstNnieParam->astForwardWithBboxCtrl[i].u32SrcNum = pstNnieParam->pstModel->astSeg[i].u16SrcNum;
			pstNnieParam->astForwardWithBboxCtrl[i].u32DstNum = pstNnieParam->pstModel->astSeg[i].u16DstNum;
			pstNnieParam->astForwardWithBboxCtrl[i].u32ProposalNum = 1;
			pstNnieParam->astForwardWithBboxCtrl[i].u32NetSegId = i;
			#if 0
			pstNnieParam->astForwardWithBboxCtrl[i].stTmpBuf = pstNnieParam->stTmpBuf;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u64PhyAddr= pstNnieParam->stTaskBuf.u64PhyAddr+u32Offset;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u64VirAddr= pstNnieParam->stTaskBuf.u64VirAddr+u32Offset;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u32Size= pstNnieParam->au32TaskBufSize[i];
			#endif
		}
		else if(SVP_NNIE_NET_TYPE_CNN == pstNnieParam->pstModel->astSeg[i].enNetType ||
            SVP_NNIE_NET_TYPE_RECURRENT== pstNnieParam->pstModel->astSeg[i].enNetType)
		{


			pstNnieParam->astForwardCtrl[i].enNnieId = pstNnieCfg->aenNnieCoreId[i];
			pstNnieParam->astForwardCtrl[i].u32SrcNum = pstNnieParam->pstModel->astSeg[i].u16SrcNum;
			pstNnieParam->astForwardCtrl[i].u32DstNum = pstNnieParam->pstModel->astSeg[i].u16DstNum;
			pstNnieParam->astForwardCtrl[i].u32NetSegId = i;
			#if 0
			pstNnieParam->astForwardCtrl[i].stTmpBuf = pstNnieParam->stTmpBuf;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u64PhyAddr= pstNnieParam->stTaskBuf.u64PhyAddr+u32Offset;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u64VirAddr= pstNnieParam->stTaskBuf.u64VirAddr+u32Offset;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u32Size= pstNnieParam->au32TaskBufSize[i];
			#endif
		}
		u32Offset += pstNnieParam->au32TaskBufSize[i];/*这里赋值要审视下，好像参数没用*/

        /*fill src blob info*/
		for(j = 0; j < pstNnieParam->pstModel->astSeg[i].u16SrcNum; j++)
	    {
            /*Recurrent blob*/
            if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->pstModel->astSeg[i].astSrcNode[j].enType)
            {
                pstNnieParam->astSegData[i].astSrc[j].enType = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].enType;
                pstNnieParam->astSegData[i].astSrc[j].unShape.stSeq.u32Dim = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.u32Dim;
                pstNnieParam->astSegData[i].astSrc[j].u32Num = pstNnieCfg->u32MaxInputNum;
                pstNnieParam->astSegData[i].astSrc[j].unShape.stSeq.u64VirAddrStep = pstNnieCfg->au64StepVirAddr[i*SAMPLE_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM];
            }
            else
            {
    		    pstNnieParam->astSegData[i].astSrc[j].enType = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].enType;
    	        pstNnieParam->astSegData[i].astSrc[j].unShape.stWhc.u32Chn = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.stWhc.u32Chn;
    	        pstNnieParam->astSegData[i].astSrc[j].unShape.stWhc.u32Height = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.stWhc.u32Height;
    	        pstNnieParam->astSegData[i].astSrc[j].unShape.stWhc.u32Width = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.stWhc.u32Width;
    	        pstNnieParam->astSegData[i].astSrc[j].u32Num = pstNnieCfg->u32MaxInputNum;
            }
	    }

        /*fill dst blob info*/
		if(SVP_NNIE_NET_TYPE_ROI == pstNnieParam->pstModel->astSeg[i].enNetType)
		{
			u32Num = pstNnieCfg->u32MaxRoiNum*pstNnieCfg->u32MaxInputNum;
		}
		else
		{
			u32Num = pstNnieCfg->u32MaxInputNum;
		}

		for(j = 0; j < pstNnieParam->pstModel->astSeg[i].u16DstNum; j++)
		{
            if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->pstModel->astSeg[i].astDstNode[j].enType)
            {
    			pstNnieParam->astSegData[i].astDst[j].enType = pstNnieParam->pstModel->astSeg[i].astDstNode[j].enType;
    			pstNnieParam->astSegData[i].astDst[j].unShape.stSeq.u32Dim =
                    pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.u32Dim;
                pstNnieParam->astSegData[i].astDst[j].u32Num = u32Num;
                pstNnieParam->astSegData[i].astDst[j].unShape.stSeq.u64VirAddrStep =
                    pstNnieCfg->au64StepVirAddr[i*SAMPLE_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM+1];
            }
            else
            {
    		    pstNnieParam->astSegData[i].astDst[j].enType = pstNnieParam->pstModel->astSeg[i].astDstNode[j].enType;
    		    pstNnieParam->astSegData[i].astDst[j].unShape.stWhc.u32Chn = pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.stWhc.u32Chn;
    		    pstNnieParam->astSegData[i].astDst[j].unShape.stWhc.u32Height = pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.stWhc.u32Height;
    		    pstNnieParam->astSegData[i].astDst[j].unShape.stWhc.u32Width = pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.stWhc.u32Width;
    		    pstNnieParam->astSegData[i].astDst[j].u32Num = u32Num;
            }
		}
	}
	return HI_SUCCESS;
}


/*****************************************************************************
*   Prototype    : SAMPLE_SVP_NNIE_ParamInit
*   Description  : Fill info of NNIE Forward parameters
*   Input        : SAMPLE_SVP_NNIE_CFG_S   *pstNnieCfg    NNIE configure parameter
* 		            SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam	 NNIE parameters
*
*
*
*   Output       :
*   Return Value : HI_S32,HI_SUCCESS:Success,Other:failure
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-03-14
*           Author       :
*           Modification : Create
*
*****************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_ParamInit(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam)
{
	HI_U32 i = 0, j = 0;
	HI_U32 u32TotalSize = 0;
	HI_U32 u32TotalTaskBufSize = 0;
	HI_U32 u32TmpBufSize = 0;
	HI_S32 s32Ret = HI_SUCCESS;
	//HI_U32 u32Offset = 0;
	HI_U64 u64PhyAddr = 0;
	HI_U8 *pu8VirAddr = NULL;
	SAMPLE_SVP_NNIE_BLOB_SIZE_S astBlobSize[SVP_NNIE_MAX_NET_SEG_NUM] = {0};
    sdc_mmz_alloc_s stMemParas;

	/*fill forward info*/
	s32Ret = SAMPLE_SVP_NNIE_FillForwardInfo(pstNnieCfg,pstNnieParam);
	SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error,SAMPLE_SVP_NNIE_FillForwardCtrl failed!\n");

    #if 1
	/*Get taskInfo and Blob mem size*/
	s32Ret = SAMPLE_SVP_NNIE_GetTaskAndBlobBufSize(pstNnieCfg,pstNnieParam,&u32TotalTaskBufSize,
		&u32TmpBufSize,astBlobSize,&u32TotalSize);
	SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error,SAMPLE_SVP_NNIE_GetTaskAndBlobBufSize failed!\n");
  

	/*Malloc mem*/

    s32Ret = SDC_MemAlloc(fd_utils,u32TotalSize,1,&stMemParas);
    u64PhyAddr = stMemParas.addr_phy;
    pu8VirAddr = (void *)stMemParas.addr_virt;

	fprintf(stdout,"SDC_MemAlloc u32TotalSize %d, size %d", u32TotalSize, stMemParas.size);

	
    u32TotalSize = stMemParas.size;

    #if 0
	s32Ret = SAMPLE_COMM_SVP_MallocCached("SAMPLE_NNIE_TASK",NULL,(HI_U64*)&u64PhyAddr,(void**)&pu8VirAddr,u32TotalSize);
    #endif

    SAMPLE_SVP_CHECK_EXPR_RET(u32TotalSize != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error,Malloc memory failed,u32TotalSize:%d,s32Ret:%d!\n", u32TotalSize, s32Ret);
	memset(pu8VirAddr, 0, u32TotalSize);
	SDC_FlushCache(u64PhyAddr,(void*)pu8VirAddr,u32TotalSize);
    #endif

	
    #if 0
	/*fill taskinfo mem addr*/
	pstNnieParam->stTaskBuf.u32Size = u32TotalTaskBufSize;
	pstNnieParam->stTaskBuf.u64PhyAddr = u64PhyAddr;
	pstNnieParam->stTaskBuf.u64VirAddr = (HI_U64)pu8VirAddr;

	/*fill Tmp mem addr*/
	pstNnieParam->stTmpBuf.u32Size = u32TmpBufSize;
	pstNnieParam->stTmpBuf.u64PhyAddr = u64PhyAddr+u32TotalTaskBufSize;
	pstNnieParam->stTmpBuf.u64VirAddr = (HI_U64)pu8VirAddr+u32TotalTaskBufSize;

	/*fill forward ctrl addr*/
	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++)
	{
		if(SVP_NNIE_NET_TYPE_ROI == pstNnieParam->pstModel->astSeg[i].enNetType)
		{
			pstNnieParam->astForwardWithBboxCtrl[i].stTmpBuf = pstNnieParam->stTmpBuf;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u64PhyAddr= pstNnieParam->stTaskBuf.u64PhyAddr+u32Offset;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u64VirAddr= pstNnieParam->stTaskBuf.u64VirAddr+u32Offset;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u32Size= pstNnieParam->au32TaskBufSize[i];
		}
		else if(SVP_NNIE_NET_TYPE_CNN == pstNnieParam->pstModel->astSeg[i].enNetType ||
            SVP_NNIE_NET_TYPE_RECURRENT == pstNnieParam->pstModel->astSeg[i].enNetType)
		{


			pstNnieParam->astForwardCtrl[i].stTmpBuf = pstNnieParam->stTmpBuf;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u64PhyAddr= pstNnieParam->stTaskBuf.u64PhyAddr+u32Offset;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u64VirAddr= pstNnieParam->stTaskBuf.u64VirAddr+u32Offset;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u32Size= pstNnieParam->au32TaskBufSize[i];
		}
		u32Offset += pstNnieParam->au32TaskBufSize[i];
	}
    #endif

	/*fill each blob's mem addr*/
	u64PhyAddr =  u64PhyAddr;//+u32TotalTaskBufSize+u32TmpBufSize;
	pu8VirAddr = pu8VirAddr;//+u32TotalTaskBufSize+u32TmpBufSize;
	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++)
	{
		/*first seg has src blobs, other seg's src blobs from the output blobs of
		those segs before it or from software output results*/
		if(0 == i)
		{
			for(j = 0; j < pstNnieParam->pstModel->astSeg[i].u16SrcNum; j++)
			{
				if(j!=0)
				{
					u64PhyAddr += astBlobSize[i].au32SrcSize[j-1];
					pu8VirAddr += astBlobSize[i].au32SrcSize[j-1];
				}
				pstNnieParam->astSegData[i].astSrc[j].u64PhyAddr = u64PhyAddr;
				pstNnieParam->astSegData[i].astSrc[j].u64VirAddr = (HI_U64)pu8VirAddr;
			}
			u64PhyAddr += astBlobSize[i].au32SrcSize[j-1];
			pu8VirAddr += astBlobSize[i].au32SrcSize[j-1];
		}

		/*fill the mem addrs of each seg's output blobs*/
		for(j = 0; j < pstNnieParam->pstModel->astSeg[i].u16DstNum; j++)
		{
			if(j!=0)
			{
				u64PhyAddr += astBlobSize[i].au32DstSize[j-1];
				pu8VirAddr += astBlobSize[i].au32DstSize[j-1];
			}
			pstNnieParam->astSegData[i].astDst[j].u64PhyAddr = u64PhyAddr;
			pstNnieParam->astSegData[i].astDst[j].u64VirAddr = (HI_U64)pu8VirAddr;
		}
		u64PhyAddr += astBlobSize[i].au32DstSize[j-1];
		pu8VirAddr += astBlobSize[i].au32DstSize[j-1];
	}
	return HI_SUCCESS;
}

/*****************************************************************************
*   Prototype    : SDC_NNIE_ParamInit
*   Description  : Init NNIE  parameters
*   Input        : SAMPLE_SVP_NNIE_CFG_S   *pstNnieCfg    NNIE configure parameter
*                  SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam    NNIE parameters
*
*
*
*   Output       :
*   Return Value : HI_S32,HI_SUCCESS:Success,Other:failure
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-20
*           Author       :
*           Modification : Create
*
*****************************************************************************/
HI_S32 SDC_NNIE_ParamInit(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, int fd)
{
	HI_S32 s32Ret = HI_SUCCESS;

    /*check*/
    SAMPLE_SVP_CHECK_EXPR_RET((NULL == pstNnieCfg || NULL == pstNnieParam),HI_ERR_SVP_NNIE_ILLEGAL_PARAM,
        SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,pstNnieCfg and pstNnieParam can't be NULL!\n");
    SAMPLE_SVP_CHECK_EXPR_RET((NULL == pstNnieParam->pstModel),HI_ERR_SVP_NNIE_ILLEGAL_PARAM,
        SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,pstNnieParam->pstModel can't be NULL!\n");

	/*NNIE parameter initialization */
	s32Ret = SAMPLE_SVP_NNIE_ParamInit(pstNnieCfg,pstNnieParam);
	SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error, SAMPLE_SVP_NNIE_ParamInit failed!\n");

	return s32Ret;
FAIL:
	s32Ret = SDC_NNIE_ParamDeinit(pstNnieParam, fd);
	SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error, SAMPLE_COMM_SVP_NNIE_ParamDeinit failed!\n");
	return HI_FAILURE;
}



/******************************************************************************
* function : Ssd init
******************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_Yolov3_ParamInit(int fd, SAMPLE_SVP_NNIE_CFG_S* pstCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SDC_NNIE_ParamInit(pstCfg,pstNnieParam,fd);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),SDC_NNIE_ParamInit failed!\n",s32Ret);

    /*init software para*/
    s32Ret = SAMPLE_SVP_NNIE_Yolov3_SoftwareInit(fd, pstCfg,pstNnieParam,
        pstSoftWareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),SAMPLE_SVP_NNIE_Yolov3_SoftwareInit failed!\n",s32Ret);

    return s32Ret;
INIT_FAIL_0:
    s32Ret = SAMPLE_SVP_NNIE_Yolov3_Deinit(pstNnieParam,pstSoftWareParam,NULL,fd);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error(%#x),SAMPLE_SVP_NNIE_Yolov3_Deinit failed!\n",s32Ret);
    return HI_FAILURE;

}

int SDC_TransYUV2RGB(int fd, sdc_yuv_frame_s *yuv, sdc_yuv_frame_s *rgb)
{
    sdc_common_head_s head;
    int nRet;
    struct iovec iov[2];

        
    // fill head struct 
    memset(&head, 0, sizeof(head));
    head.version = SDC_VERSION;
    head.url = 3;
    head.method = SDC_METHOD_CREATE;
    head.head_length = sizeof(head);
    head.content_length = sizeof(sdc_yuv_frame_s);

    iov[0].iov_base = &head;
    iov[0].iov_len = sizeof(head);
    iov[1].iov_base = yuv;
    iov[1].iov_len = sizeof(sdc_yuv_frame_s);

    // write request
    nRet = writev(fd_algorithm, iov, sizeof(iov)/sizeof(iov[0]));
    if (nRet < 0)
    {
        fprintf(stdout,"Error:failed SDC_TransYUV2RGB,nRet:%d!\n",nRet);
        return ERR;
    }
    // read response
    iov[1].iov_base = rgb;
    iov[1].iov_len = sizeof(sdc_yuv_frame_s);
    nRet = readv(fd_algorithm, iov, 2);
    if (head.code != SDC_CODE_200 || nRet < 0 || head.content_length != sizeof(sdc_yuv_frame_s))
    {
        fprintf(stdout,"Err:SDC_TransYUV2RGB,nRet:%d,rsp_head.code:%d!\n",
            nRet, head.code);
        return ERR;
    } 
    return OK;

}

void SDC_Struct2RGB(sdc_yuv_frame_s *pstSdcRGBFrame, VW_YUV_FRAME_S *pstRGBFrameData)
{
    pstRGBFrameData->enPixelFormat = PIXEL_FORMAT_RGB_888;
    pstRGBFrameData->pYuvImgAddr = (CHAR *)pstSdcRGBFrame->addr_virt;
    pstRGBFrameData->ulPhyAddr[0] = pstSdcRGBFrame->addr_phy;
    pstRGBFrameData->ulPhyAddr[1] = pstSdcRGBFrame->addr_phy + (pstSdcRGBFrame->height * pstSdcRGBFrame->stride);
    pstRGBFrameData->ulPhyAddr[2] = pstSdcRGBFrame->addr_phy + (pstSdcRGBFrame->height * pstSdcRGBFrame->stride)*2;

    pstRGBFrameData->ulVirAddr[0] = pstSdcRGBFrame->addr_virt;
    pstRGBFrameData->ulVirAddr[1] = pstSdcRGBFrame->addr_virt + (pstSdcRGBFrame->height * pstSdcRGBFrame->stride);
    pstRGBFrameData->ulVirAddr[2] = pstSdcRGBFrame->addr_virt + (pstSdcRGBFrame->height * pstSdcRGBFrame->stride)*2;

    pstRGBFrameData->uWidth = pstSdcRGBFrame->width;
    pstRGBFrameData->uHeight = pstSdcRGBFrame->height;
    pstRGBFrameData->uStride[0] = pstSdcRGBFrame->stride;
    pstRGBFrameData->uStride[1] = pstSdcRGBFrame->stride;
    pstRGBFrameData->uStride[2] = pstSdcRGBFrame->stride;
    pstRGBFrameData->uFrmSize = pstSdcRGBFrame->size;
    pstRGBFrameData->uPoolId = pstSdcRGBFrame->cookie[0];
    pstRGBFrameData->uVbBlk = pstSdcRGBFrame->cookie[1];
    return ;    
}

int SDC_TransYUV2RGBRelease(int fd, sdc_yuv_frame_s *rgb)
{
    sdc_common_head_s head;
    int nRet;
    struct iovec iov[2];

        
    // fill head struct 
    memset(&head, 0, sizeof(head));
    head.version = SDC_VERSION;
    head.url = 3;
    head.method = SDC_METHOD_DELETE;
    head.head_length = sizeof(head);
    head.content_length = sizeof(sdc_yuv_frame_s);

    iov[0].iov_base = &head;
    iov[0].iov_len = sizeof(head);
    iov[1].iov_base = rgb;
    iov[1].iov_len = sizeof(sdc_yuv_frame_s);

    // write request
    nRet = writev(fd_algorithm, iov, sizeof(iov)/sizeof(iov[0]));
    if (nRet < 0)
    {
        fprintf(stdout,"Error:failed SDC_TransYUV2RGBRelease,nRet:%d!\n",nRet);
        return ERR;
    }
    
    return OK;
}


static HI_FLOAT s_af32ExpCoef[10][16] = {
    {1.0f, 1.00024f, 1.00049f, 1.00073f, 1.00098f, 1.00122f, 1.00147f, 1.00171f, 1.00196f, 1.0022f, 1.00244f, 1.00269f, 1.00293f, 1.00318f, 1.00342f, 1.00367f},
    {1.0f, 1.00391f, 1.00784f, 1.01179f, 1.01575f, 1.01972f, 1.02371f, 1.02772f, 1.03174f, 1.03578f, 1.03984f, 1.04391f, 1.04799f, 1.05209f, 1.05621f, 1.06034f},
    {1.0f, 1.06449f, 1.13315f, 1.20623f, 1.28403f, 1.36684f, 1.45499f, 1.54883f, 1.64872f, 1.75505f, 1.86825f, 1.98874f, 2.117f, 2.25353f, 2.39888f, 2.55359f},
    {1.0f, 2.71828f, 7.38906f, 20.0855f, 54.5981f, 148.413f, 403.429f, 1096.63f, 2980.96f, 8103.08f, 22026.5f, 59874.1f, 162755.0f, 442413.0f, 1.2026e+006f, 3.26902e+006f},
    {1.0f, 8.88611e+006f, 7.8963e+013f, 7.01674e+020f, 6.23515e+027f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f},
    {1.0f, 0.999756f, 0.999512f, 0.999268f, 0.999024f, 0.99878f, 0.998536f, 0.998292f, 0.998049f, 0.997805f, 0.997562f, 0.997318f, 0.997075f, 0.996831f, 0.996588f, 0.996345f},
    {1.0f, 0.996101f, 0.992218f, 0.98835f, 0.984496f, 0.980658f, 0.976835f, 0.973027f, 0.969233f, 0.965455f, 0.961691f, 0.957941f, 0.954207f, 0.950487f, 0.946781f, 0.94309f},
    {1.0f, 0.939413f, 0.882497f, 0.829029f, 0.778801f, 0.731616f, 0.687289f, 0.645649f, 0.606531f, 0.569783f, 0.535261f, 0.502832f, 0.472367f, 0.443747f, 0.416862f, 0.391606f},
    {1.0f, 0.367879f, 0.135335f, 0.0497871f, 0.0183156f, 0.00673795f, 0.00247875f, 0.000911882f, 0.000335463f, 0.00012341f, 4.53999e-005f, 1.67017e-005f, 6.14421e-006f, 2.26033e-006f, 8.31529e-007f, 3.05902e-007f},
    {1.0f, 1.12535e-007f, 1.26642e-014f, 1.42516e-021f, 1.60381e-028f, 1.80485e-035f, 2.03048e-042f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
};

/*****************************************************************************
* Prototype :   SVP_NNIE_QuickExp
* Description : this function is used to quickly get exp result
* Input :     HI_S32    s32Value           [IN]   input value
*
*
*
*
* Output :
* Return Value : HI_FLOAT: output value.
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-11-10
* Author :
* Modification : Create
*
*****************************************************************************/
static HI_FLOAT SVP_NNIE_QuickExp( HI_S32 s32Value )
{
    if( s32Value & 0x80000000 )
    {
        s32Value = ~s32Value + 0x00000001;
        return s_af32ExpCoef[5][s32Value & 0x0000000F] * s_af32ExpCoef[6][(s32Value>>4) & 0x0000000F] * s_af32ExpCoef[7][(s32Value>>8) & 0x0000000F] * s_af32ExpCoef[8][(s32Value>>12) & 0x0000000F] * s_af32ExpCoef[9][(s32Value>>16) & 0x0000000F ];
    }
    else
    {
        return s_af32ExpCoef[0][s32Value & 0x0000000F] * s_af32ExpCoef[1][(s32Value>>4) & 0x0000000F] * s_af32ExpCoef[2][(s32Value>>8) & 0x0000000F] * s_af32ExpCoef[3][(s32Value>>12) & 0x0000000F] * s_af32ExpCoef[4][(s32Value>>16) & 0x0000000F ];
    }
}


/*****************************************************************************
* Prototype :   SVP_NNIE_SoftMax
* Description : this function is used to do softmax
* Input :     HI_FLOAT*         pf32Src           [IN]   the pointer to source data
*             HI_U32             u32Num           [IN]   the num of source data
*
*
*
*
* Output :
* Return Value : HI_SUCCESS: Success;Error codes: Failure.
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-11-10
* Author :
* Modification : Create
*
*****************************************************************************/
static HI_S32 SVP_NNIE_SoftMax( HI_FLOAT* pf32Src, HI_U32 u32Num)
{
    HI_FLOAT f32Max = 0;
    HI_FLOAT f32Sum = 0;
    HI_U32 i = 0;

    for(i = 0; i < u32Num; ++i)
    {
        if(f32Max < pf32Src[i])
        {
            f32Max = pf32Src[i];
        }
    }

    for(i = 0; i < u32Num; ++i)
    {
        pf32Src[i] = (HI_FLOAT)SVP_NNIE_QuickExp((HI_S32)((pf32Src[i] - f32Max)*SAMPLE_SVP_NNIE_QUANT_BASE));
        f32Sum += pf32Src[i];
    }

    for(i = 0; i < u32Num; ++i)
    {
        pf32Src[i] /= f32Sum;
    }
    return HI_SUCCESS;
}



/*****************************************************************************
*   Prototype    : SVP_NNIE_Yolov2_GetMaxVal
*   Description  : Yolov2 get max score value
* Input :     HI_FLOAT *pf32Val           [IN]  input score
*              HI_U32    u32Num            [IN]  score num
*              HI_U32 *  pu32MaxValueIndex [OUT] the class index of max score
*
*   Output       :
*   Return Value : HI_FLOAT: max score.
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-14
*           Author       :
*           Modification : Create
*
*****************************************************************************/
static HI_FLOAT SVP_NNIE_Yolov2_GetMaxVal(HI_FLOAT *pf32Val,HI_U32 u32Num,
    HI_U32 * pu32MaxValueIndex)
{
    HI_U32 i = 0;
    HI_FLOAT f32MaxTmp = 0;

    f32MaxTmp = pf32Val[0];
    *pu32MaxValueIndex = 0;
    for(i = 1;i < u32Num;i++)
    {
        if(pf32Val[i] > f32MaxTmp)
        {
            f32MaxTmp = pf32Val[i];
            *pu32MaxValueIndex = i;
        }
    }

    return f32MaxTmp;
}

/*****************************************************************************
*   Prototype    : SVP_NNIE_Yolov2_Iou
*   Description  : Yolov2 IOU
* Input :     SAMPLE_SVP_NNIE_YOLOV2_BBOX_S *pstBbox1 [IN]  first bbox
*              SAMPLE_SVP_NNIE_YOLOV2_BBOX_S *pstBbox2 [IN]  second bbox
*              HI_U32    u32ClassNum     [IN]  Class num
*              HI_U32    u32GridNum      [IN]  grid num
*              HI_U32    u32BboxNum      [IN]  bbox num
*              HI_U32    u32ConfThresh   [IN]  confidence thresh
*              HI_U32    u32NmsThresh    [IN]  Nms thresh
*              HI_U32    u32OriImgWidth  [IN]  input image width
*              HI_U32    u32OriImgHeight [IN]  input image height
*              HI_U32*   pu32MemPool     [IN]  assist buffer
*              HI_S32    *ps32DstScores  [OUT]  dst score of ROI
*              HI_S32    *ps32DstRoi     [OUT]  dst Roi
*              HI_S32    *ps32ClassRoiNum[OUT]  dst roi num of each class
*
*   Output       :
* Return Value : HI_DOUBLE: IOU result
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-14
*           Author       :
*           Modification : Create
*
*****************************************************************************/
static HI_DOUBLE SVP_NNIE_Yolov2_Iou(SAMPLE_SVP_NNIE_YOLOV2_BBOX_S *pstBbox1,
    SAMPLE_SVP_NNIE_YOLOV2_BBOX_S *pstBbox2)
{
    HI_FLOAT InterWidth = 0.0;
    HI_FLOAT InterHeight = 0.0;
    HI_DOUBLE f64InterArea = 0.0;
    HI_DOUBLE f64Box1Area = 0.0;
    HI_DOUBLE f64Box2Area = 0.0;
    HI_DOUBLE f64UnionArea = 0.0;

    InterWidth =  SAMPLE_SVP_NNIE_MIN(pstBbox1->f32Xmax, pstBbox2->f32Xmax) - SAMPLE_SVP_NNIE_MAX(pstBbox1->f32Xmin,pstBbox2->f32Xmin);
    InterHeight = SAMPLE_SVP_NNIE_MIN(pstBbox1->f32Ymax, pstBbox2->f32Ymax) - SAMPLE_SVP_NNIE_MAX(pstBbox1->f32Ymin,pstBbox2->f32Ymin);

    if(InterWidth <= 0 || InterHeight <= 0) return 0;

    f64InterArea = InterWidth * InterHeight;
    f64Box1Area = (pstBbox1->f32Xmax - pstBbox1->f32Xmin)* (pstBbox1->f32Ymax - pstBbox1->f32Ymin);
    f64Box2Area = (pstBbox2->f32Xmax - pstBbox2->f32Xmin)* (pstBbox2->f32Ymax - pstBbox2->f32Ymin);
    f64UnionArea = f64Box1Area + f64Box2Area - f64InterArea;

    return f64InterArea/f64UnionArea;
}

/*****************************************************************************
* Prototype :   SVP_NNIE_Yolov1_Argswap
* Description : this function is used to exchange data
* Input :     HI_S32*  ps32Src1           [IN] first input array
*             HI_S32*  ps32Src2           [IN] second input array
*             HI_U32  u32ArraySize        [IN] array size
*
*
* Output :
* Return Value : HI_SUCCESS: Success;Error codes: Failure.
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-11-10
* Author :
* Modification : Create
*
*****************************************************************************/
static void SVP_NNIE_Yolov1_Argswap(HI_S32* ps32Src1, HI_S32* ps32Src2,
    HI_U32 u32ArraySize)
{
    HI_U32 i = 0;
    HI_S32 s32Tmp = 0;
    for( i = 0; i < u32ArraySize; i++ )
    {
        s32Tmp = ps32Src1[i];
        ps32Src1[i] = ps32Src2[i];
        ps32Src2[i] = s32Tmp;
    }
}




/*****************************************************************************
* Prototype :   SVP_NNIE_Yolov1_NonRecursiveArgQuickSort
* Description : this function is used to do quick sort
* Input :     HI_S32*  ps32Array          [IN] the array need to be sorted
*             HI_S32   s32Low             [IN] the start position of quick sort
*             HI_S32   s32High            [IN] the end position of quick sort
*             HI_U32   u32ArraySize       [IN] the element size of input array
*             HI_U32   u32ScoreIdx        [IN] the score index in array element 排序的元素所在的序号
*             SAMPLE_SVP_NNIE_STACK_S *pstStack [IN] the buffer used to store start positions and end positions,用于存储起始位置和结束位置的缓冲区
*
* Output :
* Return Value : HI_SUCCESS: Success;Error codes: Failure.
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-11-10
* Author :
* Modification : Create
*
*****************************************************************************/
static HI_S32 SVP_NNIE_Yolo_NonRecursiveArgQuickSort(HI_S32* ps32Array,
    HI_S32 s32Low, HI_S32 s32High, HI_U32 u32ArraySize,HI_U32 u32ScoreIdx,
    SAMPLE_SVP_NNIE_STACK_S *pstStack)
{
    HI_S32 i = s32Low;
    HI_S32 j = s32High;
    HI_S32 s32Top = 0;
    HI_S32 s32KeyConfidence = ps32Array[u32ArraySize * s32Low + u32ScoreIdx];
    pstStack[s32Top].s32Min = s32Low;
    pstStack[s32Top].s32Max = s32High;

    while(s32Top > -1)
    {
        s32Low = pstStack[s32Top].s32Min;
        s32High = pstStack[s32Top].s32Max;
        i = s32Low;
        j = s32High;
        s32Top--;

        s32KeyConfidence = ps32Array[u32ArraySize * s32Low + u32ScoreIdx];

        while(i < j)
        {
            while((i < j) && (s32KeyConfidence > ps32Array[j * u32ArraySize + u32ScoreIdx]))
            {
                j--;
            }
            if(i < j)
            {
                SVP_NNIE_Yolov1_Argswap(&ps32Array[i*u32ArraySize], &ps32Array[j*u32ArraySize],u32ArraySize);
                i++;
            }

            while((i < j) && (s32KeyConfidence < ps32Array[i*u32ArraySize + u32ScoreIdx]))
            {
                i++;
            }
            if(i < j)
            {
                SVP_NNIE_Yolov1_Argswap(&ps32Array[i*u32ArraySize], &ps32Array[j*u32ArraySize],u32ArraySize);
                j--;
            }
        }

        if(s32Low < i-1)
        {
            s32Top++;
            pstStack[s32Top].s32Min = s32Low;
            pstStack[s32Top].s32Max = i-1;
        }

        if(s32High > i+1)
        {
            s32Top++;
            pstStack[s32Top].s32Min = i+1;
            pstStack[s32Top].s32Max = s32High;
        }
    }
    return HI_SUCCESS;
}


/*****************************************************************************
*   Prototype    : SVP_NNIE_Yolov2_NonMaxSuppression
*   Description  : Yolov2 NonMaxSuppression function
* Input :     SAMPLE_SVP_NNIE_YOLOV2_BBOX_S *pstBbox [IN]  input bbox
*              HI_U32    u32BoxNum       [IN]  Bbox num
*              HI_U32    u32ClassNum     [IN]  Class num
*              HI_U32    u32NmsThresh    [IN]  NMS thresh
*              HI_U32    u32BboxNum      [IN]  bbox num
*              HI_U32    u32MaxRoiNum    [IN]  max roi num
*
*   Output       :
*   Return Value : HI_SUCCESS: Success;Error codes: Failure.
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-14
*           Author       :
*           Modification : Create
*
*****************************************************************************/
static HI_S32 SVP_NNIE_Yolov2_NonMaxSuppression( SAMPLE_SVP_NNIE_YOLOV2_BBOX_S* pstBbox,
    HI_U32 u32BboxNum, HI_U32 u32NmsThresh,HI_U32 u32MaxRoiNum)
{
    HI_U32 i,j;
    HI_U32 u32Num = 0;
    HI_DOUBLE f64Iou = 0.0;

    for (i = 0; i < u32BboxNum && u32Num < u32MaxRoiNum; i++)
    {
        if(pstBbox[i].u32Mask == 0 )  //u32Mask进行nms的标记
        {
            u32Num++;
            for(j= i+1;j< u32BboxNum; j++)
            {
                if( pstBbox[j].u32Mask == 0 )
                {
                    f64Iou = SVP_NNIE_Yolov2_Iou(&pstBbox[i],&pstBbox[j]);
                    if(f64Iou >= (HI_DOUBLE)u32NmsThresh/SAMPLE_SVP_NNIE_QUANT_BASE)  //1288/4096=0.314
                    {
                        pstBbox[j].u32Mask = 1;
                    }
                }
            }
        }
    }

    return HI_SUCCESS;
}



/*****************************************************************************
*   Prototype    : SVP_NNIE_Yolov3_GetResult
*   Description  : Yolov3 GetResult function
* Input :      HI_S32    **pps32InputData     [IN]  pointer to the input data
*              HI_U32    au32GridNumWidth[]   [IN]  Grid num in width direction
*              HI_U32    au32GridNumHeight[]  [IN]  Grid num in height direction
*              HI_U32    au32Stride[]         [IN]  stride of input data
*              HI_U32    u32EachGridBbox      [IN]  Bbox num of each gird
*              HI_U32    u32ClassNum          [IN]  class num
*              HI_U32    u32SrcWidth          [IN]  input image width
*              HI_U32    u32SrcHeight         [IN]  input image height
*              HI_U32    u32MaxRoiNum         [IN]  Max output roi num
*              HI_U32    u32NmsThresh         [IN]  NMS thresh
*              HI_U32    u32ConfThresh        [IN]  conf thresh
*              HI_U32    af32Bias[][]         [IN]  bias
*              HI_U32*   pu32TmpBuf           [IN]  assist buffer
*              HI_S32    *ps32DstScores       [OUT] dst score
*              HI_S32    *ps32DstRoi          [OUT] dst roi
*              HI_S32    *ps32ClassRoiNum     [OUT] class roi num
*
*   Output       :
*   Return Value : HI_FLOAT: max score value.pstNnieParam
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-14
*           Author       :
*           Modification : Create
*
*****************************************************************************/
/* (HI_S32*)pstSoftwareParam->stGetResultTmpBuf.u64VirAddr,
        (HI_S32*)pstSoftwareParam->stDstScore.u64VirAddr,
        (HI_S32*)pstSoftwareParam->stDstRoi.u64VirAddr,
        (HI_S32*)pstSoftwareParam->stClassRoiNum.u64VirAddr*/

//SVP_NNIE_Yolov3_GetResult函数对应参数如下：(pps32InputData,[13,26,52],[13,26,52],au32Stride=[64,112,208],3,2,416,416,10,u32NmsThresh=1288,2048,anchor[3][6],,,,)
static HI_S32 SVP_NNIE_Yolov3_GetResult(HI_S32 **pps32InputData,HI_U32 au32GridNumWidth[],
    HI_U32 au32GridNumHeight[],HI_U32 au32Stride[],HI_U32 u32EachGridBbox,HI_U32 u32ClassNum,HI_U32 u32SrcWidth,
    HI_U32 u32SrcHeight,HI_U32 u32MaxRoiNum,HI_U32 u32NmsThresh,HI_U32 u32ConfThresh,
    HI_FLOAT af32Bias[SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM][SAMPLE_SVP_NNIE_YOLOV3_EACH_GRID_BIAS_NUM],
    HI_S32* ps32TmpBuf,HI_S32 *ps32DstScore, HI_S32 *ps32DstRoi, HI_S32 *ps32ClassRoiNum)
{
    HI_S32 *ps32InputBlob = NULL;
    HI_FLOAT *pf32Permute = NULL;
    SAMPLE_SVP_NNIE_YOLOV3_BBOX_S *pstBbox = NULL;
    HI_S32 *ps32AssistBuf = NULL;
    HI_U32 u32TotalBboxNum = 0;
    HI_U32 u32ChnOffset = 0;
    HI_U32 u32HeightOffset = 0;
    HI_U32 u32BboxNum = 0;
    HI_U32 u32GridXIdx;
    HI_U32 u32GridYIdx;
    HI_U32 u32Offset;
    HI_FLOAT f32StartX;
    HI_FLOAT f32StartY;
    HI_FLOAT f32Width;
    HI_FLOAT f32Height;
    HI_FLOAT f32ObjScore;
    HI_U32 u32MaxValueIndex = 0;
    HI_FLOAT f32MaxScore;
    HI_S32 s32ClassScore;
    HI_U32 u32ClassRoiNum;
    HI_U32 i = 0, j = 0, k = 0, c = 0, h = 0, w = 0;
    HI_U32 u32BlobSize = 0;
    HI_U32 u32MaxBlobSize = 0;

    for(i = 0; i < SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM; i++)
    {
        u32BlobSize = au32GridNumWidth[i]*au32GridNumHeight[i]*sizeof(HI_U32)*
            SAMPLE_SVP_NNIE_YOLOV3_EACH_BBOX_INFER_RESULT_NUM*u32EachGridBbox;
        if(u32MaxBlobSize < u32BlobSize)
        {
            u32MaxBlobSize = u32BlobSize;
        }
    }

    for(i = 0; i < SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM; i++)
    {
        //u32TotalBboxNum=13*13*3+26*26*3+52*52*3=10647
        u32TotalBboxNum += au32GridNumWidth[i]*au32GridNumHeight[i]*u32EachGridBbox;
    }
    // printf("u32TotalBboxNum=%d\n",u32TotalBboxNum);
    //get each tmpbuf addr
    pf32Permute = (HI_FLOAT*)ps32TmpBuf;
    pstBbox = (SAMPLE_SVP_NNIE_YOLOV3_BBOX_S*)(pf32Permute+u32MaxBlobSize/sizeof(HI_S32));
    ps32AssistBuf = (HI_S32*)(pstBbox+u32TotalBboxNum);

    for(i = 0; i < SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM; i++)
    {
        //permute,该操作将[c*h*w] 变为[h*w*c]
        u32Offset = 0;
        ps32InputBlob = pps32InputData[i];
        u32ChnOffset = au32GridNumHeight[i]*au32Stride[i]/sizeof(HI_S32); 
        u32HeightOffset = au32Stride[i]/sizeof(HI_S32);
        //u32ChnOffset=208,728,2704 对应每一个通道的字节数   u32HeightOffset=16,28,52  对应13,26,52
        // printf("u32ChnOffset==%d  u32HeightOffset==%d",u32ChnOffset,u32HeightOffset);
        for (h = 0; h < au32GridNumHeight[i]; h++)
        {
            for (w = 0; w < au32GridNumWidth[i]; w++)
            {
                for (c = 0; c < SAMPLE_SVP_NNIE_YOLOV3_EACH_BBOX_INFER_RESULT_NUM*u32EachGridBbox; c++)
                {
                    pf32Permute[u32Offset++] = (HI_FLOAT)(ps32InputBlob[c*u32ChnOffset+h*u32HeightOffset+w]) / SAMPLE_SVP_NNIE_QUANT_BASE;
                }
            }
        }

        //decode bbox and calculate score
        for(j = 0; j < au32GridNumWidth[i]*au32GridNumHeight[i]; j++)
        {
            u32GridXIdx = j % au32GridNumWidth[i];
            u32GridYIdx = j / au32GridNumWidth[i];
            for (k = 0; k < u32EachGridBbox; k++)
            {
                u32MaxValueIndex = 0;
                u32Offset = (j * u32EachGridBbox + k) * SAMPLE_SVP_NNIE_YOLOV3_EACH_BBOX_INFER_RESULT_NUM;
                //decode bbox
                f32StartX = ((HI_FLOAT)u32GridXIdx + SAMPLE_SVP_NNIE_SIGMOID(pf32Permute[u32Offset + 0])) / au32GridNumWidth[i];
                f32StartY = ((HI_FLOAT)u32GridYIdx + SAMPLE_SVP_NNIE_SIGMOID(pf32Permute[u32Offset + 1])) / au32GridNumHeight[i];
                f32Width = (HI_FLOAT)(exp(pf32Permute[u32Offset + 2]) * af32Bias[i][2*k]) / u32SrcWidth;
                f32Height = (HI_FLOAT)(exp(pf32Permute[u32Offset + 3]) * af32Bias[i][2*k + 1]) / u32SrcHeight;

                //calculate score
                f32ObjScore = SAMPLE_SVP_NNIE_SIGMOID(pf32Permute[u32Offset + 4]);
                (void)SVP_NNIE_SoftMax(&pf32Permute[u32Offset + 5], u32ClassNum);
                f32MaxScore = SVP_NNIE_Yolov2_GetMaxVal(&pf32Permute[u32Offset + 5], u32ClassNum, &u32MaxValueIndex);
                s32ClassScore = (HI_S32)(f32MaxScore * f32ObjScore*SAMPLE_SVP_NNIE_QUANT_BASE);

                //filter low score roi
                if (s32ClassScore > u32ConfThresh) //即conf大于0.5
                {
                    //将预测的中心点坐标变换成对角线坐标
                    pstBbox[u32BboxNum].f32Xmin= (HI_FLOAT)(f32StartX - f32Width * 0.5f);
                    pstBbox[u32BboxNum].f32Ymin= (HI_FLOAT)(f32StartY - f32Height * 0.5f);
                    pstBbox[u32BboxNum].f32Xmax= (HI_FLOAT)(f32StartX + f32Width * 0.5f);
                    pstBbox[u32BboxNum].f32Ymax= (HI_FLOAT)(f32StartY + f32Height * 0.5f);
                    pstBbox[u32BboxNum].s32ClsScore = s32ClassScore;
                    pstBbox[u32BboxNum].u32Mask= 0;
                    pstBbox[u32BboxNum].u32ClassIdx = (HI_S32)(u32MaxValueIndex+1);
                    
                    // printf("\ns32ClsScore==%d\n",s32ClassScore);
                    // printf("\nu32BboxNum===%d\n",u32BboxNum);
                    // printf("\npstBbox[u32BboxNum].s32ClsScore==%d\n",pstBbox[u32BboxNum].s32ClsScore);
                    u32BboxNum++;
                }
            }
        }
    }
    // printf("u32BboxNum====%d\n",u32BboxNum);
    //quick sort
    // printf("\n sort qiang socres==%d,%d\n",pstBbox[0].s32ClsScore,pstBbox[1].s32ClsScore);

    (void)SVP_NNIE_Yolo_NonRecursiveArgQuickSort((HI_S32*)pstBbox, 0, u32BboxNum - 1,
        sizeof(SAMPLE_SVP_NNIE_YOLOV3_BBOX_S)/sizeof(HI_U32),4,(SAMPLE_SVP_NNIE_STACK_S*)ps32AssistBuf);
    // printf("\n sort qiang nms==%d,%d\n",pstBbox[0].s32ClsScore,pstBbox[1].s32ClsScore);
    //Yolov3 and Yolov2 have the same Nms operation
    (void)SVP_NNIE_Yolov2_NonMaxSuppression(pstBbox, u32BboxNum, u32NmsThresh, sizeof(SAMPLE_SVP_NNIE_YOLOV3_BBOX_S)/sizeof(HI_U32));
    // printf("\n sort hong nms==%d,%d\n",pstBbox[0].s32ClsScore,pstBbox[1].s32ClsScore);

    //Get result
    for (i = 1; i <= u32ClassNum; i++)
    {
        u32ClassRoiNum = 0;
        for(j = 0; j < u32BboxNum; j++)
        {
            if ((0 == pstBbox[j].u32Mask) && (i == pstBbox[j].u32ClassIdx) && (u32ClassRoiNum < u32MaxRoiNum))
            {
                *(ps32DstRoi++) = SAMPLE_SVP_NNIE_MAX((HI_S32)(pstBbox[j].f32Xmin*u32SrcWidth), 0);
                *(ps32DstRoi++) = SAMPLE_SVP_NNIE_MAX((HI_S32)(pstBbox[j].f32Ymin*u32SrcHeight), 0);
                *(ps32DstRoi++) = SAMPLE_SVP_NNIE_MIN((HI_S32)(pstBbox[j].f32Xmax*u32SrcWidth), u32SrcWidth);
                *(ps32DstRoi++) = SAMPLE_SVP_NNIE_MIN((HI_S32)(pstBbox[j].f32Ymax*u32SrcHeight), u32SrcHeight);
                *(ps32DstScore++) = pstBbox[j].s32ClsScore;
                u32ClassRoiNum++;
                // printf("\ndebug:::%d,%d,%d,%d,%d===\n",SAMPLE_SVP_NNIE_MAX((HI_S32)(pstBbox[j].f32Xmin*u32SrcWidth), 0),SAMPLE_SVP_NNIE_MAX((HI_S32)(pstBbox[j].f32Ymin*u32SrcHeight), 0),
                //             SAMPLE_SVP_NNIE_MIN((HI_S32)(pstBbox[j].f32Xmax*u32SrcWidth), u32SrcWidth),SAMPLE_SVP_NNIE_MIN((HI_S32)(pstBbox[j].f32Ymax*u32SrcHeight), u32SrcHeight),pstBbox[j].s32ClsScore);

                // printf("\nroi %d %d %d %d %d\n",*(ps32DstRoi-4),*(ps32DstRoi-3),*(ps32DstRoi-2),*(ps32DstRoi-1),*(ps32DstScore-1));
            }
        }
        *(ps32ClassRoiNum+i) = u32ClassRoiNum;
        // printf("\nu32ClassRoiNum====%d\n",ps32ClassRoiNum[i]);

    }

    return HI_SUCCESS;
}


/*****************************************************************************
* Prototype :   SAMPLE_SVP_NNIE_Yolov3_GetResult
* Description : this function is used to Get Yolu64PhyAddrov3 result
* Input :      SAMPLE_SVP_NNIE_PARAM_S*               pstNnieParam     [IN]  the pointer to YOLOV3 NNIE parameter
*              SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S*   pstSoftwareParam [IN]  the pointer to YOLOV3 software parameter
*
*
*
*
* Output :
* Return Value : HI_SUCCESS: Success;Error codes: Failure.
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-11-10
* Author :
* Modification : Create
*
*****************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_Yolov3_GetResult(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,
    SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftwareParam)
{
    HI_U32 i = 0;
    HI_S32 *aps32InputBlob[SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM] = {0};
    HI_U32 au32Stride[SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM] = {0};

    for(i = 0; i < SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM; i++)
    {
        aps32InputBlob[i] = (HI_S32*)pstNnieParam->astSegData[0].astDst[i].u64VirAddr;
        au32Stride[i] = pstNnieParam->astSegData[0].astDst[i].u32Stride;
        // printf("size=%d",sizeof(aps32InputBlob[i])/sizeof(HI_S32*));
        // printf("au32Stride[i]==%d",au32Stride[i]);
        // printf("\n");
    }
    // for(int i=0;i<3;i++)
    // {
    //     printf("pstSoftwareParam->au32GridNumWidth=%d;pstSoftwareParam->au32GridNumHeight=%d;pstSoftwareParam->u32BboxNumEachGrid=%d",pstSoftwareParam->au32GridNumWidth[i],pstSoftwareParam->au32GridNumHeight[i],pstSoftwareParam->u32BboxNumEachGrid);
    //     printf("\n");
    // }
    // printf("pstSoftwareParam->u32ClassNum=%d;pstSoftwareParam->u32OriImWidth=%d;pstSoftwareParam->u32MaxRoiNum=%d",pstSoftwareParam->u32ClassNum,pstSoftwareParam->u32OriImWidth,pstSoftwareParam->u32MaxRoiNum);
    // printf("pstSoftwareParam->u32NmsThresh=%d;pstSoftwareParam->u32ConfThresh=%d",pstSoftwareParam->u32NmsThresh,pstSoftwareParam->u32ConfThresh);
    // printf("\n ok \n");

    return SVP_NNIE_Yolov3_GetResult(aps32InputBlob,pstSoftwareParam->au32GridNumWidth,
        pstSoftwareParam->au32GridNumHeight,au32Stride,pstSoftwareParam->u32BboxNumEachGrid,
        pstSoftwareParam->u32ClassNum,pstSoftwareParam->u32OriImWidth,
        pstSoftwareParam->u32OriImWidth,pstSoftwareParam->u32MaxRoiNum,pstSoftwareParam->u32NmsThresh,
        pstSoftwareParam->u32ConfThresh,pstSoftwareParam->af32Bias,
        (HI_S32*)pstSoftwareParam->stGetResultTmpBuf.u64VirAddr,
        (HI_S32*)pstSoftwareParam->stDstScore.u64VirAddr,
        (HI_S32*)pstSoftwareParam->stDstRoi.u64VirAddr,
        (HI_S32*)pstSoftwareParam->stClassRoiNum.u64VirAddr);
}




/*****************************************************************************
 函 数 名  : SDC_YuvChnAttrGetIdleYuvChn
 功能描述  : 查询YUV逻辑通道信息
 输入参数  : int fd  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2019年6月8日
    作    者   : jelly
    修改内容   : 新生成函数

*****************************************************************************/
int SDC_YuvChnAttrGetIdleYuvChn(int fd, unsigned int *puiChnId)
{
	int nret,i;
	char buf[4096] = { 0 };
	sdc_yuv_channel_info_s* info;
	sdc_common_head_s* head = (sdc_common_head_s*)buf;
	sdc_chan_query_s* param;

	/** query all channels' info */
	head->version = SDC_VERSION; //0x5331
	head->url = SDC_URL_YUV_CHANNEL; //0x00
	head->method = SDC_METHOD_GET; //0x02
	head->head_length = sizeof(*head);

	param = (sdc_chan_query_s*)(buf+head->head_length);
    param->channel = 0;/*通道ID为0时查询的是所有的逻辑通道*/
    //head->content_length = sizeof(sdc_chan_query_s);

	head->content_length = 0;
    
	nret = write(fd, head, head->head_length);
	if(nret < 0)
	{
	   fprintf(stdout,"writev fail yuv_channel_get,response:%d,url:%d,code:%d,method:%d\n",head->response,head->url,head->code, head->method);
	   return errno;
	}
	fprintf(stdout,"yuv_channel_get write succeed \n");

	nret = read(fd,buf,sizeof(buf));
	if(nret < 0 || head->code != SDC_CODE_200)
	{
	   fprintf(stdout,"read fail response:%d,url:%d,code:%d, method:%d\n",head->response,head->url,head->code, head->method);
	   return errno;
	}
	
	fprintf(stdout,"yuv_channel_get read succeed content_length:%d \n", head->content_length);

	info = (sdc_yuv_channel_info_s*)&buf[head->head_length];
	for(i = 0; i < head->content_length / sizeof(*info); i++, info++) 
	{
	    if ((info->is_snap_channel == 0)
			&& (info->subscribe_cnt ==0)
			&& (info->nResolutionModitfy ==1))
    	{
    	    *puiChnId = info->param.channel;
			fprintf(stdout,"Get yuv video chn：%d\n", info->param.channel);
            return 0;
    	}
	}
	
    return ERR;
}


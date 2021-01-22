/******************************************************************************

                  版权所有 (C), 2019-2029, SDC OS 开源软件小组所有

 ******************************************************************************
  文 件 名   : queue.c
  版 本 号   : 初稿
  作    者   : jelly
  生成日期   : 2019年6月8日
  最近修改   :
  功能描述   : 队列
  函数列表   :
  修改历史   :
  1.日    期   : 2019年6月8日
    作    者   : jelly
    修改内容   : 创建文件

******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>


#include "malloc.h"
#include "queue.h"
#include "sdc.h"


int QUE_CreateQueue(QUEUE_S *pstQueue, unsigned int uiMaxSize)
{
	if((NULL == pstQueue) || (0 == uiMaxSize))
	{
		printf("Err in QUE_PushQueue, pstQueue or  uiMaxSize is 0\r\n");
		return QUEUE_STATE_PARAS_ERR;       
	}

	pstQueue->pBase = (char *)malloc(sizeof(sdc_yuv_data_s)*uiMaxSize);
	if(NULL == pstQueue->pBase)
	{
		printf("Err in QUE_CreateQueue");
		return ERR;      
	}
    
	pstQueue->uiRead = 0;         //初始化參数
	pstQueue->uiWrite = 0;
	pstQueue->uiMaxSize = uiMaxSize;
    return OK;
}


int QUE_PushQueue(QUEUE_S *pstQueue, char *pucSdcYuvData)
{
    unsigned int uiPtrIndex;
    //sdc_yuv_data_s *pstYuvData = NULL;
     
	if((NULL == pstQueue) || (NULL == pucSdcYuvData))
	{
		printf("Err in QUE_PushQueue, pstQueue or  pstSdcYuvData is null\r\n");
		return QUEUE_STATE_PARAS_ERR;       
	}

    uiPtrIndex = (pstQueue->uiWrite + 1) % pstQueue->uiMaxSize;
    /*如果队列满，入队返回失败*/
    if (pstQueue->uiRead == uiPtrIndex)
    {
		return QUEUE_STATE_FULL; 
    }

    memcpy((void *)(pstQueue->pBase + sizeof(sdc_yuv_data_s)*pstQueue->uiWrite) , (void *)pucSdcYuvData, sizeof(sdc_yuv_data_s));

    //pstYuvData = (sdc_yuv_data_s *)(pstQueue->pBase + sizeof(sdc_yuv_data_s)*pstQueue->uiWrite);
    
    //printf("******************QUE_PushQueue, uiWrite:%d, channel:%d, addr:%ld,addr_phy:%ld\r\n", 
        //pstQueue->uiWrite, pstYuvData->channel, pstYuvData, pstYuvData->frame.addr_phy);

    /*更新写指针*/
    pstQueue->uiWrite = uiPtrIndex;
    
    return QUEUE_STATE_OK;

}


int QUE_PopQueue(QUEUE_S *pstQueue, char *pucSdcYuvData)
{
    unsigned int uiPtrIndex;
    //sdc_yuv_data_s *pstYuvData = NULL;
     
	if ((NULL == pstQueue) || (NULL == pucSdcYuvData))
	{
		printf("Err in QUE_PopQueue, , pstQueue or  pstSdcYuvData is null\r\n");
		return QUEUE_STATE_PARAS_ERR;       
	}

    uiPtrIndex = pstQueue->uiRead;
    
    /*队列空*/
    if (pstQueue->uiRead == pstQueue->uiWrite)
    {
		return QUEUE_STATE_EMPT; 
    }

    memcpy((void *)pucSdcYuvData, (void *)(pstQueue->pBase + sizeof(sdc_yuv_data_s)*uiPtrIndex) , sizeof(sdc_yuv_data_s));

    //pstYuvData = (sdc_yuv_data_s *)(pstQueue->pBase + sizeof(sdc_yuv_data_s)*uiPtrIndex);
    //printf("******************QUE_PopQueue, uiRead:%d, channel:%d, addr:%ld, addr_phy:%ld\r\n", 
        //uiPtrIndex, pstYuvData->channel, pstYuvData, pstYuvData->frame.addr_phy);


    /*更新读指针*/
    pstQueue->uiRead = (uiPtrIndex + 1) % pstQueue->uiMaxSize;
    
    return QUEUE_STATE_OK;
}


int QUE_GetQueueSize(QUEUE_S *pstQueue)
{
    unsigned int uiQueueSize = 0;
	if (NULL == pstQueue) 
	{
		printf("Err in QUE_GetQueueSize, pstQueue is null\r\n");
		return uiQueueSize;       
	}

    uiQueueSize = (pstQueue->uiWrite +  pstQueue->uiMaxSize - pstQueue->uiRead) % pstQueue->uiMaxSize;
    
    return uiQueueSize;
}



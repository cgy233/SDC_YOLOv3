/******************************************************************************

                  ��Ȩ���� (C), 2019-2029, SDC OS ��Դ���С������

 ******************************************************************************
  �� �� ��   : queue.c
  �� �� ��   : ����
  ��    ��   : jelly
  ��������   : 2019��6��8��
  ����޸�   :
  ��������   : ����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��6��8��
    ��    ��   : jelly
    �޸�����   : �����ļ�

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
    
	pstQueue->uiRead = 0;         //��ʼ������
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
    /*�������������ӷ���ʧ��*/
    if (pstQueue->uiRead == uiPtrIndex)
    {
		return QUEUE_STATE_FULL; 
    }

    memcpy((void *)(pstQueue->pBase + sizeof(sdc_yuv_data_s)*pstQueue->uiWrite) , (void *)pucSdcYuvData, sizeof(sdc_yuv_data_s));

    //pstYuvData = (sdc_yuv_data_s *)(pstQueue->pBase + sizeof(sdc_yuv_data_s)*pstQueue->uiWrite);
    
    //printf("******************QUE_PushQueue, uiWrite:%d, channel:%d, addr:%ld,addr_phy:%ld\r\n", 
        //pstQueue->uiWrite, pstYuvData->channel, pstYuvData, pstYuvData->frame.addr_phy);

    /*����дָ��*/
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
    
    /*���п�*/
    if (pstQueue->uiRead == pstQueue->uiWrite)
    {
		return QUEUE_STATE_EMPT; 
    }

    memcpy((void *)pucSdcYuvData, (void *)(pstQueue->pBase + sizeof(sdc_yuv_data_s)*uiPtrIndex) , sizeof(sdc_yuv_data_s));

    //pstYuvData = (sdc_yuv_data_s *)(pstQueue->pBase + sizeof(sdc_yuv_data_s)*uiPtrIndex);
    //printf("******************QUE_PopQueue, uiRead:%d, channel:%d, addr:%ld, addr_phy:%ld\r\n", 
        //uiPtrIndex, pstYuvData->channel, pstYuvData, pstYuvData->frame.addr_phy);


    /*���¶�ָ��*/
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



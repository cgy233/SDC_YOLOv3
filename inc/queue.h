/******************************************************************************

                  ��Ȩ���� (C), 2019-2029, SDC OS ��Դ���С������

 ******************************************************************************
  �� �� ��   : queue.h
  �� �� ��   : ����
  ��    ��   : jelly
  ��������   : 2019��6��9��
  ����޸�   :
  ��������   : queue.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��6��9��
    ��    ��   : jelly
    �޸�����   : �����ļ�

******************************************************************************/



#ifndef __QUEUE_H__
#define __QUEUE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


enum QUEUE_STATE_E
{
    QUEUE_STATE_EMPT,
    QUEUE_STATE_FULL,
    QUEUE_STATE_OK,
    QUEUE_STATE_PARAS_ERR,
    QUEUE_STATE_ABORT
};


typedef struct QUEUE_STRU
{
    unsigned int uiRead;    //ָ����е�һ��Ԫ��
    unsigned int uiWrite;    //ָ��������һ��Ԫ�ص���һ��Ԫ��
    unsigned int uiMaxSize; //ѭ�����е����洢�ռ�
    char *pBase;
}QUEUE_S;


extern int QUE_CreateQueue(QUEUE_S *pstQueue, unsigned int uiMaxSize);
extern int QUE_PopQueue(QUEUE_S *pstQueue, char *pucSdcYuvData);
extern int QUE_PushQueue(QUEUE_S *pstQueue, char *pucSdcYuvData);
extern int QUE_GetQueueSize(QUEUE_S *pstQueue);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __QUEUE_H__ */

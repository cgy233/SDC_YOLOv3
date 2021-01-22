/******************************************************************************

                  版权所有 (C), 2019-2029, SDC OS 开源软件小组所有

 ******************************************************************************
  文 件 名   : queue.h
  版 本 号   : 初稿
  作    者   : jelly
  生成日期   : 2019年6月9日
  最近修改   :
  功能描述   : queue.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年6月9日
    作    者   : jelly
    修改内容   : 创建文件

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
    unsigned int uiRead;    //指向队列第一个元素
    unsigned int uiWrite;    //指向队列最后一个元素的下一个元素
    unsigned int uiMaxSize; //循环队列的最大存储空间
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

/******************************************************************************

                  版权所有 (C), 2019-2029, SDC OS 开源软件小组所有

 ******************************************************************************
  文 件 名   : main.h
  版 本 号   : 初稿
  作    者   : jelly
  生成日期   : 2019年6月9日
  最近修改   :
  功能描述   : main.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年6月9日
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

#ifndef __MAIN_H__
#define __MAIN_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

enum SYSTEM_STATE_E
{
    SYS_STATE_IDLE,
    SYS_STATE_STARTING,
    SYS_STATE_NORMAL,
    SYS_STATE_ABORT
};

typedef struct SYSTEM_MANAGE_PARAS_STRU
{
    unsigned int uiSystemState;
    unsigned int uiMaxUsedBufNum;/*最大缓存buf数量*/
    
}SYSTEM_MANAGE_PARAS_S;

typedef struct META_INFO_STRU
{
    unsigned short usX;      
    unsigned short usY;      
    unsigned short usWidth;  
    unsigned short usHeight; 
    unsigned int uclass;
    float confidence;
}META_INFO_S;
 
/* 声明结构体 */
struct member
{
    int num;
    char *name;
};     


extern int main(int argc,char* argv[]);
extern void * SDC_ReadFromVideoService(void *arg);
extern void * SDC_YuvDataProc(void *arg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __MAIN_H__ */

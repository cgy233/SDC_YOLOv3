/************************************************************************
*             Copyright (C)  2018, Hisilicon Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: hi_comm_motionfusion.h
* Description: init draft
*
*************************************************************************/
#ifndef _COMM_MONTIONFUSION_H_
#define _COMM_MONTIONFUSION_H_

#include "hi_type.h"
#include "hi_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define HI_ERR_MOTIONFUSION_NOBUF                HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_NOBUF)
#define HI_ERR_MOTIONFUSION_BUF_EMPTY            HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_EMPTY)
#define HI_ERR_MOTIONFUSION_NULL_PTR             HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
#define HI_ERR_MOTIONFUSION_ILLEGAL_PARAM        HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
#define HI_ERR_MOTIONFUSION_BUF_FULL             HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_FULL)
#define HI_ERR_MOTIONFUSION_SYS_NOTREADY         HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)
#define HI_ERR_MOTIONFUSION_NOT_SUPPORT          HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SUPPORT)
#define HI_ERR_MOTIONFUSION_NOT_PERMITTED        HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)
#define HI_ERR_MOTIONFUSION_BUSY                 HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)
#define HI_ERR_MOTIONFUSION_INVALID_CHNID        HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_CHNID)
#define HI_ERR_MOTIONFUSION_CHN_UNEXIST          HI_DEF_ERR(HI_ID_MOTIONFUSION, EN_ERR_LEVEL_ERROR, EN_ERR_UNEXIST)

#define HI_TRACE_MOTIONFUSION(level, fmt, ...)\
do{ \
    HI_TRACE(level, HI_ID_MOTIONFUSION,"[Func]:%s [Line]:%d [Info]:"fmt,__FUNCTION__, __LINE__,##__VA_ARGS__);\
}while(0)

#define AXIS_NUM  (3)
#define MATRIX_NUM  (9)

#define    MFUSION_TEMP_GYRO  0x1
#define    MFUSION_TEMP_ACC   0x2
#define    MFUSION_TEMP_MAGN  0x4

typedef HI_S32 IMU_DRIFT[AXIS_NUM];
typedef HI_S32 IMU_MATRIX[MATRIX_NUM];

typedef struct hiMFUSION_EULER_S
{
    HI_S32      s32Roll;
    HI_S32      s32Pitch;
    HI_S32      s32Yaw;
} MFUSION_EULER_S;

typedef enum hiMFUSION_MODE_E
{
    MFUSION_MODE_3AXIS = 0,
    MFUSION_MODE_6AXIS ,
    MFUSION_MODE_9AXIS ,
    MFUSION_MODE_BUTT
} MFUSION_MODE_E;

typedef enum hiMFUSION_DEVICE_E
{
    MFUSION_DEVICE_GYRO = 0,
    MFUSION_DEVICE_ACC  ,
    MFUSION_DEVICE_MAGN ,
    MFUSION_DEVICE_BUTT
} MFUSION_DEVICE_E;


typedef enum hiMFUSION_USECASE_E
{
    MFUSION_USECASE_DIS = 0,
    MFUSION_USECASE_RECTIFY ,
    MFUSION_USECASE_BUTT
} MFUSION_USECASE_E;


typedef struct hiMFUSION_ATTR_S
{
    MFUSION_MODE_E    enMFusionMode;
    HI_S32            s32TemperatureMask;
} MFUSION_ATTR_S;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif


/******************************************************************************

                  版权所有 (C), 2019-2029, SDC OS 开源软件小组所有

 ******************************************************************************
  文 件 名   : sdc_os_api.h
  版 本 号   : 初稿
  作    者   : jelly
  生成日期   : 2019年6月8日
  最近修改   :
  功能描述   : sdc_os_api.c 的头文件
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

#ifndef __SDC_OS_API_H__
#define __SDC_OS_API_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "sdc.h"
#include "sample_comm_nnie.h"


#define YUV_CHANNEL_LEN (3)
typedef struct YUV_FRAME
{
    UINT32 uWidth;
    UINT32 uHeight;

    //VW_PIXEL_FORMAT_E enPixelFormat;
    int enPixelFormat;

    UINT64 ulPhyAddr[YUV_CHANNEL_LEN];
    UINT64 ulVirAddr[YUV_CHANNEL_LEN];
    UINT32 uStride[YUV_CHANNEL_LEN];

    UINT64 ullpts;

    UINT32 uVbBlk;
    UINT32 uPoolId;
    char* pYuvImgAddr;
    UINT32 uFrmSize;

} VW_YUV_FRAME_S;

/*stack for sort*/
typedef struct hiSAMPLE_SVP_NNIE_STACK
{
    HI_S32 s32Min;
    HI_S32 s32Max;
}SAMPLE_SVP_NNIE_STACK_S;


#define SAMPLE_SVP_NNIE_COORDI_NUM  4      /*coordinate numbers*/
#define SAMPLE_SVP_NNIE_HALF 0.5f          /*the half value*/

#define SAMPLE_SVP_NNIE_MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define SAMPLE_SVP_NNIE_MIN(a,b)    (((a) < (b)) ? (a) : (b))

#define SAMPLE_SVP_NNIE_PROPOSAL_WIDTH  6  /*the number of proposal values*/
#define SAMPLE_SVP_NNIE_QUANT_BASE 4096    /*the base value*/




extern void SDC_DisplayExtendHead(sdc_extend_head_s* extendhead);
extern void SDC_DisplayYuvData(sdc_yuv_data_s* yuv_data);
extern int SDC_GetHardWareId(sdc_hardware_id_s *pstHardWareParas);
extern int SDC_LoadModel(unsigned int uiLoadMode, char *pucModelFileName, SVP_NNIE_MODEL_S *pstModel);
extern int SDC_MemAlloc(int fd, unsigned int size, int uiCacheFlag, sdc_mmz_alloc_s* pstMemParas);
extern void SDC_MemFree(int fd, sdc_mmz_alloc_s* pstMemParas);
extern int SDC_ModelDecript(sdc_mmz_alloc_s *pstMmzAddr);
extern int SDC_Nnie_Forward(sdc_nnie_forward_s *p_sdc_nnie_forward);
extern int SDC_Nnie_Forward_Withbox(sdc_nnie_forward_withbox_s *p_sdc_nnie_forward_withbox);
extern int SDC_ServiceCreate(void);
extern int SDC_UnLoadModel(SVP_NNIE_MODEL_S *pstModel);
extern int SDC_YuvChnAttrGet(int fd);
extern int SDC_YuvChnAttrSet(int fd, int uiYuvChnId);
extern void SDC_YuvDataFree(int fd,  sdc_yuv_data_s *yuv_data);
extern int SDC_YuvDataReq(int fd, int extendheadflag, unsigned int uiChnId, unsigned int uiMaxUsedBufNum);

extern int SDC_LoadModel_test(unsigned int uiLoadMode, char *pucModelFileName, SVP_NNIE_MODEL_S *pstModel);
extern HI_S32 SAMPLE_SVP_NNIE_Yolov3_ParamInit(int fd, SAMPLE_SVP_NNIE_CFG_S* pstCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam);

extern HI_S32 SAMPLE_COMM_SVP_SysInit(HI_VOID);

extern int SDC_TransYUV2RGB(int fd, sdc_yuv_frame_s *yuv, sdc_yuv_frame_s *rgb);
extern void SDC_Struct2RGB(sdc_yuv_frame_s *pstSdcRGBFrame, VW_YUV_FRAME_S *pstRGBFrameData);
extern int SDC_TransYUV2RGBRelease(int fd, sdc_yuv_frame_s *rgb);
extern int SDC_YuvChnAttrGetIdleYuvChn(int fd, unsigned int *puiChnId);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SDC_OS_API_H__ */

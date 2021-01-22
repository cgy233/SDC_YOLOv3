/************************************************************************
*             Copyright (C) 2018, Hisilicon Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: mpi_montionfusion.h
* Description: init draft
*
*************************************************************************/
#ifndef  __MPI_MONTIONFUSION_H__
#define  __MPI_MONTIONFUSION_H__

#include "hi_type.h"
#include "hi_comm_motionfusion.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

//-------------------------set fusion attr  --------------------------------------------
HI_S32 HI_MPI_MOTIONFUSION_SetAttr(const MFUSION_ATTR_S *pstMFusionAttr);

HI_S32 HI_MPI_MOTIONFUSION_GetAttr(MFUSION_ATTR_S *pstMFusionAttr);


//-------------------------set calibration param --------------------------------------------
/* gyro drift cal */
HI_S32 HI_MPI_MONTIONFUSION_SetGyroDrift(HI_BOOL  bEnDrift,  IMU_DRIFT aGyroDrift);

HI_S32 HI_MPI_MONTIONFUSION_GetGyroDrift(HI_BOOL *pbEnDrift, IMU_DRIFT aGyroDrift);

/* gyro six side cal */
HI_S32 HI_MPI_MONTIONFUSION_SetGyroSixSideCal(HI_BOOL  bEnSixSideCal,  IMU_MATRIX aRotationMatrix);

HI_S32 HI_MPI_MONTIONFUSION_GetGyroSixSideCal(HI_BOOL *pbEnSixSideCal, IMU_MATRIX aRotationMatrix);

/* gyro temp cal */


/* acc offset drift cal */
HI_S32 HI_MPI_MONTIONFUSION_SetACCDrift(HI_BOOL  bEnDrift,  IMU_DRIFT aACCDrift);

HI_S32 HI_MPI_MONTIONFUSION_GetACCDrift(HI_BOOL  *pbEnDrift, IMU_DRIFT aACCDrift);

/* acc six side cal */
HI_S32 HI_MPI_MONTIONFUSION_SetACCSixSideCal(HI_BOOL  bEnSixSideCal,  IMU_MATRIX aRotationMatrix);

HI_S32 HI_MPI_MONTIONFUSION_GetAccSixSideCal(HI_BOOL  *pbEnSixSideCal, IMU_MATRIX aRotationMatrix);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif/* End of #ifndef __MPI_MONTIONFUSION_H__*/

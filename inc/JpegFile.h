#ifndef __MULTI_OSD_H__
#define __MULTI_OSD_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

//#include <string.h>
#include "sdc.h"


int32_t JpegFileInit(void);
int32_t JpegFileSaveYuv2Jpeg(const struct sdc_yuv_frame_stru *yuv_frame, const char *jpegPath);

#if 0
namespace SdcOSExamples
{
using std::string;
class JpegFile
{
public:
    JpegFile(void);
    virtual ~JpegFile(void);
    int32_t Init(void);
    int32_t SaveJpeg(const struct sdc_yuv_frame_stru &yuv_frame, const string &jpegPath) const;

protected:
    int32_t m_codecFd;  // 图片解码文件描述符

private:
    int32_t InitOsdRegion(const struct sdc_yuv_frame_stru &yuv_frame, struct sdc_osd_region_stru &osd_region) const;
    int32_t Yuv2Jpeg(const struct sdc_yuv_frame_stru &yuv_frame, const struct sdc_osd_region_stru &osd_region, struct sdc_jpeg_frame_stru &jpeg_frame) const;
    int32_t FreeJpeg(struct sdc_jpeg_frame_stru &jpeg_frame) const;
    int32_t SaveJpeg(const struct sdc_jpeg_frame_stru &jpeg_frame, const string &jpegPath) const;
};
}
#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif
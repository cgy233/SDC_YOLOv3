#include "JpegFile.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <securec.h>
//#include <string>
#include "basedef.h"

extern int fd_codec;


int32_t JpegFileInit(void)
{
	fd_codec = open("/mnt/srvfs/codec.iaas.sdc", O_RDWR);
	if (fd_codec < 0) {
		LOG_ERROR("Open the /mnt/srvfs/codec.iaas.sdc failed, errno: %d, errmsg: %s", errno, strerror(errno));
		return ERR;
	}
	LOG_ERROR("Open the /mnt/srvfs/codec.iaas.sdc successfully");
	
	return OK;
}

int32_t JpegFileYuv2Jpeg(const struct sdc_yuv_frame_stru &yuv_frame, const struct sdc_osd_region_stru &osd_region, struct sdc_jpeg_frame_stru &jpeg_frame)
{
	struct sdc_encode_jpeg_param_stru param;
	memset_s(&param, sizeof(param), 0, sizeof(param));
	param.qf = 80;
	param.osd_region_cnt = 1;
	param.region = { 0, 0, yuv_frame.width, yuv_frame.height };
	param.frame = yuv_frame;

	struct sdc_common_head_stru head;
	memset_s(&head, sizeof(head), 0, sizeof(head));
	head.version = SDC_VERSION; // 0x5331
	head.url = SDC_URL_ENCODED_JPEG; // 0x00
	head.method = SDC_METHOD_CREATE;
	head.head_length = sizeof(head);
	head.content_length = sizeof(param) + sizeof(osd_region);
	
	struct iovec iov[3];
	iov[0].iov_base = &head;
	iov[0].iov_len = sizeof(head);
	iov[1].iov_base = &param;
	iov[1].iov_len = sizeof(param);
	iov[2].iov_base = (void *)&osd_region;
	iov[2].iov_len = sizeof(osd_region);

	int32_t nret = writev(fd_codec, iov, 3);
	if(nret < 0) {
		LOG_ERROR("Write the iovec failed, errno: %d, errmsg: %s", errno, strerror(errno));
		return errno;
	}

	iov[1].iov_base = &jpeg_frame;
	iov[1].iov_len = sizeof(jpeg_frame);
	nret = readv(fd_codec, iov, 2);
	if(nret < 0) {
		LOG_ERROR("Read the jpeg frame failed, errno: %d, errmsg: %s", errno, strerror(errno));
		return errno;
	}

	if(head.head_length != sizeof(head) || head.content_length != sizeof(jpeg_frame)) {
		LOG_ERROR("Translate the Yuv frame to jpeg frame failed");
		return EIO;
	}
	LOG_DEBUG("Translate the Yuv frame to jpeg frame successfully");
	return OK;
}

int32_t JpegFileFreeJpeg(struct sdc_jpeg_frame_stru &jpeg_frame)
{
	struct sdc_common_head_stru head;
	memset_s(&head, sizeof(head), 0, sizeof(head));
	head.version = SDC_VERSION; // 0x5331
	head.url = SDC_URL_ENCODED_JPEG; // 0x00
	head.method = SDC_METHOD_DELETE;
	head.head_length = sizeof(head);
	head.content_length = sizeof(jpeg_frame);

	struct iovec iov[2];
	iov[0].iov_base = (void *)&head;
	iov[0].iov_len = sizeof(head);
	iov[1].iov_base = (void *)&jpeg_frame;
	iov[1].iov_len = sizeof(jpeg_frame);
	
	int32_t nret = writev(fd_codec, iov, 2);
	if (nret < 0) {
		LOG_ERROR("Write the iovec failed, errno: %d, errmsg: %s", errno, strerror(errno));
	}
	LOG_DEBUG("Write the iovec successfully");

	return OK;
}

static int32_t JpegFileSaveJpeg(const struct sdc_jpeg_frame_stru &jpeg_frame, const char *jpegPath)
{
	int32_t fd = -1;
	if ((fd=open(jpegPath, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0) {
		LOG_ERROR("Open the jpeg file failed, file_path: %s, errno: %d, errmsg: %s", jpegPath, errno, strerror(errno));
		return ERR;
	}

	ssize_t writeLen= 0;
	if ((writeLen=write(fd, (void*)jpeg_frame.addr_virt, jpeg_frame.size)) < 0) {
		LOG_ERROR("Write the jpeg file failed, errno: %d, errmsg: %s", errno, strerror(errno));
	} else if(writeLen != (ssize_t)jpeg_frame.size) {
		LOG_ERROR("Write the jpeg file truncted, write_size: %ld, real_size: %u", writeLen, jpeg_frame.size);
	}else {
		LOG_DEBUG("Save the jpeg file successfully, file_path: %s, real_size: %u", jpegPath, jpeg_frame.size);
	}
	close(fd);

	return OK;
}

static int32_t JpegFileSaveYuvToJpeg(const struct sdc_yuv_frame_stru &yuv_frame, const char *jpegPath)
{
	struct sdc_osd_region_stru osd_region;
	memset_s(&osd_region, sizeof(osd_region), 0, sizeof(osd_region));
	
	// 如果需要添加1个OSD，请初始化InitOsdRegion()，否在注释OSD
	// (void) InitOsdRegion(yuv_frame, osd_region);

	// 将Yuv转换成图片帧
	struct sdc_jpeg_frame_stru jpeg_frame;
	if (JpegFileYuv2Jpeg(yuv_frame, osd_region, jpeg_frame) != OK) {
		LOG_ERROR("Translate Yuv to Jpeg failed");
		return ERR;
	}
	LOG_DEBUG("Translate Yuv to Jpeg successfully,path = %s",jpegPath);

	// 用于测试，保存Jpeg图片帧至本地，如果客户将jpeg_frame上传至平台，不需要保存本地,
	// 只需要将jpeg_frame.addr_virt, jpeg_frame.size的内容封装成TLV数据内容，并上报至平台；
	(void) JpegFileSaveJpeg(jpeg_frame, jpegPath);

	// 释放Jpeg图片帧
	(void) JpegFileFreeJpeg(jpeg_frame);

	return OK;
}

int32_t JpegFileSaveYuv2Jpeg(const struct sdc_yuv_frame_stru *yuv_frame, const char *jpegPath)
{
	struct sdc_yuv_frame_stru yuv_frame_t;
	//string jpegPath_t = String.valueOf(jpegPath);
	memcpy_s(&yuv_frame_t, sizeof(sdc_yuv_frame_stru), yuv_frame, sizeof(sdc_yuv_frame_stru));
	JpegFileSaveYuvToJpeg(yuv_frame_t,jpegPath);
}

#if 0
namespace SdcOSExamples
{
JpegFile::JpegFile(void) : m_codecFd(-1)
{
	LOG_DEBUG("Create the Jpeg file object");
}

JpegFile::~JpegFile(void)
{
	if (m_codecFd != -1) {
		close(m_codecFd);
		m_codecFd = -1;
	}
	
	LOG_DEBUG("Destroy the Jpeg file object");
}

int32_t JpegFile::Init(void)
{
	m_codecFd = open("/mnt/srvfs/codec.iaas.sdc", O_RDWR);
	if (m_codecFd < 0) {
		LOG_ERROR("Open the /mnt/srvfs/codec.iaas.sdc failed, errno: %d, errmsg: %s", errno, strerror(errno));
		return ERR;
	}
	LOG_ERROR("Open the /mnt/srvfs/codec.iaas.sdc successfully");
	
	return OK;
}

int32_t JpegFile::InitOsdRegion(const struct sdc_yuv_frame_stru &yuv_frame, struct sdc_osd_region_stru &osd_region) const
{
	// 中文采用双字节编码
	wchar_t Font[] = {
		0x4e2d, 0x534e, 0x4eba, 0x6c11, 0x5171, 0x5548c, 0x56fd, 0xff0c, 0x6d59, 0x6c5f,   
		0x7701, 0x676d, 0x5dde, 0x5e02, 0x6ee8, 0x6c5f, 0x533a, 0xff0c, 0x6d4b, 0x8bd5, 0xff01
    };

	memset_s(&osd_region, sizeof(osd_region), 0, sizeof(osd_region));
    osd_region.region.x = 0;
    osd_region.region.y = 0;
    osd_region.region.w = yuv_frame.width;
    osd_region.region.h = yuv_frame.height;
	(void)memcpy_s(osd_region.osd.format,sizeof(osd_region.osd.format),
		"fontsize=2;fgcolor=0x111111;fgalpha=120", sizeof("fontsize=2;fgcolor=0x111111;fgalpha=120") - 1);

    (void)memcpy_s(osd_region.osd.content, sizeof(osd_region.osd.content), Font, sizeof(Font));
    osd_region.osd.content_len = sizeof(Font);
	return OK;
}

int32_t JpegFile::Yuv2Jpeg(const struct sdc_yuv_frame_stru &yuv_frame, const struct sdc_osd_region_stru &osd_region, struct sdc_jpeg_frame_stru &jpeg_frame) const
{
	struct sdc_encode_jpeg_param_stru param;
	memset_s(&param, sizeof(param), 0, sizeof(param));
	param.qf = 80;
	param.osd_region_cnt = 1;
	param.region = { 0, 0, yuv_frame.width, yuv_frame.height };
	param.frame = yuv_frame;

	struct sdc_common_head_stru head;
	memset_s(&head, sizeof(head), 0, sizeof(head));
	head.version = SDC_VERSION; // 0x5331
	head.url = SDC_URL_ENCODED_JPEG; // 0x00
	head.method = SDC_METHOD_CREATE;
	head.head_length = sizeof(head);
	head.content_length = sizeof(param) + sizeof(osd_region);
	
	struct iovec iov[3];
	iov[0].iov_base = &head;
	iov[0].iov_len = sizeof(head);
	iov[1].iov_base = &param;
	iov[1].iov_len = sizeof(param);
	iov[2].iov_base = (void *)&osd_region;
	iov[2].iov_len = sizeof(osd_region);

	int32_t nret = writev(m_codecFd, iov, 3);
	if(nret < 0) {
		LOG_ERROR("Write the iovec failed, errno: %d, errmsg: %s", errno, strerror(errno));
		return errno;
	}

	iov[1].iov_base = &jpeg_frame;
	iov[1].iov_len = sizeof(jpeg_frame);
	nret = readv(m_codecFd, iov, 2);
	if(nret < 0) {
		LOG_ERROR("Read the jpeg frame failed, errno: %d, errmsg: %s", errno, strerror(errno));
		return errno;
	}

	if(head.head_length != sizeof(head) || head.content_length != sizeof(jpeg_frame)) {
		LOG_ERROR("Translate the Yuv frame to jpeg frame failed");
		return EIO;
	}
	LOG_DEBUG("Translate the Yuv frame to jpeg frame successfully");
	return OK;
}

int32_t JpegFile::FreeJpeg(struct sdc_jpeg_frame_stru &jpeg_frame) const
{
	struct sdc_common_head_stru head;
	memset_s(&head, sizeof(head), 0, sizeof(head));
	head.version = SDC_VERSION; // 0x5331
	head.url = SDC_URL_ENCODED_JPEG; // 0x00
	head.method = SDC_METHOD_DELETE;
	head.head_length = sizeof(head);
	head.content_length = sizeof(jpeg_frame);

	struct iovec iov[2];
	iov[0].iov_base = (void *)&head;
	iov[0].iov_len = sizeof(head);
	iov[1].iov_base = (void *)&jpeg_frame;
	iov[1].iov_len = sizeof(jpeg_frame);
	
	int32_t nret = writev(m_codecFd, iov, 2);
	if (nret < 0) {
		LOG_ERROR("Write the iovec failed, errno: %d, errmsg: %s", errno, strerror(errno));
	}
	LOG_DEBUG("Write the iovec successfully");

	return OK;
}

int32_t JpegFile::SaveJpeg(const struct sdc_yuv_frame_stru &yuv_frame, const string &jpegPath) const
{
	struct sdc_osd_region_stru osd_region;
	memset_s(&osd_region, sizeof(osd_region), 0, sizeof(osd_region));
	
	// 如果需要添加1个OSD，请初始化InitOsdRegion()，否在注释OSD
	// (void) InitOsdRegion(yuv_frame, osd_region);

	// 将Yuv转换成图片帧
	struct sdc_jpeg_frame_stru jpeg_frame;
	if (Yuv2Jpeg(yuv_frame, osd_region, jpeg_frame) != OK) {
		LOG_ERROR("Translate Yuv to Jpeg failed");
		return ERR;
	}
	LOG_DEBUG("Translate Yuv to Jpeg successfully");

	// 用于测试，保存Jpeg图片帧至本地，如果客户将jpeg_frame上传至平台，不需要保存本地,
	// 只需要将jpeg_frame.addr_virt, jpeg_frame.size的内容封装成TLV数据内容，并上报至平台；
	(void) SaveJpeg(jpeg_frame, jpegPath);

	// 释放Jpeg图片帧
	(void) FreeJpeg(jpeg_frame);

	return OK;
}

int32_t JpegFile::SaveJpeg(const struct sdc_jpeg_frame_stru &jpeg_frame, const string &jpegPath) const
{
	int32_t fd = -1;
	if ((fd=open(jpegPath.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0) {
		LOG_ERROR("Open the jpeg file failed, file_path: %s, errno: %d, errmsg: %s", jpegPath.c_str(), errno, strerror(errno));
		return ERR;
	}

	ssize_t writeLen= 0;
	if ((writeLen=write(fd, (void*)jpeg_frame.addr_virt, jpeg_frame.size)) < 0) {
		LOG_ERROR("Write the jpeg file failed, errno: %d, errmsg: %s", errno, strerror(errno));
	} else if(writeLen != (ssize_t)jpeg_frame.size) {
		LOG_ERROR("Write the jpeg file truncted, write_size: %ld, real_size: %u", writeLen, jpeg_frame.size);
	}else {
		LOG_DEBUG("Save the jpeg file successfully, file_path: %s, real_size: %u", jpegPath.c_str(), jpeg_frame.size);
	}
	close(fd);

	return OK;
}
}

#endif


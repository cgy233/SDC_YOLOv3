/******************************************************************************

                  ��Ȩ���� (C), 2019-2029, SDC OS ��Դ���С������

 ******************************************************************************
  �� �� ��   : main.c
  �� �� ��   : ����
  ��    ��   : jelly
  ��������   : 2019��6��8��
  ����޸�   :
  ��������   : ������
  �����б�   :
                             main
                             sdc_ReadFromVideoService
  �޸���ʷ   :
  1.��    ��   : 2019��6��8��
    ��    ��   : jelly
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#include <inttypes.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

#include <sys/types.h>



#include "main.h"
#include "sdc.h"
#include "sdc_os_api.h"
#include "queue.h"

#include "sample_comm_nnie.h"
#include "label_event.h"

#include "JpegFile.h"

// ������� �����װ�õĿ�
#include "detect_car.h"

// ����ʻ��ʻ�� �궨�����

#define CHANGE 1 

#define DEBUG_LOG 1 
#define JPEGTIME 1

#if DEBUG_LOG
static int g_flag = 0;
void printf_log(char *remark)
{
	FILE *fp = NULL;
	if (g_flag == 0)
	{
		fp = fopen("./test.txt", "w+");
		g_flag = 1;
	}
	else
	{
		fp = fopen("./test.txt", "a+");
	}
	fprintf(fp, "%s\n", remark);
	fclose(fp);
}
#endif

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/
extern int fd_video;
extern int fd_codec;
extern int fd_utils;
extern int fd_algorithm;
extern int fd_event;
extern int fd_cache;
extern unsigned short int trans_id;




/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/
extern unsigned int sleep (unsigned int seconds);


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/
QUEUE_S g_ptrQueue;
#define MAX_OBJ_NUM 10

#if CHANGE
// �й�Buffer�ı�������ȫ�ֱ��� 
int *car_point[D_CAR_NUM];
int front[D_CAR_NUM]; // ǰ������
int back[D_CAR_NUM]; // ��������
int last_frame_num[D_CAR_NUM]; //���1֡���ڵ�buff
int car_exist_flag[D_CAR_NUM]; // �����뿪��־
#endif

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

SYSTEM_MANAGE_PARAS_S g_stSystemManage;
unsigned int g_uiRecvNum = 0;
unsigned int g_uiFreeNum = 0;

unsigned int g_uiRecvPos = 0;
unsigned int g_uiFreePos = 0;


/*ssd para*/
SAMPLE_SVP_NNIE_MODEL_S s_stYoloModel = {0};
SAMPLE_SVP_NNIE_PARAM_S s_stYoloNnieParam = {0};
SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S s_stYolov3SoftwareParam = {0};
SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S s_stSsdSoftwareParam = {0};
SAMPLE_SVP_NNIE_CFG_S   stNnieCfg_SDC = {0};

HI_BOOL BGRInProcessingFlag = HI_FALSE;
HI_CHAR BGRBuffer[304*300*3];
HI_FLOAT thresh = 0.5; 

typedef enum IVS_FRAME_TYPE_E
{
    VW_FRAME_ORIGION = 0,
    VW_FRAME_SCALE_1,
    VW_FRAME_TYPE_MAX,
} VW_IVS_FRAME_TYPE_E;

typedef struct VW_IVSFRAMEDATA
{
    char reverse0[2616];
    VW_YUV_FRAME_S astYuvframe[VW_FRAME_TYPE_MAX];
    char reverse1[272];
} VW_IVSFRAMEDATA_S;


extern int SDC_SVP_ForwardBGR(HI_CHAR *pcSrcBGR, SDC_SSD_RESULT_S *pstResult, SDC_SSD_INPUT_SIZE_S InputSize);



/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define MAX_YUV_BUF_NUM 50 

#define  APP_NAME   "safetyhat"
#define LABEL_EVENT_DATA                 ("tag.saas.sdc")

#define YUVtoJPEG  1 // YUV JPEG �������
 

//excavator_dx

int SDC_LabelEventDel(int fp, unsigned int baseid, unsigned int id, char *cAppName)
{
    int nDataLen;
	int nResult;
	char *pcTemp = NULL;
	
    paas_shm_cached_event_s shm_event;
    sdc_extend_head_s shm_head = {
        .type = SDC_HEAD_SHM_CACHED_EVENT,
        .length = 8,
    };

    sdc_common_head_s head;
    head.version = SDC_VERSION;
    head.url = SDC_URL_PAAS_EVENTD_EVENT;
    head.method = SDC_METHOD_CREATE;
    head.head_length = sizeof(head) + sizeof(shm_head);
    head.content_length = sizeof(shm_event);

    struct iovec iov[3];
    iov[0].iov_base = &head;
    iov[0].iov_len = sizeof(head);
    iov[1].iov_base = &shm_head;
    iov[1].iov_len = sizeof(shm_head);
    iov[2].iov_base = &shm_event;
    iov[2].iov_len = sizeof(shm_event);

    SDC_SHM_CACHE_S shm_cache;
    memset(&shm_cache, 0, sizeof(shm_cache));

    LABEL_EVENT_DATA_S * pevent = NULL;
    nDataLen = sizeof(label);
    shm_cache.size  = sizeof(LABEL_EVENT_DATA_S) + nDataLen;
	shm_cache.ttl = 0;
    //printf("ioctl fail\n");
	
    nResult = ioctl(fd_cache, SDC_CACHE_ALLOC,&shm_cache);
    if(nResult != 0)
    {
        printf("ioctl fail\n");
        goto EVENT_FAIL;
    }
    pevent = (LABEL_EVENT_DATA_S *)shm_cache.addr_virt;
	
    pevent->data = (char *)pevent +sizeof(LABEL_EVENT_DATA_S);
    pevent->base.id = baseid;

    (void)sprintf(pevent->base.name, "%s", LABEL_EVENT_DATA);

	pevent->base.length = nDataLen;
	memset(pevent->data, 0, sizeof(nDataLen));
    pcTemp = pevent->data;

	*(uint32_t *)pcTemp = 0;//add
	pcTemp += sizeof(uint32_t);
	
	strcpy_s(pcTemp, 32, cAppName); //app name
	pcTemp += 32;
	
	*(uint64_t *)pcTemp = id;

	shm_event.addr_phy = shm_cache.addr_phy;	
	shm_event.size = shm_cache.size;	
	shm_event.cookie = shm_cache.cookie;
	nResult = writev(fp, iov, 3);
	if(nResult == -1)
    {
        printf("ioctl fail\n");
        goto EVENT_FAIL;
    }   
	munmap(pevent, sizeof(LABEL_EVENT_DATA_S) + nDataLen);

    return 0;
EVENT_FAIL:
    if(pevent)
    {
        munmap(pevent, sizeof(LABEL_EVENT_DATA_S) + nDataLen);
    }

    return -1;
	
}


int SDC_LabelEventPublish(int fp, unsigned int baseid, int iDataLen, char *cEventMsg, uint64_t pts)
{
	int nResult;
    paas_shm_cached_event_s shm_event;
    sdc_extend_head_s shm_head = {
        .type = SDC_HEAD_SHM_CACHED_EVENT,
        .length = 8,
    };

    sdc_common_head_s head;
    head.version = SDC_VERSION;
    head.url = SDC_URL_PAAS_EVENTD_EVENT;
    head.method = SDC_METHOD_CREATE;
    head.head_length = sizeof(head) + sizeof(shm_head);
    head.content_length = sizeof(shm_event);

    struct iovec iov[3];
    iov[0].iov_base = &head;
    iov[0].iov_len = sizeof(head);
    iov[1].iov_base = &shm_head;
    iov[1].iov_len = sizeof(shm_head);
    iov[2].iov_base = &shm_event;
    iov[2].iov_len = sizeof(shm_event);


    SDC_SHM_CACHE_S shm_cache;
    memset(&shm_cache, 0, sizeof(shm_cache));

    LABEL_EVENT_DATA_S * pevent = NULL;
    //nDataLen= sizeof(label) + uiPolygonNum * sizeof(polygon) + iPointNum * sizeof(car_point) + (uiTagNum - '0')*sizeof(tag) + iStrNum;
    shm_cache.size  = sizeof(LABEL_EVENT_DATA_S) + iDataLen;
    nResult = ioctl(fd_cache, SDC_CACHE_ALLOC,&shm_cache);
    if(nResult != 0)
    {
        printf("ioctl fail\n");
        goto EVENT_FAIL;
    }
    pevent = (LABEL_EVENT_DATA_S *)shm_cache.addr_virt;
	pevent->data = (char *)pevent + sizeof(LABEL_EVENT_DATA_S);
    pevent->base.id = baseid;
	pevent->base.src_timestamp = pts;
	pevent->base.tran_timestamp = pts + 10;/*Ĭ����д*/
    (void)sprintf(pevent->base.name, "%s", LABEL_EVENT_DATA);
	(void)sprintf(pevent->base.publisher, "%s", "test");

	pevent->base.length = iDataLen;
	memcpy_s(pevent->data, iDataLen, cEventMsg, iDataLen);
	
    //printf("length= %d\n",iDataLen);

	shm_event.addr_phy = shm_cache.addr_phy;	
	shm_event.size = shm_cache.size;	
	shm_event.cookie = shm_cache.cookie;	
	nResult = writev(fp, iov, 3);
	if(nResult == -1)
    {
        printf("writev fail\n");
        goto EVENT_FAIL;
    }   
	munmap(pevent, sizeof(LABEL_EVENT_DATA_S) + iDataLen);

    return 0;
EVENT_FAIL:
    if(pevent)
    {
        munmap(pevent, sizeof(LABEL_EVENT_DATA_S) + iDataLen);
    }

    return -1;
	
}

/* ��Ƶ������ֳ�*/
void * SDC_ReadFromVideoService(void *arg)       
{
    //struct member *temp;
    int iReadLen = 0;
    char cMsgReadBuf[2048];
    sdc_common_head_s *pstSdcMsgHead = NULL;
    //sdc_extend_head_s *extend_head  = NULL;
    char *pucSdcYuvData = NULL;
    //sdc_yuv_data_s *pstYuvData = NULL;
    int iQueueState;
    int iQueueStoreNum = 0;
    
    /* �߳�pthread��ʼ���� */
    printf("SDC_ReadFromVideoService pthread start!\n");

    pstSdcMsgHead = (sdc_common_head_s *)cMsgReadBuf;
        
    while(1)
    {
        if (g_stSystemManage.uiSystemState != SYS_STATE_NORMAL)
        {
            usleep(10000);
            continue;
        }

        g_uiRecvPos =  1;

        //printf("*******1111111111******\n");

        iQueueStoreNum = QUE_GetQueueSize(&g_ptrQueue);
        if (iQueueStoreNum > 1)
        {              
            //printf("Recv yuv data iQueueStoreNum:%d!\n", iQueueStoreNum); 
            //usleep(100000);
            continue;
        }
                
        iReadLen = read(fd_video, (void *)cMsgReadBuf, 2048);

        //printf("*******22222222******\n");
        
        g_uiRecvPos = 2;
        
        if (iReadLen < 0)
        {
            fprintf(stdout,"read fail response:%d,url:%d,code:%d, method:%d\n",
                pstSdcMsgHead->response,pstSdcMsgHead->url,pstSdcMsgHead->code, pstSdcMsgHead->method);
            
            continue;
        }


        //fprintf(stdout,"read succeed response:%d,url:%d,code:%d, method:%d, iReadLen:%d\n",
            //pstSdcMsgHead->response,pstSdcMsgHead->url,pstSdcMsgHead->code, pstSdcMsgHead->method, iReadLen);

        switch (pstSdcMsgHead->url)
        {
            case SDC_URL_YUV_CHANNEL:
            
            //fprintf(stdout,"Read Rsp  url:%d\n ",pstSdcMsgHead->url);
            break;
            
            case SDC_URL_YUV_DATA:
 
            
            //fprintf(stdout,"Read Rsp  url:%d, response:%d, content_length:%d\n",
                //pstSdcMsgHead->url, pstSdcMsgHead->response, pstSdcMsgHead->content_length);
 
            if (pstSdcMsgHead->content_length != 0)
            {
                #if 0
                sdc_for_each_extend_head(pstSdcMsgHead, extend_head) 
                {
                    //clock_gettime(CLOCK_BOOTTIME, &time1);
                    //fprintf(stdout,"time33333333333:%lu, %lu\n", time1.tv_sec, time1.tv_nsec);
                    SDC_DisplayExtendHead(extend_head);
                }
                #endif
 
                pucSdcYuvData = &cMsgReadBuf[pstSdcMsgHead->head_length];

                g_uiRecvNum += pstSdcMsgHead->content_length/sizeof(sdc_yuv_data_s);
 
                //pstYuvData = (sdc_yuv_data_s *)pucSdcYuvData;
                //fprintf(stdout,"Recv yuv data channel:%d, pts:%ld frameNum:%d\n ", 
                    //pstYuvData->channel, pstYuvData->pts, pstSdcMsgHead->content_length/sizeof(sdc_yuv_data_s)); 
                iQueueState = QUE_PushQueue(&g_ptrQueue, pucSdcYuvData);                
                if (iQueueState != QUEUE_STATE_OK)
                {
                    printf("QUE_PushQueue State:%d!\n", iQueueState); 

                    SDC_YuvDataFree(fd_video, (sdc_yuv_data_s *)pucSdcYuvData);
                    g_uiFreeNum++;
                    usleep(5000);
                    
                }
                else
                {

                }
            }
            else
            {
                //
            }
 
            break;
            
            case SDC_URL_VENC_DATA:
            fprintf(stdout,"Read Rsp  url:%d\n ",pstSdcMsgHead->url);
            break;
            
            case SDC_URL_YUV_SNAP:
            fprintf(stdout,"Read Rsp  url:%d\n ",pstSdcMsgHead->url);
            break;
            
            case SDC_URL_RED_LIGHT_ENHANCED:
            fprintf(stdout,"Read Rsp  url:%d\n ",pstSdcMsgHead->url);
            break;
            
            default:
            {
                fprintf(stdout,"Read From Video Services ,unknow url:%d\n ",pstSdcMsgHead->url);
            }
            break;
        }
        g_uiRecvPos = 3;

    }
                                         
    return NULL;                         
}

/* YUV����֡����*/
void * SDC_YuvDataProc(void *arg)       
{
#if CHANGE
	// ����ʻ��ʻ�� �ֲ���������
	int cur_front = 0;
	int cur_back = 0;
	int cur_data_len = 0;

	int forward_time = FORWARD_TIME; //ms ���1֡��ʱ
	int wait_time = WAIT_TIME;//1 * 60 * 1000; //ms �ȴ���ʱ��
	int one_car_pint_num =  0;
	int cur_car_num = 0;//��ǰ�����г�����Ŀ��
	// int i = 0; // ԭ�����Ѿ��к�������i 505��
	int j = 0;
	int x_middle = 0;
	int y_middle = 0;
	int middle_data = 0;

	int full_flag = 0;
	int buffer_num = 0;


	int incr_type_cnt = 0;
	int no_of_type; // ��ǰ֡��Ŀ������
	int data_num = 0;
	int cur_data = 0;
	int frame_num = 0;


	int min_value = 0;
	int distance = 0;
	int select_buffer_num = 0;
	int car_in_flag = 0;
	int all_frame_num = 0;

	char remark[256];
#endif
#if JPEGTIME
    char jpegchar[100];
    time_t t;
    struct tm * lt;
#endif
    sdc_yuv_data_s stSdcYuvData;
    sdc_yuv_data_s stSdcYuvfram;
    int iQueueState;
    SDC_SSD_INPUT_SIZE_S InputSize;
	SDC_SSD_RESULT_S stResult;
    sdc_yuv_frame_s sdcRgb;
    VW_YUV_FRAME_S rgbimg; 
    int iRetCode = OK;
	UINT32 idx = 0; 
	uint64_t pts;
    struct timespec time1 = {0, 0};
    //struct timespec time2 = {0, 0};
    struct timespec time3 = {0, 0};
    unsigned int uiTimeCout = 0;

	int i = 0;
	int iLength = 0;
	int iObjectNum = 0;
	META_INFO_S astMetaInfo[10] = {0};
	char cLabelSendBuf[4096] = {0};
	char *pcTemp = NULL;
	int iTagLen = 0;
	char auTempBuf[32]={0};

#ifdef YUVtoJPEG
	//������ȡjpegͼƬ�ӿ�--yjh0119
	//JpegFileInit();
#endif

#if CHANGE
	// ����ʻ��ʻ���������ڴ�, ����ʼ���������ٶ��Ѿ��յ����Ƶ������źţ���ʼ��¼�����켣

	//��⵽һ����ʱ�������ڴ�洢�ڴ����ݡ�
	one_car_pint_num =  (int)(wait_time/forward_time);

	//һ������������꣬��16bit��y�ӱ꣬��16bit��x���ꡣ
	 for(i=0;i<D_CAR_NUM; i++)
	 {
		 printf("Call Malloc.\n");
		 car_point[i] = (int *)malloc( one_car_pint_num * sizeof(int) );
		 if( car_point[i] == NULL )
		 {
			 fprintf(stderr, "Error - unable to allocate required memory\n");
		 }
	 }

	//���вο��� http://data.biancheng.net/view/173.html
    //���ö�ͷָ��Ͷ�βָ�룬��������û��Ԫ��ʱ����ͷ�Ͷ�βָ��ͬһ���ַ
	 for(i=0;i<D_CAR_NUM; i++)
	 {
		front[i] = 0;
		back[i] = 0;
		car_exist_flag[i] = 0;
		last_frame_num[i] = 0;
	 }

	 //x_middle = 580;
	 //y_middle = 470;

	 //���軭������һ����
	 //ͨ������ʶ�𣬱���õ�һ������һ�����������ꡣ
	 //buffer_num = get_car_buffer_num(car_exist_flag);
	 //printf("buffer %d\n", buffer_num);
	 // ���Ƶ�����ת��Ϊ����
	 //���ݳ�������תΪ���ĵ㣬��ʼ׷�����ĵ�����
	 //middle_data = get_middle_data(x_middle, y_middle); // ��16��x�� ��16��y�� ������������
	 //trigger_car_track(middle_data, buffer_num, one_car_pint_num, front, back, car_exist_flag, car_point[buffer_num]);
	 //cur_car_num = cur_car_num + 1;

	 //��һ����
	 // buffer_num = get_car_buffer_num(car_exist_flag);
	 // x_middle = mid_x[1];
	 // y_middle = mid_y[1];
	 // middle_data = get_middle_data(x_middle, y_middle);
	 // trigger_car_track(middle_data, buffer_num, one_car_pint_num, front, back, car_exist_flag, car_point[buffer_num]);
	 // cur_car_num = cur_car_num + 1;
#endif
    printf("SDC_YuvDataProc pthread start!\n"); 

     /* ���س�ʼ��ģ��*/

    while(1)
    {
        time(&t);
        lt = localtime(&t);
        if (g_stSystemManage.uiSystemState != SYS_STATE_NORMAL)
        {
            //usleep(10000);
            continue;
        }

        g_uiFreePos = 1;
        
        /*�Ӷ�����ȡ��һ֡���ݽ��д���*/
        iQueueState = QUE_PopQueue(&g_ptrQueue, (char *)&stSdcYuvData);
        if (iQueueState != QUEUE_STATE_OK)
        {    
            //printf("QUE_PopQueue State:%d!\n", iQueueState); 
            //usleep(500000);
            continue;
        }
        else
        {
            //printf("get frame with:%d !\n", stSdcYuvData.frame.width);
            if (stSdcYuvData.frame.width != 416)
            {
                /*ʹ������ͷ�YUV ����*/
                SDC_YuvDataFree(fd_video, &stSdcYuvData);
                g_uiFreeNum++;
                printf("get frame with:%d != 416 !\n", stSdcYuvData.frame.width);
                continue;
            }
            stSdcYuvfram=stSdcYuvData;

            clock_gettime(CLOCK_BOOTTIME, &time1);            

            iRetCode = SDC_TransYUV2RGB(fd_algorithm, &(stSdcYuvData.frame), &sdcRgb);

			pts = stSdcYuvData.pts;			

                                    // sprintf(jpegchar,"/tmp/%d-%d-%d-%d-%d.jpeg",1+lt->tm_mon,lt->tm_mday,lt->tm_hour,lt->tm_min,lt->tm_sec);
                                    // JpegFileSaveYuv2Jpeg(&stSdcYuvfram.frame, jpegchar);
            g_uiFreeNum++;     
            
            if (iRetCode != OK)
            {
                printf("Err in SDCIveTransYUV2RGB!\n");
                continue;
            }

            SDC_Struct2RGB(&sdcRgb, &rgbimg);
            //clock_gettime(CLOCK_BOOTTIME, &time2);
            //uiTimeCout = (unsigned int)(time2.tv_sec - time1.tv_sec)*1000 + (unsigned int)((time2.tv_nsec - time1.tv_nsec)/1000000);
            //fprintf(stdout,"time111111:%d\n", uiTimeCout);
            
                
            InputSize.ImageWidth = 416;
            InputSize.ImageHeight = 416;
 
            stResult.numOfObject = 10;
            stResult.thresh = thresh;
            stResult.pObjInfo = (SDC_SSD_OBJECT_INFO_S *)malloc(stResult.numOfObject * sizeof(SDC_SSD_OBJECT_INFO_S));
 
            if (SDC_SVP_ForwardBGR(rgbimg.pYuvImgAddr, &stResult, InputSize) != OK)
            {
                printf("Err in SDC_SVP_ForwardBGR!\n");
                continue;
            }

            //clock_gettime(CLOCK_BOOTTIME, &time3);
            //uiTimeCout = (unsigned int)(time3.tv_sec - time2.tv_sec)*1000 + (unsigned int)((time3.tv_nsec - time2.tv_nsec)/1000000);
            //fprintf(stdout,"time2222222:%d\n", uiTimeCout);

                /*print result, this sample has 21 classes:
                             class 0:background     class 1:plane           class 2:bicycle
                             class 3:bird           class 4:boat            class 5:bottle
                             class 6:bus            class 7:car             class 8:cat
                             class 9:chair          class10:cow             class11:diningtable
                             class 12:dog           class13:horse           class14:motorbike
                             class 15:person        class16:pottedplant     class17:sheep

                             class 18:sofa          class19:train           class20:tvmonitor*/
            #if 1
			/*�������Ԫ����*/
			SDC_LabelEventDel(fd_event, 0, 0, APP_NAME);
			#endif
			
			idx = 0;	
			memset_s(astMetaInfo,sizeof(META_INFO_S) * 10, 0, sizeof(META_INFO_S) * 10);
            for(i = 0; i < stResult.numOfObject; i++)
            {
                if(stResult.pObjInfo[i].confidence > thresh)
                {
	                if(stResult.pObjInfo[i].x_left < 0) stResult.pObjInfo[i].x_left = 0;
	                if(stResult.pObjInfo[i].y_top < 0) stResult.pObjInfo[i].y_top = 0;
	                if(stResult.pObjInfo[i].w < 0) stResult.pObjInfo[i].w = 0;
	                if(stResult.pObjInfo[i].h < 0) stResult.pObjInfo[i].h = 0;
		
                    #if 1
                    printf("Object[%d] class[%u] confidece[%f] {%03d, %03d, %03d, %03d, %03d, %03d}\n", \
                        i, stResult.pObjInfo[i].class, stResult.pObjInfo[i].confidence, \
                        stResult.pObjInfo[i].x_left * 1920/InputSize.ImageWidth, \
                        stResult.pObjInfo[i].y_top * 1080/InputSize.ImageHeight, \
                        stResult.pObjInfo[i].x_right * 1920/InputSize.ImageWidth, \
                        stResult.pObjInfo[i].y_bottom * 1080/InputSize.ImageHeight, \
                        stResult.pObjInfo[i].w * 1920/InputSize.ImageWidth, \
                        stResult.pObjInfo[i].h * 1080/InputSize.ImageHeight);
                    //printf("center");
                    //printf("%03d,%03d\n",(stResult.pObjInfo[i].x_left * 1920/InputSize.ImageWidth+ stResult.pObjInfo[i].x_right * 1920/InputSize.ImageWidth)/2,
					//(stResult.pObjInfo[i].y_top * 1080/InputSize.ImageHeight+stResult.pObjInfo[i].y_bottom * 1080/InputSize.ImageHeight)/2);
					//#else
					astMetaInfo[idx].uclass = stResult.pObjInfo[i].class;
					astMetaInfo[idx].usX = stResult.pObjInfo[i].x_left*10000/InputSize.ImageWidth;
					astMetaInfo[idx].usY = stResult.pObjInfo[i].y_top*10000/InputSize.ImageHeight;
					astMetaInfo[idx].usWidth = stResult.pObjInfo[i].w*10000/InputSize.ImageWidth;
					astMetaInfo[idx].usHeight = stResult.pObjInfo[i].h*10000/InputSize.ImageHeight;
					astMetaInfo[idx].confidence = stResult.pObjInfo[i].confidence;	
					#endif	
					
#if CHANGE
					// �������ĵ�����
					// (stResult.pObjInfo[i].x_left * 1920/InputSize.ImageWidth) // left
					// (stResult.pObjInfo[i].y_top * 1080/InputSize.ImageHeight) // top
					// (stResult.pObjInfo[i].x_right * 1920/InputSize.ImageWidth) // right 
					// (stResult.pObjInfo[i].y_bottom * 1080/InputSize.ImageHeight)// bottom
					if (x_middle == 0)
					{
#if NEW_DETECT_F
						// �������ص�����
						x_middle = (int)(((float)((stResult.pObjInfo[i].x_left * 1920/InputSize.ImageWidth) + (stResult.pObjInfo[i].x_right * 1920/InputSize.ImageWidth)))/2);
						y_middle = (int)(((float)((stResult.pObjInfo[i].y_top * 1080/InputSize.ImageHeight) + (stResult.pObjInfo[i].y_bottom * 1080/InputSize.ImageHeight)))/2);
#else
						x_middle = (int)(((float)((stResult.pObjInfo[i].x_left * 1920/InputSize.ImageWidth) + (stResult.pObjInfo[i].x_right * 1920/InputSize.ImageWidth)))/2.05);
						y_middle = (int)(((float)((stResult.pObjInfo[i].y_top * 1080/InputSize.ImageHeight) + (stResult.pObjInfo[i].y_bottom * 1080/InputSize.ImageHeight)))/1.9);
#endif

						 //���軭������һ����
						 //ͨ������ʶ�𣬱���õ�һ������һ�����������ꡣ
						 buffer_num = get_car_buffer_num(car_exist_flag);
						 printf("buffer %d\n", buffer_num);

						 // ���Ƶ�����ת��Ϊ����
						 //���ݳ�������תΪ���ĵ㣬��ʼ׷�����ĵ�����
						 middle_data = get_middle_data(x_middle, y_middle); // ��16��x�� ��16��y�� ������������
						 trigger_car_track(middle_data, buffer_num, one_car_pint_num, front, back, car_exist_flag, car_point[buffer_num]);
						 cur_car_num = cur_car_num + 1;
					}

#if NEW_DETECT_F
						x_middle = (int)(((float)((stResult.pObjInfo[i].x_left * 1920/InputSize.ImageWidth) + (stResult.pObjInfo[i].x_right * 1920/InputSize.ImageWidth)))/2);
						y_middle = (int)(((float)((stResult.pObjInfo[i].y_top * 1080/InputSize.ImageHeight) + (stResult.pObjInfo[i].y_bottom * 1080/InputSize.ImageHeight)))/2);
#else
						x_middle = (int)(((float)((stResult.pObjInfo[i].x_left * 1920/InputSize.ImageWidth) + (stResult.pObjInfo[i].x_right * 1920/InputSize.ImageWidth)))/2.05);
						y_middle = (int)(((float)((stResult.pObjInfo[i].x_right * 1920/InputSize.ImageWidth) + (stResult.pObjInfo[i].y_bottom * 1080/InputSize.ImageHeight)))/1.9);
#endif
					printf("x_middle: %d\t", x_middle);
					printf("y_middle: %d\n", y_middle);
					// DEBUG
#if DEBUG_LOG
					sprintf(remark, "x_middle: %d, x_middle: %d\n", x_middle, y_middle);
					printf_log(remark);
#endif
					// ѹ������ ��16 X ��16 Y
					middle_data = get_middle_data(x_middle, y_middle);

					//mid_d_x = middle_data &0xffff;
					//mid_d_y = (middle_data>>16) &0xffff;	

					min_value = 4096*4096; //����һ��������
					
					// ��buffer����Ƚ���Сֵ Ȼ�����
					for(i=0; i < D_CAR_NUM; i++)
					{
						if (car_exist_flag[i] == 1)
						{
							cur_front = front[i];
							cur_back = back[i];
							cur_data =  get_cur_data_queue(car_point[i], cur_front, one_car_pint_num);
							distance = calc_distant(middle_data, cur_data);
							printf("distance: %d\n", distance);
#if DEBUG_LOG
					sprintf(remark, "distance: %d\n", distance);
					printf_log(remark);
#endif

							if (distance < min_value)
							{
								min_value  = distance;
								select_buffer_num = i;
							}
						}
					}
					if (min_value == 4096*4096)
					{
						printf("[error] : cannot find min_value\n");
					}
					if (no_of_type == 3){
						printf("min_value: %d\n", min_value);
					}
					//��֡�����ƶ������ֵ����100���ص㣬��Ϊ��ЧĿ�궪��
					if (min_value < OBJECT_MIN_DISTANCE*OBJECT_MIN_DISTANCE)
					{
						//find which car
						put_mid_data_buffer(middle_data, select_buffer_num, front, back, one_car_pint_num, car_point[select_buffer_num]);
						last_frame_num[select_buffer_num] = frame_num;
					}
#endif
                    idx++;
					//break;
                }
            }
			iObjectNum = idx;
            //DisplayChannelData(pts, stResult.numOfObject);            
            memset_s(cLabelSendBuf,4096, 0, 4096);

            if (iObjectNum > 0)
        	{
				pcTemp = cLabelSendBuf;
				*(uint32_t *)pcTemp = 1;//add
				pcTemp += sizeof(uint32_t);
				
				strcpy_s(pcTemp, 32, APP_NAME); //app name
				pcTemp += 32;
				
				*(uint64_t *)pcTemp = 0;//id
				pcTemp += sizeof(uint64_t);

				*(uint16_t *)pcTemp = iObjectNum;//polygon_cnt
				pcTemp += sizeof(uint16_t);

				for (i=0; i < iObjectNum; i++)
				{
					*(int32_t *)pcTemp = 0xFF0000;//color = 345455;
					pcTemp += sizeof(int32_t);

					*(int32_t *)pcTemp = 5;//edge_width = 3;
					pcTemp += sizeof(int32_t);

					*(uint32_t *)pcTemp = 0;//pstPolygon->attr = 1;
					pcTemp += sizeof(uint32_t);

					*(int32_t *)pcTemp = 0xFF0000;//pstPolygon->bottom_color = 345455;
					pcTemp += sizeof(int32_t);

					*(int32_t *)pcTemp = 0;//pstPolygon->transparency = 128;
					pcTemp += sizeof(int32_t);

					*(int32_t *)pcTemp = 4;//pstPolygon->iPointcnt = 4;
					pcTemp += sizeof(int32_t);

					
                    /*�����Ǿ��ο�����*/
					*(uint32_t *)pcTemp = astMetaInfo[i].usX;//x1;
					pcTemp += sizeof(uint32_t);

					*(uint32_t *)pcTemp = astMetaInfo[i].usY;//y1;
					pcTemp += sizeof(uint32_t);

					*(uint32_t *)pcTemp = astMetaInfo[i].usX;//x2;
					pcTemp += sizeof(uint32_t);

					*(uint32_t *)pcTemp = astMetaInfo[i].usY + astMetaInfo[i].usHeight;//y2;
					pcTemp += sizeof(uint32_t);

					*(uint32_t *)pcTemp = astMetaInfo[i].usX + astMetaInfo[i].usWidth;//x3;
					pcTemp += sizeof(uint32_t);

					*(uint32_t *)pcTemp = astMetaInfo[i].usY + astMetaInfo[i].usHeight;//y3;
					pcTemp += sizeof(uint32_t);

					*(uint32_t *)pcTemp = astMetaInfo[i].usX + astMetaInfo[i].usWidth;//x4;
					pcTemp += sizeof(uint32_t);

					*(uint32_t *)pcTemp = astMetaInfo[i].usY;//y4;
					pcTemp += sizeof(uint32_t);
				}

				/*��tag_cnt ��ֵ��һ��Ŀ�������ִ������ŶȺ�Ŀ��*/ 
				//iObjectNum = 1;
				*pcTemp = iObjectNum * 2;//tag_cnt
				pcTemp++;
				//printf("\n***********************************");				
				for (i=0; i < iObjectNum; i++)
				{
					*(int32_t *)pcTemp = 0xFF0000;//color = 345455;
					pcTemp += sizeof(int32_t);
					
					strcpy_s(pcTemp, 32, "����"); //font
					pcTemp += 32;

					*(int32_t *)pcTemp = 16;//size
					pcTemp += sizeof(int32_t);

					*(uint32_t *)pcTemp = astMetaInfo[i].usX + 25;//pos-x
					pcTemp += sizeof(uint32_t);

					*(uint32_t *)pcTemp = astMetaInfo[i].usY + 25;//pos-y
					pcTemp += sizeof(uint32_t);	

					switch(astMetaInfo[i].uclass)
					{
					    case 1:
						iTagLen = 20;//sizeof("��ȫñ");
						*(uint32_t *)pcTemp = iTagLen;//len
					    pcTemp += sizeof(uint32_t);				 	
								
					    strcpy_s(pcTemp, 127, "car"); 
						pcTemp += iTagLen;
                        printf("car\n");
						
						break;
						
						
						default:
						iTagLen = 20;//sizeof("δ֪");
						*(uint32_t *)pcTemp = iTagLen;//len
					    pcTemp += sizeof(uint32_t);				 	
								
					    strcpy_s(pcTemp, 127, "unknown"); 
						pcTemp += iTagLen;
						printf("unknow\n");
						//printf(stderr, "class:%d\r\n",stResult.pObjInfo[i].class);
						break;
					}
                    
					*(uint32_t *)pcTemp = 0xFF0000;//color = 345455;
					pcTemp += sizeof(uint32_t);
					
					strcpy_s(pcTemp, 32, "����"); //font
					pcTemp += 32;

					*(uint32_t *)pcTemp = 16;//size
					pcTemp += sizeof(uint32_t);

					*(uint32_t *)pcTemp = astMetaInfo[i].usX + 25;//pos-x
					pcTemp += sizeof(uint32_t);


					*(uint32_t *)pcTemp = astMetaInfo[i].usY + 300;//pos-y
					pcTemp += sizeof(uint32_t);		
					
					/*���������Ŷ�����*/

					memset_s(auTempBuf, 32,0,32);
					sprintf(auTempBuf, "���Ŷ�: %2.2f%%", (float)astMetaInfo[i].confidence*100);
					
					iTagLen = sizeof(auTempBuf);
					*(uint32_t *)pcTemp = iTagLen;//len
				    pcTemp += sizeof(uint32_t);				 	
							
				    strcpy_s(pcTemp, 32, auTempBuf); 
					pcTemp += iTagLen;		
				}

				/*��ʱʱ��*/
				*pcTemp = 1;
				pcTemp++;
				iLength = (int)(pcTemp - cLabelSendBuf);

				iRetCode = SDC_LabelEventPublish(fd_event, 0, iLength, cLabelSendBuf, pts);
				if (iRetCode != OK)
	            {
	                printf("Err in SDC_LabelEventPublish!\n");
	            }
        	}

            //printf("\nscesss!!!   stResult.numOfObject==%d   \n",stResult.numOfObject);
            free(stResult.pObjInfo);

            (void)SDC_TransYUV2RGBRelease(fd_algorithm, &sdcRgb);

#if CHANGE
#if DEBUG_LOG
			// ��ӡ��ǰ֡
			sprintf(remark, "Frame: %d", frame_num);
			printf_log(remark);
#endif
			// ÿ������һ֡���ж��Ƿ�Ҫ����Check Buffer.
			if (((frame_num+ 1) % CHECK_CAR_IN_TIME) == 0)//ÿ��һ���������ڣ����һ�³���״̬�����֣�һ����ͣ�ڳ�λ���棬����һ���ǳ����뿪��
			{
				for(i=0; i < D_CAR_NUM; i++)
				{
					if (car_exist_flag[i] == 1)
					{
						if (check_last_frame(last_frame_num[i], frame_num))//���뿪�����buffer
						{
#if DEBUG_LOG
							sprintf(remark, "frame: %d,  flag1: %d\n",frame_num,last_frame_num[i]);
							printf_log(remark);
#endif
							front[i] = 0;
							back[i] = 0;
							car_exist_flag[i] = 0;
							last_frame_num[i] = 0;
						}
						else
						{
							//��⳵�Ƿ��ڳ�λ��,�ȿ��ٶ�ͨ��5֡��ȷ���Ƿ��ڳ�λ������ڳ�λ�У���Ҫȷ��buffer��ÿһ�����ڳ�λ�У�Ȼ���������־��
							cur_front = front[i];
							cur_back = back[i];
							car_in_flag = check_car_weather_in(car_point[i], cur_front, cur_back, one_car_pint_num);
#ifdef YUVtoJPEG
                            if(car_in_flag == 1){
                                    // ����Yuv��Ƶ֡ΪJpegͼƬ���������ڱ��ز���ʹ��--yjh0119
                                    sprintf(jpegchar,"/tmp/%d-%d-%d-%d-%d.jpeg",1+lt->tm_mon,lt->tm_mday,lt->tm_hour,lt->tm_min,lt->tm_sec);
                                    JpegFileSaveYuv2Jpeg(&stSdcYuvfram.frame, jpegchar);
                            }
#endif
#if DEBUG_LOG
							sprintf(remark, "frame: %d, flag2: %d",frame_num, car_in_flag);
							printf_log(remark);
#endif
						}
					}
				}
			}
			frame_num++;
#endif
        }

       
        g_uiFreePos = 5;
		
        clock_gettime(CLOCK_BOOTTIME, &time3);
        uiTimeCout = (unsigned int)(time3.tv_sec - time1.tv_sec)*1000 + (unsigned int)((time3.tv_nsec - time1.tv_nsec)/1000000);
        
        fprintf(stdout,"forward_time:%d\n", uiTimeCout);
        SDC_YuvDataFree(fd_video, &stSdcYuvData);
 
        //fprintf(stdout,"SDC used complete YuvDataFree\n "); 
    }
#if CHANGE
		// �ͷ�ָ��
		for(i=0;i<D_CAR_NUM; i++)	
		{
			free(car_point[i]);
		}
#endif
   
    return NULL;                         
}                                        

                                   
/* main���� */                             
int main(int argc, char *argv[])
{
	int nret;
	sdc_hardware_id_s stHardWareParas;
	unsigned int uiYuvChnId = 1;
	
	pthread_t tidpRecv;    
    pthread_t tidpProc;
    struct member *b; 
    char *pcModelName = "./car_detection_test01_inst.wk";//"./nnie_dingxin_deploy_picked_5000_loss_2_0_chip.wk";
    //char *pcModelName = "./FaceEnhance.wk";

    g_stSystemManage.uiSystemState = SYS_STATE_IDLE;
    g_stSystemManage.uiMaxUsedBufNum = 10;

    /*���ļ���������*/
	nret = SDC_ServiceCreate();
	if(nret) 
    {
        fprintf(stderr, "nret:%d Err in SDC_ServiceCreate!\r\n", nret);
        return ERR;  
    }
	
    nret = QUE_CreateQueue(&g_ptrQueue, MAX_YUV_BUF_NUM);
    if(nret != OK) 
    {
        fprintf(stderr, "Err in QUE_CreateQueue!\r\n");
        return ERR;  
    }

    /*��ȡID��Ϣ*/
    if (SDC_GetHardWareId(&stHardWareParas) == OK) 
    {
        fprintf(stdout,"SDC_GetHardWareId Succeed: %s\n",stHardWareParas.id);
    }
    else
    {
        fprintf(stdout,"Err in SDC_GetHardWareId\n");
        return ERR;
    }

	/*��̬��ȡһ��yuvͨ������APPִ��ʱ��Ҫ����ռ�õ�ͨ��*/
	if (SDC_YuvChnAttrGetIdleYuvChn(fd_video, &uiYuvChnId) != 0)
    {
        fprintf(stdout,"Err in SDC_YuvChnAttrGetIdleYuvChn\n");
        return ERR;
    }

    if (SDC_YuvChnAttrSet(fd_video, uiYuvChnId) != OK)
    {
        fprintf(stdout,"Err in SDC_YuvChnAttrSet\n");
        return ERR;
    }


    /*Ssd Load model*/
    fprintf(stdout, "Yolo Load model!\n");   
    nret = SDC_LoadModel(1, pcModelName,&s_stYoloModel.stModel);//SDC_LoadModel_test
    if(nret != OK) 
    {
        fprintf(stdout, "Err in SDC_LoadModel!\r\n");
        return ERR;  
    }
    
    stNnieCfg_SDC.u32MaxInputNum = 1; //max input image num in each batch
    stNnieCfg_SDC.u32MaxRoiNum = 0;
    stNnieCfg_SDC.aenNnieCoreId[0] = SVP_NNIE_ID_0;//set NNIE core

    fprintf(stdout, "Yolo parameter initialization!\n");
    
    s_stYoloNnieParam.pstModel = &s_stYoloModel.stModel;



    nret = SAMPLE_SVP_NNIE_Yolov3_ParamInit(fd_utils, &stNnieCfg_SDC,&s_stYoloNnieParam,&s_stYolov3SoftwareParam);
    
    // printf("\nnumclass====%d\n",s_stYolov3SoftwareParam.stClassRoiNum.unShape.stWhc.u32Width);
    if(nret != OK) 
    {
        fprintf(stdout, "Error,SAMPLE_SVP_NNIE_yolov3_ParamInit failed!\n");
        return ERR;  
    }
   
    
	/* Ϊ�ṹ�����b��ֵ */                      
	b = (struct member *)malloc(sizeof(struct member));           
	b->num=1;                            
	b->name="SDC_ReadFromVideoService";  


    g_stSystemManage.uiSystemState = SYS_STATE_STARTING;
					  
    /* �����̴߳���YUV����*/  
	
	if ((pthread_create(&tidpProc, NULL, SDC_YuvDataProc, (void*)b)) == ERR)
	{                                    
		 fprintf(stderr, "create error!\n");       
		 return ERR;                        
	}
	

    /* �����̶߳���Ƶ�����߳� */                    
	if ((pthread_create(&tidpRecv, NULL, SDC_ReadFromVideoService, (void*)b)) == ERR)
	{                                    
		 fprintf(stderr, "create error!\n");       
		 return ERR;                        
	}  
									  
	/* ���߳�pthread������ */                  
	sleep(2);
    g_stSystemManage.uiSystemState = SYS_STATE_NORMAL;
    
    if (SDC_YuvDataReq(fd_video, 2, uiYuvChnId, g_stSystemManage.uiMaxUsedBufNum) != OK)
    {
        fprintf(stdout,"Err in SDC_YuvDataReq\n");
        return ERR;
    }
									  
	while(1)
	{
	    fprintf(stderr, "Main Work normal,RecvNum:%d, FreeNum:%d,RecvPos:%d, FreePos:%d\n", 
            g_uiRecvNum, g_uiFreeNum, g_uiRecvPos, g_uiFreePos);  

        fprintf(stderr, "FrameNum:%d\r\n",QUE_GetQueueSize(&g_ptrQueue));
        sleep(5);  
		//PublishTest();
		sleep(5);
		SDC_LabelEventDel(fd_event, 2, 0, APP_NAME);
		fprintf(stdout, "test!\n");
	}
    
	return 0;
} 

 

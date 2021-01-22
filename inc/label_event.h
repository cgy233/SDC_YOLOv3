/******************************************************************************

                  ��Ȩ���� (C), 2001-2011, ��Ϊ�������޹�˾

 ******************************************************************************
  �� �� ��   : label_event.h
  �� �� ��   : 
  ��    ��   : 
  ��������   : 
  ����޸�   :
  ��������   : 
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 
    ��    ��   : 
    �޸�����   : 

******************************************************************************/
#ifndef __LABEL_EVENT_H__
#define __LABEL_EVENT_H__

#include <stdint.h>
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */
//����
typedef struct
{
    uint32_t x;
    uint32_t y;
}point;
//�����
typedef struct
{
    int32_t color;//rgb
    int32_t edge_width;//0-default
    uint32_t attr;//0x01-�Ƿ�����ɫ
    int32_t bottom_color; //��ɫ��ɫ
    int32_t transparency; //��ɫ͸����
    int32_t iPointcnt;//��ĸ��������ֻ�����һ��
    point points[0];
}polygon;
//���ֱ�ע
typedef struct
{
    int32_t color;//rgb
    char font[32];//������Ⱦ����������
    int32_t size;//���ִ�С
    point pos;//�������Ͻ�λ��
    int32_t len;
    char str[0];
}tag;
//��ע�¼�
typedef struct
{
    uint32_t add_flag; //��� || ɾ��
    char app_name[32];
    uint64_t id;
    uint16_t polygon_cnt;
    polygon polygons[0];
    uint8_t tag_cnt;
    tag tags[0];
    uint8_t ttl;//default 1 second
}label;

#ifdef __cplusplus
#if __cplusplus
}
#endif 
#endif/* End of #ifdef __cplusplus */

#endif  /* __LABEL_EVENT_H__ */



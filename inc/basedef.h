#ifndef __BASE_DEFINED_H__
#define __Base_DEFINED_H__
#ifdef __cplusplus
extern "C" {
#endif    

#define LOG_DEBUG(fmt, arg...) do { \
    fprintf(stdout, "[%s][%04d][%s]" fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##arg); \
} while(0)

#define LOG_ERROR(fmt, arg...) do { \
    fprintf(stderr, "[%s][%04d][%s]" fmt "\n",  __FILE__, __LINE__, __FUNCTION__, ##arg); \
} while(0)

#define OK      0
#define ERR     -1

#ifdef __cplusplus
}
#endif   
#endif /* __BASE_DEFINED_H__ */
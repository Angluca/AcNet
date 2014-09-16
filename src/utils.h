#ifndef  __UTILS_H__
#define  __UTILS_H__
#include <stdio.h>
#include <stdlib.h>

#ifndef	NDEBUG
#define ERROR_MSG(format,...) do{\
	fprintf(stderr, "[ERROR|%s:%d>%s()<<"#format"]\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
}while(0);
#else
#define ERROR_MSG(format,...) do{\
	fprintf(stderr, "[ERROR|%s:%d>%s()<<"#format"]\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
}while(0);
#endif

#define PRINT_MSG(format,...) do{\
	fprintf(stderr,"["format"]\n",##__VA_ARGS__);\
}while(0);

#ifndef	NDEBUG
#define  DEBUG_MSG(format,...) do{\
	fprintf(stderr, "[DEBUG|%s:%d>%s()<<"#format"]\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
}while(0);
#else
#define  DEBUG_MSG(format,...)
#endif

#define  EXIT_APP(format,...) do{\
	fprintf(stderr, "[ERROR|%s:%d>%s()<<"#format"]\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
	exit(-1);	\
}while(0);

#ifdef __cplusplus
extern "C" {
#endif
    int HexStrToInt(const char* str);
    void print_log(const char* str);
#ifdef __cplusplus
}
#endif
//
//#ifndef	NDEBUG
//#define ERROR_MSG(format,...) {\
//char buf[65536]; \
//sprintf(buf, "[ERROR|%s:%d>%s()<<"#format"]\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
//print_log(buf);\
//}
//#else
//#define ERROR_MSG(format,...) {\
//char buf[65536]; \
//sprintf(buf, "[ERROR|%s:%d>%s()<<"#format"]\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
//print_log(buf);\
//}
//#endif
//
//#define PRINT_MSG(format,...) {\
//char buf[65536]; \
//sprintf(buf, "["format"]\n",##__VA_ARGS__);\
//print_log(buf);\
//}
//
//#ifndef	NDEBUG
//#define  DEBUG_MSG(format,...){\
//char buf[65536]; \
//sprintf(buf, "[DEBUG|%s:%d>%s()<<"#format"]\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
//print_log(buf);\
//}
//#else
//#define  DEBUG_MSG(format,...)
//#endif
//
//#define  EXIT_APP(format,...) {\
//char buf[65536]; \
//sprintf(buf, "[ERROR|%s:%d>%s()<<"#format"]\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
//print_log(buf);\
//exit(-1);	\
//}

#endif  /*__UTILS_H__*/

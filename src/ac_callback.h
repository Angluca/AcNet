#ifndef  __AC_CALLBACK_H__
#define  __AC_CALLBACK_H__
/**
 * @file ac_callback.h
 * @author Angluca
 * @date 2012-12-26
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ac_callback_func)(int,uint16_t,void*, int ret);
typedef void (*callback_extra_free_function)(void*);

struct ac_callback
{
	int fd;
	uint16_t id;
	void *extra;
	ac_callback_func callback;
	callback_extra_free_function extra_free_func;
};

/* if use c++ new extra data, must define a free_func free it.
 * void delete_extra(ANYTHING *extra) { * free extra; }*/
/* if free_func or extra is null, extra no free */
struct ac_callback* callback_alloc(ac_callback_func func, int fd, int id, void *extra, callback_extra_free_function free_func);
void callback_free(struct ac_callback *callback);

/* callback function and free_extra, if define free extra function */
int callback_call(struct ac_callback *callback, int ret);

/* It's callback_extra_free_function, <free(extra)> */
void extra_defualt_free(void * extra);

#ifdef __cplusplus
}
#endif

#endif  /*__AC_CALLBACK_H__*/

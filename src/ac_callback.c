/**
 * @file ac_callback.c
 * @author Angluca
 * @date 2012-12-26
 */
#include "ac_callback.h"

#include "utils.h"

#include <stdlib.h>
#include <assert.h>

#define   _ac_callback_size  (sizeof(struct ac_callback))

struct ac_callback* callback_alloc(ac_callback_func func, int fd, int id, void *extra, callback_extra_free_function free_func)
{
	struct ac_callback *callback = (struct ac_callback*) malloc(_ac_callback_size);
	if(NULL == callback) {
		EXIT_APP("alloc callback failed.");
	}
	callback->fd = fd;
	callback->id = id;
	callback->extra = extra;
	callback->callback = func;
	callback->extra_free_func = free_func;
	return callback;
}

static inline void _callback_extra_free(struct ac_callback *callback) {
	assert(callback);
	if(callback->extra_free_func && callback->extra) {
		callback->extra_free_func(callback->extra);
		callback->extra = NULL;
		callback->extra_free_func = NULL;
	}
}
void callback_free(struct ac_callback *callback)
{
	assert(callback);
	_callback_extra_free(callback);
	free(callback);
}

int callback_call(struct ac_callback *callback, int result)
{
	assert(callback);
	int ret = callback->callback(callback->fd, callback->id, callback->extra, result);
	_callback_extra_free(callback);
	return ret;
}

void callback_extra_free(void * extra)
{
	assert(extra);
	free(extra);
}


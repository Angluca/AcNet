/**
 * @file ac_package.c
 * @author Angluca
 * @date 2012-12-26
 */
#include "ac_package.h"
#include "utils.h"
#include "ac_buffer.h"
#include "ac_callback.h"

#include <stdlib.h>
#include <arpa/inet.h>

#include <assert.h>
#include <memory.h>

#define   _package_size  sizeof(struct ac_package)
#define   _package_head_size  sizeof(struct ac_package_head)

struct ac_package_head* package_head_alloc_and_copy(struct ac_package_head* head)
{
	struct ac_package_head *_head = (struct ac_package_head*) malloc(_package_head_size);
	assert(_head);
	if(NULL == _head) {
		EXIT_APP("allocate memory failed.");
	}
	memcpy(_head, head, _package_head_size);
	return _head;
}
int package_head_copy(struct ac_package_head* lhs, struct ac_package_head* rhs)
{
	assert(lhs && rhs);
	memcpy(lhs, rhs, _package_head_size);
	return 0;
}
void package_head_free(struct ac_package_head* head)
{
	assert(head);
	free(head);
}

uint16_t package_size(struct ac_package_head* head)
{
	/*return ntohs(head->size);*/
	return (head->size);
}

struct ac_package* package_alloc(struct ac_buffer* head, struct ac_buffer* buffer)
{
	struct ac_package* package = (struct ac_package*)calloc(1, _package_size);
	assert(package);
	if(NULL == package) {
		EXIT_APP("allocate memory failed.");
	}
	package->head = head;
	package->buffer = buffer;

	return package;
}

void package_free(struct ac_package* package)
{

	if(package->head) {
		buffer_free(package->head);
	}
	if(package->buffer) {
		buffer_free(package->buffer);
	}
	if(package->send_callback) {
		callback_free(package->send_callback);
	}

	free(package);
}

void package_reset(struct ac_package* package)
{
	if(package->head) {
		buffer_reset(package->head);
	}
	if(package->buffer) {
		buffer_reset(package->buffer);
	}
	struct ac_callback *call = package->send_callback;
	if(call) {
		callback_free(call);
		package->send_callback = NULL;
	}
}

struct ac_buffer* package_read_complete(struct ac_package* package, int* out_complete)
{
	int ret = 0;
	if(package->head) {
		ret = is_buffer_read_complete(package->head, _package_head_size);
		*out_complete = ret;
		if(ret == 0) {
			return package->head;
		} else if(ret < 0) {
			return NULL;
		}
	}
    assert(package->head);
    assert(package->head->buf);
	struct ac_package_head *head = (struct ac_package_head*)(package->head->buf);
	if(head->size && package->buffer) {
		ret = is_buffer_read_complete(package->buffer, head->size);
		*out_complete = ret;
		if(ret == 0) {
			return package->buffer;
		} else if(ret < 0) {
			return NULL;
		}
	}
	return NULL;
}

struct ac_buffer* package_write_complete(struct ac_package* package, int *out_complete)
{
	int ret = 0;
	if(package->head) {
		ret = is_buffer_write_complete(package->head, _package_head_size);
		*out_complete = ret;
		if(ret == 0) {
			return package->head;
		} else if(ret < 0) {
			return NULL;
		}
	}
    assert(package->head);
    assert(package->head->buf);
	struct ac_package_head *head = (struct ac_package_head*)(package->head->buf);
	if(head->size && package->buffer) {
		struct ac_package_head *head = (struct ac_package_head*)package->head->buf;
		ret = is_buffer_write_complete(package->buffer, package_size(head));
		*out_complete = ret;
		if(ret == 0) {
			return package->buffer;
		} else if(ret < 0) {
			return NULL;
		}
	}
	return NULL;
}

int package_callback(struct ac_package* package, int ret)
{
	if(package->send_callback) {
		return callback_call(package->send_callback, ret);
	}
	return 0;
}


/**
 * @file ac_buffer.c
 * @author Angluca
 * @date 2012-12-26
 */

#include "ac_buffer.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

/*#include <sys/queue.h>*/

/*struct ac_buffer*/
/*{*/
	/*uint16_t read_offset;*/
	/*uint16_t write_offset;*/
	/*uint8_t *buf;*/
	/*uint16_t size;*/

	/*[>SIMPLEQ_ENTRY(ac_buffer) _link;<]*/
/*};*/

#define   _extra_buffer_size  32
#define   _ac_buffer_size  sizeof(struct ac_buffer)

static inline int _read_remain(struct ac_buffer* buffer) {
	assert((buffer->write_offset - buffer->read_offset) >= 0);
	return (buffer->write_offset - buffer->read_offset);
}
static inline int _write_remain(struct ac_buffer* buffer) {
	assert((buffer->size - buffer->write_offset) >= 0);
	return (buffer->size - buffer->write_offset);
}
static inline void _increase_read_index(struct ac_buffer* buffer, int size) {
	int remain = _read_remain(buffer);
	if(size > remain) size = remain;
	buffer->read_offset += size;
}
static inline void _increase_write_index(struct ac_buffer* buffer, int size)
{
	int remain = _write_remain(buffer);
	if(size > remain) size = remain;
	buffer->write_offset += size;
}

static inline uint8_t _buffer_read(struct ac_buffer* buffer, void *value, int size)
{
	assert(buffer);
	assert(value);
	int remain = _read_remain(buffer);
	if(remain < size) {
		return 0;
	}
	memcpy(value, ( buffer->buf + buffer->read_offset ), size);
	_increase_read_index(buffer, size);
	return 1;
}
static inline uint8_t _buffer_write(struct ac_buffer* buffer, void *value, int size)
{
	assert(buffer);
	assert(value);
	int remain = _write_remain(buffer);
	if(remain < size) {
		buffer_resize(buffer, remain + size);
	}
	memcpy(( buffer->buf+buffer->write_offset ), value, size);
	_increase_write_index(buffer, size);
	return 1;
}

struct ac_buffer* buffer_alloc(int size)
{
	struct ac_buffer * buffer = (struct ac_buffer*)calloc(1, _ac_buffer_size);
	assert(buffer);
	if(NULL == buffer) {
		EXIT_APP("memory allocate failed.");
	}
    int total_size = size;
	buffer->buf = (uint8_t*)calloc(1, total_size);
	if(NULL == buffer) {
		EXIT_APP("memory allocate failed.");
	}
	buffer->size = total_size;
	return buffer;
}

struct ac_buffer* buffer_alloc_and_copy(struct ac_buffer* buffer)
{
	assert(buffer);
	int len = buffer_size(buffer);
	struct ac_buffer *_buffer = buffer_alloc(len);
	buffer_copy(_buffer, buffer->buf, len);
	return _buffer;
}

struct ac_buffer* buffer_extra_alloc(int size)
{
    return buffer_alloc(size + _extra_buffer_size);
    //return buffer_alloc(size);
}

void buffer_free(struct ac_buffer* buffer)
{
	assert(buffer);
	free(buffer->buf);
	free(buffer);
}

int buffer_size(struct ac_buffer* buffer)
{
	return buffer->write_offset;
}

int buffer_reset(struct ac_buffer* buffer)
{
	assert(buffer);
	buffer->read_offset = buffer->write_offset = 0;
	memset(buffer->buf, 0, buffer->size);
	return 0;
}

int buffer_resize(struct ac_buffer* buffer, int size)
{
	assert(buffer);
	assert(size > 0);
	if(size < 1) return -1;
	if(buffer->size < size) {
		/*uint8_t *p = (uint8_t*)calloc(1, size);*/
		/*uint8_t *tmp = buffer->buf;*/
		/*if(NULL == p) {*/
			/*EXIT_APP("allocate memory failed.");*/
		/*}*/
		/*if(buffer->size > 0) {*/
			/*memcpy(p, buffer->buf, buffer->size);*/
			/*free(tmp);*/
		/*}*/
		uint8_t *p = (uint8_t*)realloc(buffer->buf, size);
		if(NULL == p) {
			EXIT_APP("allocate memory failed.");
		}
		buffer->buf = p;
		buffer->size = size;
	}
	return 0;
}

void buffer_increase_read_index(struct ac_buffer* buffer, int size)
{
	_increase_read_index(buffer, size);
}

void buffer_increase_write_index(struct ac_buffer* buffer, int size)
{
	_increase_write_index(buffer, size);
}

int buffer_copy(struct ac_buffer* buffer, uint8_t *value, int size)
{
	assert(buffer);
	assert(value);
	if(buffer->size < size) {
		buffer_resize(buffer, size);
	}
	memcpy(buffer->buf, value, size);
	buffer->write_offset = size;
	return 0;
}

int buffer_read_remain_len(struct ac_buffer *buffer)
{
	return _read_remain(buffer);
}

int buffer_read(struct ac_buffer* buffer, uint8_t *value, int size)
{
	return _buffer_read(buffer, value, size);
}

int buffer_write(struct ac_buffer* buffer, uint8_t *value, int size)
{
	return _buffer_write(buffer, value, size);
}

int is_buffer_read_complete(struct ac_buffer* buffer, int end_idx)
{
	assert(buffer);
	int remain;
    if(end_idx < 1) {
        remain = _read_remain(buffer);
    }
    else {
        remain = end_idx - buffer->read_offset;
    }
	if(remain > 0) return 0;
	if(remain == 0) return 1;
	return -1;
}

int is_buffer_write_complete(struct ac_buffer* buffer, int end_idx)
{
	assert(buffer);
	int remain;
	if(end_idx < 1) {
		remain = _write_remain(buffer);
    }
	else {
		remain = end_idx - buffer->write_offset;
	}
	if(remain > 0) return 0;
	if(remain == 0) return 1;
	return -1;
}

uint8_t* buffer_content(struct ac_buffer* buffer, int *out_writed)
{
	assert(buffer);
	assert(out_writed);
	*out_writed = buffer->write_offset;
	/*if(*out_writed < 1) return NULL;*/
	uint8_t* p = buffer->buf;
	return p;
}
uint8_t* buffer_read_content(struct ac_buffer* buffer, int *out_remain)
{
	assert(buffer);
	assert(out_remain);
	*out_remain = _read_remain(buffer);
	/*if(*out_remain < 1) return NULL;*/
	uint8_t* p = buffer->buf + buffer->read_offset;
	return p;
}

uint8_t* buffer_write_content(struct ac_buffer* buffer, int *out_remain, int end_idx)
{
	assert(buffer);
	assert(out_remain);
	if(end_idx < 1) {
		/**out_remain = -1;*/
		*out_remain = _write_remain(buffer);
	}
	else {
		*out_remain = end_idx - buffer->write_offset;
	}
	/*if(*out_remain < 1) return NULL;*/
	uint8_t* p = buffer->buf + buffer->write_offset;
	return p;
}

/* protocol functions */
#define   _byte_size  sizeof(char)
#define   _short_size  sizeof(short)
#define   _int_size  sizeof(int)
#define   _int64_size  sizeof(int64_t)
#define   _float_size  sizeof(float)
#define   _double_size  sizeof(double)

static inline uint8_t _buffer_read_array(struct ac_buffer* buffer, void* value, int *size, int type_size)
{
	uint16_t n;
	if(( _buffer_read(buffer, &n, _short_size) == 0 ) ||
			(*size < n)) return 0;
	*size = (int)n;
	return _buffer_read(buffer, value, n * type_size);
}
static inline uint8_t _buffer_write_array(struct ac_buffer* buffer, void* value, int size, int type_size)
{
	/* uint16_t s = (uint16_t)size;*/
	if(( size < 1 ) ||
		( _buffer_write(buffer, &size, _short_size) == 0 )) return 0;
	return _buffer_write(buffer, value, size * type_size);
}

/* read type */
int buffer_read_char(struct ac_buffer* buffer, char *value)
{
	return _buffer_read(buffer, value, _byte_size);
}
int buffer_read_byte(struct ac_buffer* buffer, uint8_t *value)
{
	return _buffer_read(buffer, value, _byte_size);
}
int buffer_read_char_array(struct ac_buffer* buffer, char *value, int *size)
{
	return _buffer_read_array(buffer, value, size, _byte_size);
}
int buffer_read_byte_array(struct ac_buffer* buffer, uint8_t *value, int *size)
{
	return _buffer_read_array(buffer, value, size, _byte_size);
}

int buffer_read_short(struct ac_buffer* buffer, int16_t *value)
{
	return _buffer_read(buffer, value, _short_size);
}
int buffer_read_ushort(struct ac_buffer* buffer, uint16_t *value)
{
	return _buffer_read(buffer, value, _short_size);
}
int buffer_read_short_array(struct ac_buffer* buffer, int16_t *value, int *size)
{
	return _buffer_read_array(buffer, value, size, _short_size);
}
int buffer_read_ushort_array(struct ac_buffer* buffer, uint16_t *value, int *size)
{
	return _buffer_read_array(buffer, value, size, _short_size);
}

int buffer_read_int(struct ac_buffer* buffer, int *value)
{
	return _buffer_read(buffer, value, _int_size);
}
int buffer_read_uint(struct ac_buffer* buffer, uint32_t *value)
{
	return _buffer_read(buffer, value, _int_size);
}
int buffer_read_int_array(struct ac_buffer* buffer, int *value, int *size)
{
	return _buffer_read_array(buffer, value, size, _int_size);
}
int buffer_read_uint_array(struct ac_buffer* buffer, uint32_t *value, int *size)
{
	return _buffer_read_array(buffer, value, size, _int_size);
}

int buffer_read_int64(struct ac_buffer* buffer, int64_t *value)
{
	return _buffer_read(buffer, value, _int64_size);
}
int buffer_read_uint64(struct ac_buffer* buffer, uint64_t *value)
{
	return _buffer_read(buffer, value, _int64_size);
}
int buffer_read_int64_array(struct ac_buffer* buffer, int64_t *value, int *size)
{
	return _buffer_read_array(buffer, value, size, _int64_size);
}
int buffer_read_uint64_array(struct ac_buffer* buffer, uint64_t *value, int *size)
{
	return _buffer_read_array(buffer, value, size, _int64_size);
}

int buffer_read_float(struct ac_buffer* buffer, float *value)
{
	return _buffer_read(buffer, value, _float_size);
}
int buffer_read_float_array(struct ac_buffer* buffer, float *value, int *size)
{
	return _buffer_read_array(buffer, value, size, _float_size);
}

int buffer_read_double(struct ac_buffer* buffer, double *value)
{
	return _buffer_read(buffer, value, _double_size);
}
int buffer_read_double_array(struct ac_buffer* buffer, double *value, int *size)
{
	return _buffer_read_array(buffer, value, size, _double_size);
}

int buffer_read_string(struct ac_buffer* buffer, char *value, int *size)
{
	return _buffer_read_array(buffer, value, size, _byte_size);
}

/* write type*/
int buffer_write_char(struct ac_buffer* buffer, char value)
{
	return _buffer_write(buffer, &value, _byte_size);
}
int buffer_write_byte(struct ac_buffer* buffer, uint8_t value)
{
	return _buffer_write(buffer, &value, _byte_size);
}
int buffer_write_char_array(struct ac_buffer* buffer, char *value, int size)
{
	return _buffer_write_array(buffer, value, size, _byte_size);
}
int buffer_write_byte_array(struct ac_buffer* buffer, uint8_t *value, int size)
{
	return _buffer_write_array(buffer, value, size, _byte_size);
}

int buffer_write_short(struct ac_buffer* buffer, int16_t value)
{
	return _buffer_write(buffer, &value, _short_size);
}
int buffer_write_ushort(struct ac_buffer* buffer, uint16_t value)
{
	return _buffer_write(buffer, &value, _short_size);
}
int buffer_write_short_array(struct ac_buffer* buffer, int16_t *value, int size)
{
	return _buffer_write_array(buffer, value, size, _short_size);
}
int buffer_write_ushort_array(struct ac_buffer* buffer, uint16_t *value, int size)
{
	return _buffer_write_array(buffer, value, size, _short_size);
}

int buffer_write_int(struct ac_buffer* buffer, int32_t value)
{
	return _buffer_write(buffer, &value, _int_size);
}
int buffer_write_uint(struct ac_buffer* buffer, uint32_t value)
{
	return _buffer_write(buffer, &value, _int_size);
}
int buffer_write_int_array(struct ac_buffer* buffer, int32_t *value, int size)
{
	return _buffer_write_array(buffer, value, size, _int_size);
}
int buffer_write_uint_array(struct ac_buffer* buffer, uint32_t *value, int size)
{
	return _buffer_write_array(buffer, value, size, _int_size);
}

int buffer_write_int64(struct ac_buffer* buffer, int64_t value)
{
	return _buffer_write(buffer, &value, _int64_size);
}
int buffer_write_uint64(struct ac_buffer* buffer, uint64_t value)
{
	return _buffer_write(buffer, &value, _int64_size);
}
int buffer_write_int64_array(struct ac_buffer* buffer, int64_t *value, int size)
{
	return _buffer_write_array(buffer, value, size, _int64_size);
}
int buffer_write_uint64_array(struct ac_buffer* buffer, uint64_t *value, int size)
{
	return _buffer_write_array(buffer, value, size, _int64_size);
}

int buffer_write_float(struct ac_buffer* buffer, float value)
{
	return _buffer_write(buffer, &value, _float_size);
}
int buffer_write_float_array(struct ac_buffer* buffer, float *value, int size)
{
	return _buffer_write_array(buffer, value, size, _float_size);
}

int buffer_write_double(struct ac_buffer* buffer, double value)
{
	return _buffer_write(buffer, &value, _double_size);
}
int buffer_write_double_array(struct ac_buffer* buffer, double *value, int size)
{
	return _buffer_write_array(buffer, value, size, _double_size);
}

int buffer_write_string(struct ac_buffer* buffer, char *value, int size)
{
	return _buffer_write_array(buffer, value, size, _byte_size);
}

/* read type */


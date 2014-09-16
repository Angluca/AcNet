/**
 * @file ac_buffer.h
 * @author Angluca
 * @date 2012-12-26
 */

#ifndef  __AC_BUFFER_H__
#define  __AC_BUFFER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ac_buffer
{
	uint16_t read_offset;
	uint16_t write_offset;
	uint8_t *buf;
	uint16_t size;
};


struct ac_buffer* buffer_alloc(int size);
struct ac_buffer* buffer_alloc_and_copy(struct ac_buffer* buffer);
struct ac_buffer* buffer_extra_alloc(int size);

void buffer_free(struct ac_buffer* buffer);

int buffer_size(struct ac_buffer* buffer);
int buffer_reset(struct ac_buffer* buffer);
int buffer_resize(struct ac_buffer* buffer, int size);

int buffer_copy(struct ac_buffer* buffer, uint8_t *value, int size);

int buffer_read_remain_len(struct ac_buffer *buffer);
int buffer_read(struct ac_buffer* buffer, uint8_t *value, int size);
int buffer_write(struct ac_buffer* buffer, uint8_t *value, int size);

void buffer_increase_read_index(struct ac_buffer* buffer, int size);
void buffer_increase_write_index(struct ac_buffer* buffer, int size);
/* -------------------------------------------*/
/**
 * @Synopsis  is_buffer_read_complete
 * @Param buffer
 * @Returns -1:error 0:n 1:y
 */
/* -------------------------------------------*/
int is_buffer_read_complete(struct ac_buffer* buffer, int end_idx);
/* -------------------------------------------*/
/**
 * @Synopsis  is_buffer_write_complete
 * @Param buffer
 * @Returns -1:error 0:n 1:y
 */
/* -------------------------------------------*/
int is_buffer_write_complete(struct ac_buffer* buffer, int end_idx);

uint8_t* buffer_content(struct ac_buffer* buffer, int *out_writed);
uint8_t* buffer_read_content(struct ac_buffer* buffer, int *out_remain);
uint8_t* buffer_write_content(struct ac_buffer* buffer, int *out_remain, int end_idx);

/* protocol functions */
/* read type */
int buffer_read_char(struct ac_buffer* buffer, char *value);
int buffer_read_byte(struct ac_buffer* buffer, uint8_t *value);
int buffer_read_char_array(struct ac_buffer* buffer, char *value, int *size);
int buffer_read_byte_array(struct ac_buffer* buffer, uint8_t *value, int *size);

int buffer_read_short(struct ac_buffer* buffer, int16_t *value);
int buffer_read_ushort(struct ac_buffer* buffer, uint16_t *value);
int buffer_read_short_array(struct ac_buffer* buffer, int16_t *value, int *size);
int buffer_read_ushort_array(struct ac_buffer* buffer, uint16_t *value, int *size);

int buffer_read_int(struct ac_buffer* buffer, int32_t *value);
int buffer_read_uint(struct ac_buffer* buffer, uint32_t *value);
int buffer_read_int_array(struct ac_buffer* buffer, int32_t *value, int *size);
int buffer_read_uint_array(struct ac_buffer* buffer, uint32_t *value, int *size);

int buffer_read_int64(struct ac_buffer* buffer, int64_t *value);
int buffer_read_uint64(struct ac_buffer* buffer, uint64_t *value);
int buffer_read_int64_array(struct ac_buffer* buffer, int64_t *value, int *size);
int buffer_read_uint64_array(struct ac_buffer* buffer, uint64_t *value, int *size);

int buffer_read_float(struct ac_buffer* buffer, float *value);
int buffer_read_double(struct ac_buffer* buffer, double *value);
int buffer_read_float_array(struct ac_buffer* buffer, float *value, int *size);
int buffer_read_double_array(struct ac_buffer* buffer, double *value, int *size);

int buffer_read_string(struct ac_buffer* buffer, char *value, int *size);

/* write type*/
int buffer_write_char(struct ac_buffer* buffer, char value);
int buffer_write_byte(struct ac_buffer* buffer, uint8_t value);
int buffer_write_char_array(struct ac_buffer* buffer, char *value, int size);
int buffer_write_byte_array(struct ac_buffer* buffer, uint8_t *value, int size);

int buffer_write_short(struct ac_buffer* buffer, int16_t value);
int buffer_write_ushort(struct ac_buffer* buffer, uint16_t value);
int buffer_write_short_array(struct ac_buffer* buffer, int16_t *value, int size);
int buffer_write_ushort_array(struct ac_buffer* buffer, uint16_t *value, int size);

int buffer_write_int(struct ac_buffer* buffer, int value);
int buffer_write_uint(struct ac_buffer* buffer, uint32_t value);
int buffer_write_int_array(struct ac_buffer* buffer, int *value, int size);
int buffer_write_uint_array(struct ac_buffer* buffer, uint32_t *value, int size);

int buffer_write_int64(struct ac_buffer* buffer, int64_t value);
int buffer_write_uint64(struct ac_buffer* buffer, uint64_t value);
int buffer_write_int64_array(struct ac_buffer* buffer, int64_t *value, int size);
int buffer_write_uint64_array(struct ac_buffer* buffer, uint64_t *value, int size);

int buffer_write_float(struct ac_buffer* buffer, float value);
int buffer_write_float_array(struct ac_buffer* buffer, float *value, int size);

int buffer_write_double(struct ac_buffer* buffer, double value);
int buffer_write_double_array(struct ac_buffer* buffer, double *value, int size);

int buffer_write_string(struct ac_buffer* buffer, char *value, int size);

#ifdef __cplusplus
}
#endif

#endif  /*__AC_BUFFER_H__*/

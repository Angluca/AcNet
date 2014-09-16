#include "ac_buffer.h"
#include <string.h>
#include <stdio.h>

void test_buffer()
{
	char *p = "hellow buffer";
	struct ac_buffer* buffer = buffer_alloc(2+strlen(p));
	uint16_t i = strlen(p);
	buffer_write(buffer, (uint8_t*)&i, 2);
	buffer_write(buffer, (uint8_t*)p, strlen(p));
	int b = buffer_resize(buffer, 4096);
	printf("b:%d\n", b);
	uint16_t k = 0;
	buffer_read(buffer, (uint8_t*)&k, 2);
	char buf[1024] = {0};
	buffer_read(buffer, (uint8_t*)buf, strlen(p));
	if(is_buffer_write_complete(buffer, 15) == 1) {
		printf("buffer write complete size:%d\n", buffer->size);
	}
	if(is_buffer_read_complete(buffer) == 1) {
		printf("i:%d p:%s, k:%d, buf:%s\n", i, p, k, buf);
	}
	buffer_reset(buffer);

	char cc = 90;
	uint8_t ret;
	ret = buffer_write_char(buffer, &cc);
	cc = 0;
	ret = buffer_read_char(buffer, &cc);
	printf("char:%d\n", cc);
	char cz[10] = {0,1,2,3,4,5,6,7,8,9};
	int n = 10;
	ret = buffer_write_char_array(buffer, cz, n);
	ret = buffer_write_string(buffer, cz, n);
	ret = buffer_read_char_array(buffer, cz, &n);
	printf("ret %d\n", ret);
	int a;
	printf("char[%d]={", n);
	for(a=0; a<n; ++a) {
		printf(" %d", cz[a]);
	}
	printf("} s:%d\n", buffer->write_offset);
	ret = buffer_read_string(buffer, cz, &n);
	printf("string[%d]={", n);
	for(a=0; a<n; ++a) {
		printf(" %d", cz[a]);
	}
	printf("} s:%d\n", buffer->write_offset);
	ret = buffer_read_string(buffer, cz, &n);
	printf("ret %d\n", ret);
}

#ifndef  __AC_PACKAGE_H__
#define  __AC_PACKAGE_H__
/**
 * @file ac_package.h
 * @author Angluca
 * @date 2012-12-26
 */


#include <stdint.h>
#include <sys/queue.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ac_buffer;
struct ac_callback;

#define  g_package_head_buffer_size 6
struct ac_package_head
{
	union {
		uint8_t buf[g_package_head_buffer_size];
		struct {
			uint16_t size;
			uint16_t id;
			uint16_t result;
		};
	};
};

struct ac_package_head* package_head_alloc_and_copy(struct ac_package_head* head);
int package_head_copy(struct ac_package_head* lhs, struct ac_package_head* rhs);
void package_head_free(struct ac_package_head* head);

uint16_t package_size(struct ac_package_head* head);

struct ac_package
{
	/*union {
		struct ac_package_head header;
		uint8_t header_buf[6];
	};*/

	struct ac_buffer *head;
	struct ac_buffer *buffer;
	struct ac_callback* send_callback;
	STAILQ_ENTRY(ac_package) _link;
};

struct ac_package* package_alloc(struct ac_buffer* head, struct ac_buffer* buffer);
void package_free(struct ac_package* package);
void package_reset(struct ac_package* package);

/* -------------------------------------------*/
/**
 * @Synopsis  is_package_read_complete
 * @Param package
 * @Returns -1:error 0:no 1:yes
 */
/* -------------------------------------------*/
struct ac_buffer* package_read_complete(struct ac_package* package, int* out_complete);
/* -------------------------------------------*/
/**
 * @Synopsis  is_package_write_complete
 * @Param package
 * @Returns -1:error 0:no 1:yes
 */
/* -------------------------------------------*/
struct ac_buffer* package_write_complete(struct ac_package* package, int* out_complete);

int package_callback(struct ac_package* package, int ret);

static inline void package_set_send_callback(struct ac_package* package, struct ac_callback* callback)
{
	package->send_callback = callback;
}


#ifdef __cplusplus
}
#endif


#endif  /*__AC_PACKAGE_H__*/


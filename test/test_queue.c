#include <stdio.h>
#include <sys/queue.h>
#include <memory.h>
struct my_test
{
	int value;
	SIMPLEQ_ENTRY(my_test) entity;
};
SIMPLEQ_HEAD(my_queue, my_test);

void test_queue()
{
	struct my_queue my_head;
	struct my_test test[5];
	SIMPLEQ_INIT(&my_head);
	int i;
	for(i=0; i<5; ++i) {
		memset(&test[i], 0, sizeof(struct my_test));
		test[i].value = i+1;
		SIMPLEQ_INSERT_TAIL(&my_head, &test[i], entity);
	}
	printf("first:%d\n", SIMPLEQ_FIRST(&my_head)->value);
	SIMPLEQ_REMOVE_HEAD(&my_head, entity);
	printf("first:%d\n", SIMPLEQ_FIRST(&my_head)->value);
	/*struct task_t t[5];                                                    */
	/*int i;*/
	/*SIMPLEQ_INIT(&task_head);*/
	/*for (i = 0; i < 5; i++)*/
	/*{                     */
		/*memset(&t[i], 0, sizeof(t[i]));*/
		/*t[i].data = i; */
		/*SIMPLEQ_INSERT_TAIL(&task_head, &t[i], entries);*/
	/*}*/
	/*// delete one element in the queue*/
	/*SIMPLEQ_REMOVE(&task_head, &t[3], task_t, entries);*/
	/*struct task_t *item;*/
	/*SIMPLEQ_FOREACH(item, &task_head, entries) { */
		/*printf("data: %i\n", item->data);*/
	/*}*/
}

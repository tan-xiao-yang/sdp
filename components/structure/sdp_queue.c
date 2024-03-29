#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sdp_queue.h"

#define PTR_CHECK(p) {  \
    if (!p) {           \
        return -1;      \
    }                   \
}

sdp_queue_t *sdp_queue_create(int node_num, int node_size)
{
    sdp_queue_t *queue = NULL;

    queue = (sdp_queue_t *)malloc(sizeof(sdp_queue_t));

    if (!queue) {
        return NULL;
    }

    queue->addr = malloc(node_num * node_size);

    if (!queue->addr) {
        return NULL;
    }

    memset(queue->addr, 0, node_num * node_size);

    queue->prod = queue->addr;
    queue->cons = queue->addr;

    queue->total = node_num;
    queue->avail = node_num;
    queue->offset = node_size;

    return queue;
}

int sdp_enqueue(sdp_queue_t *queue, void *node, int size)
{
    PTR_CHECK(queue);
    PTR_CHECK(node);
    PTR_CHECK(queue->addr);

    if (size > queue->offset) {
        return -2;
    }

    if (!queue->avail) {
        return -3;
    }

    if (queue->prod > queue->addr + (queue->offset * queue->total) ||
        queue->prod + queue->offset > queue->addr + (queue->offset * queue->total)) {
        
        queue->prod = queue->addr;
    }

    memcpy(queue->prod, node, size);

    queue->prod = queue->prod + queue->offset;
    
    queue->avail--;

    return 0;
}

int sdp_dequeue(sdp_queue_t *queue, void *node, int size)
{
    PTR_CHECK(queue);
    PTR_CHECK(node);
    PTR_CHECK(queue->addr);

    if (queue->avail == queue->total) {
        return -4;
    }

    if (queue->offset < size) {
        return -5;
    }

    if (queue->cons > queue->addr + (queue->offset * queue->total) ||
        queue->cons + queue->offset > queue->addr + (queue->offset * queue->total)) {
        
        queue->cons = queue->addr;
    }

    memcpy(node, queue->cons, size);

    queue->cons = queue->cons + queue->offset;
    
    queue->avail++;

    return 0;
}

int sdp_queue_head(sdp_queue_t *queue, void *node, int size)
{
    PTR_CHECK(queue);
    PTR_CHECK(node);
    PTR_CHECK(queue->addr);

    if (queue->avail == queue->total) {
        return -4;
    }

    if (queue->offset < size) {
        return -5;
    }

    memcpy(node, queue->cons, size);

    return 0;
}

void sdp_queue_free(sdp_queue_t *queue)
{
    if (queue) {
        if (queue->addr) {
            free(queue->addr);
            queue->addr = NULL;
        }

        queue->prod = NULL;
        queue->cons = NULL;
        
        queue->total = 0;
        queue->avail = 0;
        queue->offset = 0;

        free(queue);
    }
}

#if 0
int main()
{
    sdp_queue_t *queue = sdp_queue_create(100, sizeof(int));

    if (!queue) {
        printf("faild to create queue\n");
        return -1;
    }

    int i;
    int node;
    int idex = 0;

    int start = 0;
    
    int loop = start + 50;
    for (i = start; i < loop; ++i, start++) {
        if (sdp_enqueue(queue, &i, sizeof(i))) {
            printf("failed to enqueue\n");
            goto FAIL;
        }
    }
    
    while (!sdp_dequeue(queue, &node, sizeof(node))) {
        printf("dequeue[%d] : %d\n", ++idex, node);
    }

    loop = start + 100;
    for (i = start; i < loop; ++i, start++) {
        if (sdp_enqueue(queue, &i, sizeof(i))) {
            printf("failed to enqueue\n");
            goto FAIL;
        }
    }
    
    while (!sdp_dequeue(queue, &node, sizeof(node))) {
        printf("dequeue[%d] : %d\n", ++idex, node);
    }

    loop = start + 100;
    for (i = start; i < loop; ++i, start++) {
        if (sdp_enqueue(queue, &i, sizeof(i))) {
            printf("failed to enqueue\n");
            goto FAIL;
        }
    }
    
    while (!sdp_dequeue(queue, &node, sizeof(node))) {
        printf("dequeue[%d] : %d\n", ++idex, node);
    }

    sdp_queue_free(queue);

    return 0;

FAIL :

    sdp_queue_free(queue);

    return -1;
}
#endif

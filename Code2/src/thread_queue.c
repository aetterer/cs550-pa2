#include <stdlib.h>
#include "thread_queue.h"

thread_node_t *thread_head = NULL;
thread_node_t *thread_tail = NULL;

void thread_enqueue(int *client_socket) {
  thread_node_t *new_node = malloc(sizeof(thread_node_t));
  new_node->client_socket = client_socket;
  new_node->next = NULL;

  if (thread_tail == NULL) {
    thread_head = new_node;
    thread_tail = new_node;
  } else {
    thread_tail->next = new_node;
  }
}

int * thread_dequeue() {
  if (thread_head == NULL) {
    return NULL;
  } else {
    int *result = thread_head->client_socket;
    thread_node_t *temp = thread_head;
    thread_head = thread_head->next;
    if (thread_head == NULL) {
      thread_tail = NULL;
    }
    free(temp);
    return result;
  }
}

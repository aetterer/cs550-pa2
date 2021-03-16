#include <stdlib.h>
#include "thread_queue.h"

node_t *head = NULL;
node_t *tail = NULL;

void enqueue(int *client_socket) {
  node_t *new_node = malloc(sizeof(node_t));
  new_node->client_socket = client_socket;
  new_node->next = NULL;

  if (tail == NULL) {
    head = new_node;
  } else {
    tail->next = new_node;
  }
}

int * dequeue() {
  if (head == NULL) {
    return NULL;
  } else {
    int *result = head->client_socket;
    node_t *temp = head;
    head = head->next;
    if (head == NULL) {
      tail = NULL;
    }
    free(temp);
    return result;
  }
}

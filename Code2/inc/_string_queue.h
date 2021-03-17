#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct Node {
  char *key;
  struct Node *next;
} node_t;

typedef struct Queue {
  node_t *head;
  node_t *tail;
} queue_t;

node_t *new_node(char *k);

queue_t *new_queue();

void enqueue(queue_t *q, char *k);

int dequeue(queue_t *q, char *buf);

queue_t * queue_from_string(char *);

int size(queue_t *q);

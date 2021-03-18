#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "string_queue.h"

//typedef struct Node {
//  char *key;
//  struct Node *next;
//} node_t;

//typedef struct Queue {
//  node_t *head;
//  node_t *tail;
//} queue_t;

node_t *new_node(char *k) {
  node_t *tmp = (node_t *)malloc(sizeof(node_t));
  tmp->key = k;
  tmp->next = NULL;
  return tmp;
}

queue_t *new_queue() {
  queue_t *q = (queue_t *)malloc(sizeof(queue_t));
  q->head = NULL;
  q->tail = NULL;
  return q;
}

void enqueue(queue_t *q, char *k) {
  node_t *nn = new_node(k);
  if (q->tail == NULL) {
    q->head = nn;
    q->tail = nn;
    return;
  }

  q->tail->next = nn;
  q->tail = nn;
}

int dequeue(queue_t *q, char *buf) {
  if (q->head == NULL)
    return -1;

  node_t *d = q->head;
  q->head = q->head->next;

  if (q->head == NULL)
    q->tail = NULL;

  strcpy(buf, d->key);
  free(d);
  return 0;
}

queue_t * queue_from_string(char *str) {
  queue_t *q;
  char *token;

  q = new_queue();
  token = strtok(str, " ");
  while (token != NULL) {
    enqueue(q, token);
    token = strtok(NULL, " ");
  }
  return q;
}

int size(queue_t *q) {
  node_t *current = q->head;
  if (current == NULL) return 0;
  
  int i = 1;
  while(current->next != NULL) {
    i++;
    current = current->next;
  }
  return i;
}


#include <stdlib.h>
#include <string.h>
#include "cmd_queue.h"

cmd_node_t *cmd_head = NULL;
cmd_node_t *cmd_tail = NULL;

void cmd_enqueue(char *cmd) {
  cmd_node_t *new_node = malloc(sizeof(cmd_node_t));
  new_node->cmd = cmd;
  new_node->next = NULL;

  if (cmd_tail == NULL) {
    cmd_head = new_node;
    cmd_tail = new_node;
  } else {
    cmd_tail->next = new_node;
  }
}

void cmd_dequeue(char *buf) {
  if (cmd_head == NULL) {
    return;
  } else {
    char *result = cmd_head->cmd;
    cmd_node_t *temp = cmd_head;
    cmd_head = cmd_head->next;
    if (cmd_head == NULL) {
      cmd_tail = NULL;
    }
    free(temp);
    if (buf != NULL) {
        strcpy(buf, result);
    }
  }
}

void cmd_queue_from_string(char *str) {
    char *token = strtok(str, " ");
    while (token != NULL) {
        cmd_enqueue(token);
        token = strtok(NULL, " ");
    }
}

void flush_cmd_queue() {
    while (cmd_head != NULL && cmd_tail != NULL) {
        cmd_dequeue(NULL);
    }
}

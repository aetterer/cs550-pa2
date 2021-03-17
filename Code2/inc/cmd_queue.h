typedef struct cmd_node {
  char *cmd;
  struct cmd_node *next;
} cmd_node_t;

void cmd_enqueue(char *);
void cmd_dequeue(char *);
void cmd_queue_from_string(char *);
void flush_cmd_queue();

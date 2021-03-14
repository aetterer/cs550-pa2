typedef struct node {
  int *client_socket;
  struct node *next;
} node_t;

void enqueue(int *);

int * dequeue();

typedef struct thread_node {
  int *client_socket;
  struct thread_node *next;
} thread_node_t;

void thread_enqueue(int *);

int * thread_dequeue();

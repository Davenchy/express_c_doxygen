/**
 * @file express.c
 * @brief Simple Express chain implementation.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @typedef Node
 * @brief Represents a linked list node.
 * @see Node
 *
 * @struct Node
 * @brief Represents doubly linked list node.
 * @see node_create
 *
 * Use create_node function to allocate one.
 *
 * To free just call the **stdlib** `free()` function.
 */
typedef struct Node {
  void *value;       /**< Pointer to the linked list node value */
  struct Node *next; /**< Pointer to the linked list next node */
  struct Node *prev; /**< Pointer to the linked list previous node */
} Node;

/**
 * @typedef List
 * @brief Represents a doubly linked list.
 * @see List
 *
 * @struct List
 * @brief Represents a doubly linked list data structure.
 * @see list_push
 * @see list_shift
 * @see list_clear
 *
 * You don't have to allocate it.
 *
 * But don't forget to call `list_clear(*List)` to free any remaining nodes,
 * as nodes created in *heap*.
 */
typedef struct List {
  Node *head; /**< aaa */
  Node *tail; /**< adds */
} List;

/**
 * @typedef ExpressCommand
 * @brief Represents command for the Express chain execute function.
 *
 * @enum ExpressCommand
 * @brief Represents command for the Express chain execute function.
 * @see ExpressCallback
 * @see express_execute
 */
typedef enum ExpressCommand {
  E_CONTINUE, /**< Continue chain execution */
  E_TRIGGER,  /**< Trigger stop action */
} ExpressCommand;

/**
 * @typedef Express
 * @brief Object that stores chain of callbacks and executes them one after the
 * other.
 * @see Express
 *
 * @struct Express
 * @brief Object that stores chain of callbacks and executes them one after the
 * other.
 *
 * @see List
 * @see express_create
 * @see express_add
 * @see express_destroy
 *
 * You don't have to allocate this object in heap.
 * Just create it by using `Express express_create()` function.
 *
 * Don't forget to call `express_destory(*Express)`, this function just make
 * sure that no linked list nodes remain in the heap.
 *
 * This object is **thread safe**.
 */
typedef struct Express {
  List chain; /**< Linked list object that stores all the callback functions.*/
  pthread_mutex_t lock; /**< Muxtex Lock for thread safety.*/
} Express;

/**
 * @typedef ExpressCallback
 * @brief A callback that passed to *express_add*.
 * @param ExpressCommand Pointer to ExpressCallback function.
 * @see express_add
 * @see ExpressCommand
 *
 * @return ExpressCommand that tells the Express chain executer what to do next.
 */
typedef ExpressCommand (*ExpressCallback)(void);

/* =============== Function Prototypes ================== */

/**
 * @brief Creates an empty Express object.
 *
 * @return An empty Express object, that is allocated on the stack.
 */
Express express_create();

/**
 * @brief Adds ExpressCallback to the chain of execution.
 *
 * @param app Pointer to Express object.
 * @param cb Pointer to ExpressCallback function to add to the chain.
 *
 * This function is *Thread Safe*.
 */
void express_add(Express *app, ExpressCallback cb);

/**
 * @brief Executes the Express chain.
 *
 * @param app Pointer to Express object.
 *
 * This function is *Thread Safe*.
 */
void express_execute(Express *app);

/**
 * @brief Cleans any heap nodes created for the chain.
 *
 * @param app Pointer to Express object.
 */
void express_destroy(Express *app);

/**
 * @brief ExpressCallback function that prints hello.
 * @see express_add
 *
 * @return E_CONTINUE as an ExpressCommand to continue chain exection.
 */
ExpressCommand hello_callback(void);

/**
 * @brief ExpressCallback that prints out.
 * @see express_add
 *
 * @return E_CONTINUE as an ExpressCommand to continue chain exection.
 */
ExpressCommand out_callback(void);

/**
 * @brief ExpressCallback that prints trigger.
 * @see express_add
 *
 * @return E_TRIGGER as an ExpressCommand to stop chain exection.
 */
ExpressCommand trigger_callback(void);

/* =============== Main ================== */

int main(void) {
  Express app = express_create();

  express_add(&app, hello_callback);
  express_add(&app, trigger_callback);
  express_add(&app, out_callback);

  express_execute(&app);
  express_destroy(&app);

  return 0;
}

ExpressCommand hello_callback() {
  printf("Hello\n");
  return E_CONTINUE;
}

ExpressCommand out_callback() {
  printf("Out\n");
  return E_CONTINUE;
}

ExpressCommand trigger_callback() {
  printf("Trigger\n");
  return E_TRIGGER;
}

/* =============== Node Type ================== */

/**
 * @brief Allocates a new Node object in heap.
 *
 *
 * @param value Pointer to the value holded by the node.
 * @param next Pointer to the next node.
 * @param prev Pointer to the previous node.
 * @return Pointer to the heap allocated Node object.
 * @see Node
 *
 * Don't forget to free the Node by calling `free(*Node)`
 */
Node *node_create(void *value, Node *next, Node *prev) {
  Node *node = malloc(sizeof(Node));
  if (!node) {
    fprintf(stderr, "Failed to allocate memory\n");
    exit(EXIT_FAILURE);
  }

  node->value = value;
  node->next = next;
  node->prev = prev;
  return node;
}

/* =============== List Type ================== */

/**
 * @brief Pushes/Adds a new value to the list
 *
 * @param list Pointer to List.
 * @param value Pointer to the value.
 * @see List
 * @see Node
 * @see node_create
 * @see Express::chain
 *
 * Creates a new node using node_create then pushes the node to the end of the
 * List.
 */
void list_push(List *list, void *value) {
  if (!list || !value)
    return;

  Node *node = node_create(value, NULL, list->tail);
  if (!node)
    return;

  if (list->tail) {
    list->tail->next = node;
    list->tail = node;
  } else {
    list->head = node;
    list->tail = node;
  }
}

/**
 * @brief Frees Node objects from heap.
 *
 * @param list Pointer to List to clear and free.
 * @see List
 * @see Express::chain
 *
 * Just freese the Node object, you must handle Node::value on your own.
 */
void list_clear(List *list) {
  if (!list)
    return;

  list->tail = NULL;
  while (list->head) {
    Node *next = list->head->next;
    free(list->head);
    list->head = next;
  }
}

/**
 * @brief Pops the first Node from the beginning of the chain.
 *
 * @param list Pointer to the List to pop from.
 * @return Pointer to the value holded by the popped Node.
 * @see Node
 *
 * The value is never allocated or freed by the list functions.
 * Do it on your own.
 *
 * Simple example to free all values holded by the linked list.
 *
 * ~~~~~~~~~~~~~~~~~~~~~{.c}
 * Express app = express_create();
 * ...
 * void *value = NULL;
 *
 * while (value = list_shift(&app.chain)) {
 *  free(value);
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~
 *
 * The previous example could fail if a value was set as *NULL*.
 * So, implement your types well.
 */
void *list_shift(List *list) {
  if (!list || !list->tail)
    return NULL;

  Node *node = list->head;
  if (list->head == list->tail)
    list->head = list->tail = NULL;
  else {
    node->next->prev = NULL;
    list->head = node->next;
  }

  void *value = node->value;
  free(node);
  return value;
}

/* =============== Express ================== */

Express express_create() {
  Express app = {0};

  if (pthread_mutex_init(&app.lock, NULL) != 0) {
    fprintf(stderr, "Failed to initialize lock\n");
    exit(1);
  }

  return app;
}

void express_destroy(Express *app) {
  list_clear(&app->chain);
  pthread_mutex_destroy(&app->lock);
}

/**
 * @brief Adds a callback to the Express chain.
 *
 * @param app Pointer to Express object.
 * @param cb Pointer to ExpressCallback function.
 *
 * This function will do nothing if **app** or **cb** is **NULL**.
 */
void express_add(Express *app, ExpressCallback cb) {
  if (!app || !cb)
    return;
  pthread_mutex_lock(&app->lock);
  list_push(&app->chain, cb);
  pthread_mutex_unlock(&app->lock);
}

/**
 * @brief Executes Express chain of callbacks
 *
 * @param app Pointer to express object.
 *
 * Call this function after adding all callbacks to **Express**.
 *
 * @see Express
 * @see express_add
 */
void express_execute(Express *app) {
  if (!app)
    return;

  pthread_mutex_lock(&app->lock);

  ExpressCallback cb = NULL;
  ExpressCommand cmd = E_CONTINUE;

  while (cmd == E_CONTINUE) {
    cb = list_shift(&app->chain);
    if (cb)
      cmd = cb();
  }

  pthread_mutex_unlock(&app->lock);
}

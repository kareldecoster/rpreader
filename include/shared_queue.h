#ifndef MYSHAREDQUEUE_
#define MYSHAREDQUEUE_


typedef struct queue queue_t;
typedef void* element_ptr_t;
typedef queue_t* queue_ptr_t;

//Define callback functions.
typedef void destroy_element_func(element_ptr_t*);
typedef void copy_element_func(element_ptr_t* dest_element, element_ptr_t src_element);
typedef int compare_element_func(element_ptr_t x, element_ptr_t y);
typedef void print_element_func(element_ptr_t element);


/*
 ** The default queue size is 5
 */
#ifndef QUEUE_SIZE
    #define QUEUE_SIZE 100
#endif

/*
 **  Creates (memory allocation!) and initializes the queue and prepares it for usage
 **  Return a pointer to the newly created queue
 **  Returns NULL if queue creation failed
 */
queue_ptr_t queue_create( destroy_element_func *, copy_element_func*, compare_element_func*, print_element_func* , int amnt_readers);

/*  
 **  Add an element to the queue
 **  Does nothing if queue is full
 */
void queue_enqueue(queue_t* queue, element_ptr_t element);

/*
 **  Delete the queue from memory; set queue to NULL
 **  The queue can no longer be used unless queue_create is called again
 */
void queue_free(queue_t** queue);

/*
 **  Return the number of elements in the queue
 */
int queue_size(queue_t* queue);

/*
 **  Return a pointer to the top element in the queue
 **  Returns NULL if queue is empty
 */
element_ptr_t queue_top(queue_t* queue);

/*
 **  Remove the top element from the queue
 **  Does nothing if queue is empty
 */
void queue_dequeue(queue_t* queue);

void queue_print(queue_t* queue);

#endif //MYSHAREDQUEUE_



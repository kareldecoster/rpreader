#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include "../include/shared_queue.h"


struct queue{
	int head;
	int tail;
	int size;
	int max_size;
	element_ptr_t* data;
	int dq_rqs_left;
	int amnt_of_readers;
	destroy_element_func* destroy_element; // callback function
	copy_element_func* copy_element;
	compare_element_func* compare_element;
	print_element_func* print_element;
	pthread_mutex_t* data_mutex;

};

void pthread_err_handler( int err_code, char *msg, char *file_name, char line_nr )
{
	if ( 0 != err_code )
	{
		fprintf( stderr, "\n%s failed with error code %d in file %s at line %d\n", msg, err_code, file_name, line_nr );

	}
}

// Returns a pointer to a newly-allocated queue.
queue_ptr_t queue_create(	destroy_element_func *destroy_element, 
							copy_element_func* copy_element,
							compare_element_func* compare_element,
							print_element_func* print_element,
							int amnt_readers){
							
	queue_ptr_t queue = NULL;
	if((queue = malloc(sizeof(queue_t)))==NULL){
		printf("Malloc of queue failed\n");
		return NULL;
	}
	queue->head = 0;
	queue->tail = 0;
	queue->size = 0;
	queue->max_size = QUEUE_SIZE;
  	queue->destroy_element = destroy_element;
  	queue->copy_element = copy_element;
  	queue->compare_element = compare_element;
  	queue->print_element = print_element;
  	queue->amnt_of_readers = amnt_readers;
  	queue->dq_rqs_left = queue->amnt_of_readers-1;
  	queue->data_mutex = malloc(sizeof(pthread_mutex_t));
  	pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
  	*queue->data_mutex  = data_mutex;
  	
	if(((queue->data) = malloc((queue->max_size) * sizeof(element_ptr_t)))==NULL){
		printf("Malloc of data pointers failed\n");
		free(queue);
		queue=NULL;
		return NULL;
	}
	int i;
	for(i=0; i<(queue->max_size);i++){
		queue->data[i] = NULL;
	}	
	return queue;
}

void queue_free(queue_t** queue){
	assert(*queue != NULL);
	while((*queue)->size>0){
		queue_dequeue(*queue);
	}
	//free the data
	free((*queue)->data);
	(*queue)->data = NULL;
	//free the mutex
	free((*queue)->data_mutex);
	(*queue)->data_mutex = NULL;
	//free the queue
	free(*queue);
	*queue = NULL;
}

int queue_size(queue_t* queue){
	assert(queue != NULL);
	return queue->size;

}

void queue_enqueue(queue_t* queue, element_ptr_t element){
	assert(queue != NULL);
	int res;
	res = pthread_mutex_lock(queue->data_mutex);
	pthread_err_handler( res, "pthread_mutex_lock", __FILE__, __LINE__ );
	//IN LOCK
		if(queue->size < queue->max_size){
			queue->size = queue->size + 1;
		}else{
			queue->tail = (queue->tail + 1) % queue->max_size;
			queue->destroy_element(&(queue->data[queue->head]));
		}	
		queue->copy_element(&(queue->data[queue->head]), element);
		queue->head = (queue->head + 1 ) % queue->max_size;
	//OUT OF LOCK
	res = pthread_mutex_unlock( queue->data_mutex );
	pthread_err_handler( res, "pthread_mutex_unlock", __FILE__, __LINE__ );
}

element_ptr_t queue_top(queue_t* queue){
	return  queue->data[queue->tail];
}

void queue_dequeue(queue_t* queue){
	assert(queue != NULL);
	int res;
	res = pthread_mutex_lock( queue->data_mutex );
	pthread_err_handler( res, "pthread_mutex_unlock", __FILE__, __LINE__ );
	//IN LOCK
		if(queue->size<1){
		}else{
			if(queue->dq_rqs_left==0){
				queue->destroy_element(&(queue->data[queue->tail]));
				queue->data[queue->tail] = NULL;
				queue->size--;
				queue->tail = (queue->tail + 1)%(queue->max_size);
			
				queue->dq_rqs_left = queue->amnt_of_readers -1;
			}else{
				queue->dq_rqs_left--;
			}
		//OUT OF LOCK
		res = pthread_mutex_unlock( queue->data_mutex );
		pthread_err_handler( res, "pthread_mutex_unlock", __FILE__, __LINE__ );
	}
}

void queue_print(queue_t* queue){
	int index = 0;
	while(index < queue->size) {
		printf("On index %d : ", ((queue->tail + index)%(queue->max_size)));
		queue->print_element(queue->data[(queue->tail + index)%(queue->max_size)]);	
		index++;
	}
	printf("###############################\n");
}
















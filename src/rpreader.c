#define _GNU_SOURCE
#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

//my includes
#include <shared_queue.h>
#include <rplidar.h>
#include <config.h>
#include <conn_mgr.h>

//FUNCTION PROTOTYPES
void destroy_element(element_ptr_t*);
void copy_element(element_ptr_t*, element_ptr_t);
int compare_element(element_ptr_t, element_ptr_t);
void print_element(element_ptr_t);
void run_child (int);
void write_to_fifo(char*);
void* rp_mgr_thread_loop();
void* conn_mgr_thread_loop();
void sig_handler(int);
void child_sig_handler(int);
void print_help();

//GLOBAL VARIABLES
FILE* fifo;
queue_ptr_t queue;
volatile int done = 0;
long sequence_number = 0;
int server_port;
char server_ip[] = "000.000.000.000"; 


int main(int argc, char *argv[] ){
    //checking command line arguements
    if (argc != 3){
        print_help();
        exit(EXIT_SUCCESS);
    }
    else{
        strncpy(server_ip, argv[1],strlen(server_ip));
        server_port = atoi(argv[2]);
    }
    
	pid_t child_pid; 
	int result;
	 
	result = mkfifo("my_fifo", 0666);
	if ( result == -1 )								
	{	
		if(errno != EEXIST){								
			perror("Error executing syscall");			
			exit( EXIT_FAILURE );	
		}					
	}	
	child_pid = fork();
	if ( child_pid == -1 )								
	{												
		perror("Error executing syscall");			
		exit( EXIT_FAILURE );						
	}
	  
	if ( child_pid == 0  )
	{  
		fifo = fopen("my_fifo", "r");
		run_child( 0 );
		return 0;
	}
	else
	{ 
		fifo = fopen("my_fifo", "w");
		//RUN PARENT HERE!
		signal(SIGINT, sig_handler);
		pthread_t rp_mgr;
		pthread_t conn_mgr;
		int amount_of_readers = 1;
		queue = queue_create(&destroy_element, &copy_element, &compare_element, &print_element, amount_of_readers);
		assert(queue!=NULL);
		
		char msg[50];
		sprintf(msg, "Adress %s : %d\n", server_ip, server_port);
		write_to_fifo(msg);
		//Start the threads
		pthread_create(&rp_mgr, NULL, &rp_mgr_thread_loop, NULL);
		pthread_create(&conn_mgr, NULL, &conn_mgr_thread_loop, NULL);
		
		
		//Wait for all your threads to close.
		pthread_join(conn_mgr, NULL);
		pthread_join(rp_mgr, NULL);
		
		//Only after all threads have stopped!
		queue_free(&queue);
		fclose(fifo);
		return 0;
	}
}

//LOOP FOR THE RPLIDAR MANAGER THREAD
void* rp_mgr_thread_loop(){
	write_to_fifo("Started RPlidar thread.\n");
	rplidar_run(&done, queue);
	write_to_fifo("Exited RPlidar thread.\n");
	pthread_exit(NULL);
}

//LOOP FOR THE CONNECTION MANAGER THREAD
void* conn_mgr_thread_loop(){
    write_to_fifo("Started Connection thread.\n");
    conn_mgr_run(server_ip, server_port, queue, &done);
    write_to_fifo("Exited Connection thread.\n");
    pthread_exit(NULL);
}

//LOOP FOR THE LOG PROCESS
void run_child ( int exit_code )
{
	signal(SIGINT, child_sig_handler);
	char* result;
	char recv_buf[70];
	FILE* fp;
	fp = fopen("log", "w"); 
	do 
	{
		result = fgets(recv_buf, 70, fifo);
		if ( result != NULL )
	 	{ 
			fputs(recv_buf, fp);
	  	}
	} while ( result != NULL );
	fclose(fifo);
	fclose(fp);
	exit( exit_code ); // this terminates the child process
  
}



void write_to_fifo(char* message){
	char send_buf[100];
	char buff[20];
	time_t ltime; /* calendar time */
	ltime=time(NULL); /* get current cal time */
	strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&ltime));
	sprintf(send_buf,"%ld %s : %s",sequence_number++, buff, message);
	if ( fputs( send_buf, fifo ) == EOF ){
	  fprintf( stderr, "Error writing data to fifo\n");
	  exit( EXIT_FAILURE );
	} 
	fflush(fifo);
}

void destroy_element(element_ptr_t* element)  
{
	free(*element);
	*element = NULL;
}

void copy_element(element_ptr_t* dest_element, element_ptr_t src_element){
	*dest_element = src_element;
}

int compare_element(element_ptr_t x, element_ptr_t y){
	rplidar_data_t* a = (rplidar_data_t*) x;
	rplidar_data_t* b = (rplidar_data_t*) y;
  if(a->ts == b->ts){
  	return 0;
  }
  if((a->ts) > (b->ts)){
  	return 1;
  }else{
  	return -1;
  }
  
}

void print_element(element_ptr_t element){
	rplidar_data_t* a = (rplidar_data_t*) element;
	printf("Measurement --- Position : %lf ; %lf\n", (a->x), (a->y));
	
}

void sig_handler(int signum){
	done = 1; //This will end the infinite loops of the threads.
	printf("\nSHUTTING DOWN...\n");
}

void child_sig_handler(int signum){
	if(signum == SIGINT){
	//We dont want the child to get killed yet, it must exit when the fifo has no more writers to it.
	}
}

void print_help(void){
  printf("Use this program with 2 command line options: \n");
  printf("\t%-15s : TCP server IP address\n", "\'server IP\'");
  printf("\t%-15s : TCP server port number\n", "\'server port\'");
}

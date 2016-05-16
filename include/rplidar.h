#ifndef RPLIDAR_
#define RPLIDAR_


#define BAUDRATE        B115200    // Device baudrate for UART communication
#define PORT            "/dev/ttyO5"   // Serial Port to connect to. tty0x with x the UART port number. Replace by ttyUSB0 for USB


#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <math.h>
#include <errno.h>

//my includes
#include <shared_queue.h>
#include <config.h>


typedef struct rplidar rplidar_t;


/* connection functions */ 
rplidar_t* rplidar_create(); /* create new rplidar connection */
void rplidar_destroy( rplidar_t** rplidar ); /* close the connection with the rplidar */

int rplidar_run(volatile int *done, queue_ptr_t queue);

#endif //RPLIDAR_

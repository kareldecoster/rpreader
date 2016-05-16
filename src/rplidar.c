#include <rplidar.h>


//internal functions
void _stop_scan(rplidar_t*);
int _start_scan(rplidar_t*);
void _reset(rplidar_t*);
int _check_header(rplidar_t*);
rplidar_data_t* _init_rplidar_data();
void _read_scan(rplidar_t*, rplidar_data_t*);


struct rplidar {
    int serial_fd;  /* port file descriptor */
};


int rplidar_run(volatile int *done, queue_ptr_t queue){
    //setup
    system("echo 23 > /sys/class/gpio/export");
    system("echo out > /sys/class/gpio/gpio23/direction");
    system("echo 1 > /sys/class/gpio/gpio23/value");
    rplidar_t* rplidar = rplidar_create();
    //usleep(20000);
    //_reset(rplidar);
    //usleep(20000);
    if(_start_scan( rplidar) == 1){
    	printf("Error starting up\n");
        usleep(20000);
        _reset(rplidar);
        *done = 1;
    }
    //loop
    while(!(*done)){
        rplidar_data_t* data = _init_rplidar_data();
        _read_scan(rplidar,data);
        queue_enqueue(queue, data);
    }
    //exit
    system("echo 23 > /sys/class/gpio/unexport");
    rplidar_destroy(&rplidar);
    return EXIT_SUCCESS;
}

/* open a new rplidar connection */
rplidar_t* rplidar_create() {
    rplidar_t* rplidar = malloc( sizeof( rplidar_t ) );
    if ( !rplidar ) {
        printf( "Malloc error \n" );
        return NULL;
    }

    char port[20] = PORT; /* port to connect to */
    speed_t baud = BAUDRATE; /* baud rate */

    rplidar->serial_fd = open( port, O_RDWR | O_NOCTTY | O_NDELAY ); /* connect to port */
    if ( rplidar->serial_fd == -1 ) {
        printf("Error connecting \n");
        return 0;
    }

    /* set UART settings (always for RPlidar: 115200 8N1) */
    struct termios settings;
    tcgetattr( rplidar->serial_fd, &settings );

    cfsetispeed( &settings, baud ); /* set baud rate */
    cfsetospeed( &settings, baud ); /* set baud rate */
    settings.c_cflag |= ( CLOCAL | CREAD ); /* enable RX & TX */ //CLOCAL: ignore modem status lines (?)
    settings.c_cflag &= ~PARENB; /* no parity */
    settings.c_cflag &= ~CSTOPB; /* 1 stop bit */
    settings.c_cflag &= ~CSIZE; /* character size mask. Do this before setting size with CS8 */
    settings.c_cflag |= CS8;  /* 8 data bits */

    //settings.c_cflag &= ~CNEW_RTSCTS; // no hw flow control (not supported?)
    settings.c_iflag &= ~(IXON | IXOFF | IXANY); // no sw flow control
    settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);    // raw input mode
    settings.c_oflag &= ~OPOST;    // raw output mode

    tcflush( rplidar->serial_fd, TCIFLUSH );
    
    if ( fcntl( rplidar->serial_fd, F_SETFL, FNDELAY ) ) {
        printf("Error: Line number %d in file %s \n", __LINE__, __FILE__);
    }

    settings.c_cc[VTIME] = 0;	/* inter-character timer unused */
    settings.c_cc[VMIN] = 1;	/* blocking read until 1 chars received */
    
    if ( tcsetattr( rplidar->serial_fd, TCSANOW, &settings ) ) {
        printf("Error: line number %d in file %s \n", __LINE__, __FILE__);
    }
    tcflush( rplidar->serial_fd, TCOFLUSH );

    return rplidar;
}

/* close the connection with the rplidar */
void rplidar_destroy( rplidar_t** rplidar ) {
    uint8_t i = close( (*rplidar)->serial_fd );
    if ( i == -1 ) printf( "error closing serial port" );
    free( *rplidar );
    *rplidar = NULL;
}

/*
*   Private functions
*/

void _stop_scan(rplidar_t* rplidar){
   uint8_t msg[2] = {0xA5, 0x25};
   write(rplidar->serial_fd, msg, 2);
}

int _start_scan(rplidar_t* rplidar){
    uint8_t msg[2] = {0xA5, 0x20};
    write(rplidar->serial_fd, msg, 2);
    return _check_header(rplidar);
}


void _reset(rplidar_t* rplidar){
    uint8_t msg[2] = {0xA5, 0x40};
    write(rplidar->serial_fd, msg, 2);
    tcflush( rplidar->serial_fd, TCIFLUSH );
}

int _check_header(rplidar_t* rplidar){
    uint8_t header[7];
    int index = 0;
	int size = 0;
	while(index < 7){
		size = read(rplidar->serial_fd, &header[index], 7-index);
		if(size > 0){
			index += size;
		}
	}
	printf("%x %x %x %x %x %x %x \n",header[0], header[1], header[2], header[3], header[4], header[5], header[6]);
	int failed = 0;
    if(0xA5!=header[0] || 0x5A!=header[1]) {
        failed = 1;
	}if(((header[5]&0xC0)>>6) != 0x01) {
        failed = 1;
	}if(header[2] != 5) {
        failed = 1;
	}if(header[6] != 0x81){
        failed = 1;
    }
	return failed;
}

rplidar_data_t* _init_rplidar_data(){
    rplidar_data_t* data = malloc(sizeof(rplidar_data_t));
    FILE* fp;
    if(data != NULL){
        fp = fopen("/var/lib/mercator/x","r");
        if(fp != NULL){
            
            fscanf(fp,"%lf",&(data->x));
            fclose(fp);
        }else{
            data->x = 6;
        }
        fp = fopen("/var/lib/mercator/y","r");
        if(fp != NULL){
            fscanf(fp,"%lf",&(data->y));
            fclose(fp);
        }else{
            data->y = 6;
        }
        time(&(data->ts));
    }
    return data;
}

void _read_scan(rplidar_t* rplidar, rplidar_data_t* data){
    uint16_t size = 360*5;// 5 bytes per measurement, 360 measurements per scan
    uint16_t index = 0;
    uint8_t buffer[360*5]; 
    //read in a full scan
    //int bytes_available;
    //ioctl(rplidar->serial_fd, FIONREAD, &bytes_available);
    //printf("Bytes available = %d \n", bytes_available);
    while(index < size){
		int16_t bytes_read = read(rplidar->serial_fd, &buffer[index], size-index);
		if(bytes_read > 0){
			index += bytes_read;
			//printf("%d bytes read. Now  reading %d / %d \n",bytes_read, index, size);
		}else{
		    //printf("%s\n", strerror(errno));
		}
	}
	//put values into data
	index = 0;
	while (index < 360){
        uint8_t quality = buffer[5*index] & 0xFC;
        uint8_t start_flag = buffer[5*index] & 0x01;
        uint8_t inv_start_flag = (buffer[5*index] & 0x02)>>1;
        uint8_t check_flag = buffer[5*index + 1] & 0x01;
        if((quality > 0) && (inv_start_flag != start_flag) && (check_flag == 1)){ // Good Measurement
            int16_t angle = (128 * buffer[5*index + 2] + (buffer[5*index + 1]>>1))>>6;
            data->scan[(angle+270)%360] = ((buffer[5*index + 4]<<8) + buffer[5*index + 3]) / 4. ;           
        }
        index++;
	}
    
}

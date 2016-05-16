#include <conn_mgr.h>

void conn_mgr_run(char* server_ip, int server_port, queue_ptr_t queue, volatile int *done){
    //setup
    if( server_ip == NULL || server_port == 0 || queue == NULL || done == NULL){
		return;
    }
    Socket client = tcp_active_open(server_port, server_ip);
    
    while(!(*done)){
        rplidar_data_t* data = (rplidar_data_t*)queue_top(queue);
        if(data != NULL){
        	/* Make local copies so we can dequeue asap */
        	time_t timestamp = time(&timestamp);
            double x = data->x;
            double y = data->y;
            double scan[360];
            memcpy(scan, data->scan, 360*sizeof(double));
            //printf("Scan Pos = %lf : %lf | distance = %lf\n", x, y, scan[0]);
            queue_dequeue(queue);
            data = NULL;
            tcp_send(client, (void *)&timestamp, sizeof(time_t));
            tcp_send(client, (void *)&x, sizeof(double));
            tcp_send(client, (void *)&y, sizeof(double));
            tcp_send(client, (void *)scan, 360*sizeof(double));          
        }  
    }
    tcp_close(&client);      
    //clean up
}

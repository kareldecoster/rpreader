#ifndef _CONN_MGR_H_
#define _CONN_MGR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//my includes
#include <config.h>
#include <shared_queue.h>
#include <tcpsocket.h>

void conn_mgr_run(char* server_ip, int server_port, queue_ptr_t queue, volatile int *done);

#endif

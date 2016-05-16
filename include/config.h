#ifndef _CONFIG_H_
#define _CONFIG_H_


#include <stdint.h>
#include <time.h>

/*------------------------------------------------------------------------------
		definitions (defines, typedefs, ...)
------------------------------------------------------------------------------*/

typedef struct{
	double scan[360];
	double x;
	double y;
	time_t ts;
}rplidar_data_t, * rplidar_data_ptr_t;
			

#endif /* _CONFIG_H_ */


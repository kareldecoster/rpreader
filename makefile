IDIR = ./include
CFLAGS = -Wall -Werror -g -pthread -I$(IDIR)
CC = gcc

ODIR = ./src/obj
LDIR =./lib

_LIBS=
LIBS = $(patsubst %,$(LDIR)/%,$(_LIBS))

_DEPS = rplidar.h config.h shared_queue.h conn_mgr.h tcpsocket.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = rpreader.o rplidar.o shared_queue.o conn_mgr.o tcpsocket.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: ./src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
all: $(OBJ) 
	$ make rpreader

rpreader: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	
clean: 
	$ rm -f $(ODIR)/*.o core $(INCDIR)/*~
	$ rm rpreader
	$ clear

	
	
.PHONY: clean

#
#	Scheduler Algorithm Simulator 
#	Github id : virtus2
#	 
#
#   Makfeile :
#       - Makefile for sched compilation.
#

CC = gcc
INC = -I${CURDIR}/include/
CFLAGS = -g $(INC)

OBJ_SCHED = sched.o sched_test.o 

SRCS = $(OBJ_SCHED:.o=.c)

TARGET_SCHED = sched

.SUFFIXES : .c .o

.c.o:
	@echo "Compilingi scheduler simulator $< ..."
	$(CC) -c $(CFLAGS) -o $@ $<

$(TARGET_SCHED) : $(OBJ_SCHED)
	$(CC) -o $(TARGET_SCHED) $(OBJ_SCHED)

all : $(TARGET_SCHED)

dep : 
	gccmaedep $(INC) $(SRCS)

clean :
	@echo "Cleaning scheduler simulator $< ..."
	rm -rf $(OBJ_SCHED) $(TARGET_SCHED) 

new :
	$(MAKE) clean
	$(MAKE)

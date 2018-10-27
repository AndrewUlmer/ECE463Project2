CFLAGS = -g -Wall
CC = gcc

# change based on type of router to be built
# value can be either DISTVECTOR or PATHVECTOR
ROUTERMODE = DISTVECTOR

# if DEBUG is 1, debugging messages are printed
DEBUG = 0

# Check which OS
OS := $(shell uname)
ifeq ($(OS), SunOS)
	SOCKETLIB = -lsocket
endif

all : router

endian.o   :   ne.h endian.c
	$(CC) $(CFLAGS) -D $(ROUTERMODE) -c endian.c

routingtable.o   :   ne.h routingtable.c
	$(CC) $(CFLAGS) -D $(ROUTERMODE) -c routingtable.c
	
router  :   endian.o routingtable.o router.c
	$(CC) $(CFLAGS) -D $(ROUTERMODE) -D DEBUG=$(DEBUG) endian.o routingtable.o router.c -o router -lnsl $(SOCKETLIB)

unit-test  : routingtable.o unit-test.c
	$(CC) $(CFLAGS) -D $(ROUTERMODE) -D DEBUG=$(DEBUG) routingtable.o unit-test.c -o unit-test -lnsl $(SOCKETLIB)

R : router
	./router 0 localhost 9999 8080	

NE : 
	./ne 9999 1_routers.conf

router0 : router
	./router 0 localhost 2000 3000

router1 : router
	./router 1 localhost 2000 3001

router2 : router
	./router 2 localhost 2000 3002

router3 : router
	./router 3 localhost 2000 3003

clean :
	rm -f *.o
	rm -f router
	rm -f unit-test

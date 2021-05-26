CFLAGS = -Wall -Werror -g
CC     = gcc $(CFLAGS)

########################################
# Programs / Object files to build

bl_server.o : bl_server.c blather.h
	$(CC) -c bl_server.c

server_funcs.o : server_funcs.c blather.h
	$(CC) -c server_funcs.c

util.o : util.c blather.h
	$(CC) -c util.c

bl_server : bl_server.o	 server_funcs.o	util.o blather.h
	$(CC) -o bl_server  bl_server.o	 server_funcs.o	util.o -lpthread

bl_client.o : bl_client.c blather.h
	$(CC) -c bl_client.c

simpio.o : simpio.c blather.h
	$(CC) -c simpio.c

bl_client : bl_client.o	 simpio.o util.o blather.h
	$(CC) -o bl_client  bl_client.o	 simpio.o util.o -lpthread

simpio_demo.o : simpio_demo.c blather.h
	$(CC) -c simpio_demo.c 

simpio_demo : simpio_demo.o	 simpio.o util.o blather.h
	$(CC) -o simpio_demo  simpio_demo.o simpio.o util.o -lpthread

clean:
	rm -f bl_server server_funcs util bl_client simpio simpio_demo *.o



########################################
help :
	@echo 'Typical usage is:'
	@echo '  > make bl_server                 build the server'
	@echo '  > make bl_client                 build the client'
	@echo '  > make clean                     remove all compiled items'
	@echo '  > make clean-tests               remove temporary files generated during testing'
	@echo '  > make zip                       create a zip file for submission'
	@echo '  > make test                      run all tests'
########################################

#include test_Makefile

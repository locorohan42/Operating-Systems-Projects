CFLAGS = -Wall -Werror -g 
CC     = gcc $(CFLAGS)

########################################
# Programs / Object files to build

bl_client.o : bl_client.c blather.h
	$(CC) -c bl_client.c 

bl_server.o : bl_server.c blather.h
	$(CC) -c bl_server.c

server_funcs.o : server_funcs.c blather.h
	$(CC) -c server_funcs.c 	

simpio.o : simpio.c blather.h
	$(CC) -c simpio.c 	

util.o : util.c blather.h
	$(CC) -c util.c 

blather: bl_client.o bl_server.o server_funcs.o simpio.o util.o blather.h
	$(CC) -o blather bl_client.o bl_server.o server_funcs.o simpio.o util.o	

clean:
	rm -f bl_client bl_server server_funcs simpio util *.o

	

########################################
help :
	@echo 'Typical usage is:'
	@echo '  > make commando                  build all programs'
	@echo '  > make clean                     remove all compiled items'
	@echo '  > make clean-test                remove temporary files generated during testing'
	@echo '  > make zip                       create a zip file for submission'
	@echo '  > make test                      run all tests'
	@echo '  > make test-cmd                  run tests that verify cmd functions'
	@echo '  > make test-commando             run tests that verify commando application works'
########################################

include test_Makefile
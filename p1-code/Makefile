CFLAGS = -Wall -Werror -g 
CC     = gcc $(CFLAGS)

########################################
# Programs / Object files to build

commando.o : commando.c commando.h
	$(CC) -c commando.c 

cmd.o : cmd.c commando.h 
	$(CC) -c cmd.c

cmdcol.o : cmdcol.c commando.h 
	$(CC) -c cmdcol.c 	

util.o : util.c commando.h 
	$(CC) -c util.c 

commando: commando.o cmd.o cmdcol.o util.o commando.h
	$(CC) -o commando commando.o cmd.o cmdcol.o util.o	

clean:
	rm -f commando cmd cmdcol util *.o

	

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
# Testing Targets

# test : test-commando test-cmd

# test_cmd : test_cmd.c cmd.o 
# 	$(CC) -o test_cmd.c

# test-cmd : test_cmd 
# 	@chmod u+x testy 
# 	./testy test_cmd.org $(testnum)	

# test-commando : cmd.o cmdol.o util.o commando.o	
# 	@chmod u+x testy 
# 	./testy test_commando.org $(testnum)

# clean:
# 	@echo Removing temporary test files
# 	rm -rf *.o test-results

# > make test                 # run all tests, both of the above
# > make test-cmd             # test functions in the cmd.c and cmdcol.c files
# > make test-cmd testnum=9   # test functions, only run test 9
# > make test-commando        # test the commando application
# > make clean-tests          # remove temporary files generated during testing
# > make zip                  # create a zip of the project for submission


include test_Makefile
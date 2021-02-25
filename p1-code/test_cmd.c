#include <string.h>
#include "commando.h"
#include <sys/stat.h>
#include <fcntl.h>

#define PRINT_TEST sprintf(sysbuf,"awk 'NR==(%d+1){P=1;print \"{\"} P==1 && /ENDTEST/{P=0; print \"}\"} P==1{print}' %s", __LINE__, __FILE__); \
                   system(sysbuf);

void test_print_cmd(cmd_t *cmd){
  if(cmd == NULL){
    printf("cmd is NULL\n" );
    return;
  }
  printf("cmd->name: %s\n", cmd->name);
  printf("cmd->argv[]:\n" );
  for(int i=0; i<ARG_MAX+1; i++){
    printf("  [%3d] : %s\n",i,cmd->argv[i]);
    if(cmd->argv[i] == NULL){
      break;
    }
  }
  printf("cmd->pid > 0 : %s\n", (cmd->pid > 0) ? "yes" : "no" );
  printf("cmd->pid: #%d\n", cmd->pid);
  printf("cmd->out_pipe[PREAD]  > 0: %s\n", (cmd->out_pipe[PREAD]  > 0) ? "yes" : "no" );
  printf("cmd->out_pipe[PWRITE] > 0: %s\n", (cmd->out_pipe[PWRITE] > 0) ? "yes" : "no" );
  printf("cmd->status: %d\n", cmd->status);
  printf("cmd->str_status: %s\n", cmd->str_status);
  printf("cmd->finished: %d\n", cmd->finished);
  printf("cmd->output_size: %d\n",cmd->output_size);
  printf("cmd->output:\n%s\n", (char *) cmd->output);
}

void assert_pointers_unique(char *argv[], cmd_t *cmd){
  for(int i=0; argv[i] != NULL; i++){
    if(argv[i] == cmd->argv[i]){
      printf("argv[%d] == cmd->argv[%d], should point to different locations\n",i,i);
      printf("Must strdup() argv entries to create independent memory location for cmd->argv entries\n" );
    }
  }
}


int main(int argc, char *argv[]){
  if(argc < 2){
    printf("usage: %s <test_name>\n", argv[0]);
    return 1;
  }
  char *test_name = argv[1];
  char sysbuf[1024];

  if(0){}
  else if( strcmp( test_name, "cmd_new_1" )==0 ) {
    PRINT_TEST;
    // Tests creation and init of a cmd_t along
    // with cmd_free() to deallocate it
    char *argv[] = {
      "cat",
      "test_data/quote.txt",
      "test_data/gettysburg.txt",
      NULL
    };
    cmd_t *cmd = cmd_new(argv);
    test_print_cmd(cmd);
    assert_pointers_unique(argv, cmd);
    cmd_free(cmd);
  } // ENDTEST

  else if( strcmp( test_name, "cmd_new_2" )==0 ) {
    PRINT_TEST;
    // Tests creation and init of a cmd_t along
    // with cmd_free() to deallocate it
    char *argv[] = {
      "grep",
      "-i",
      "the ",
      "test_data/gettysburg.txt",
      NULL
    };
    cmd_t *cmd = cmd_new(argv);
    test_print_cmd(cmd);
    assert_pointers_unique(argv, cmd);
    cmd_free(cmd);
  } // ENDTEST

  else if( strcmp( test_name, "cmd_new_3" )==0 ) {
    PRINT_TEST;
    // Tests creation and init of a cmd_t along
    // with cmd_free() to deallocate it
    char *argv[] = {
      "ls",
      "-F",
      "-a ",
      "-1",
      "test_data/stuff",
      NULL
    };
    cmd_t *cmd = cmd_new(argv);
    test_print_cmd(cmd);
    assert_pointers_unique(argv, cmd);
    cmd_free(cmd);
  } // ENDTEST

  else if( strcmp( test_name, "cmd_start_1" )==0 ) {
    PRINT_TEST;
    // Tests cmd_start() to properly adjust data in
    // a cmd_t after forking a child process to run
    // the command. Fields such as str_status should
    // reflect that the command has started running.
    // If the pipes have been setup properly, no output
    // will appear on the screen for the child command.
    char *argv[] = {
      "cat",
      "test_data/quote.txt",
      "test_data/gettysburg.txt",
      NULL
    };
    cmd_t *cmd = cmd_new(argv);
    assert_pointers_unique(argv, cmd);
    cmd_start(cmd);
    test_print_cmd(cmd);
    cmd_free(cmd);
  } // ENDTEST


  else if( strcmp( test_name, "cmd_start_2" )==0 ) {
    PRINT_TEST;
    // Tests cmd_start() to properly adjust data in
    // a cmd_t after forking a child process to run
    // the command. Fields such as str_status should
    // reflect that the command has started running.
    // If the pipes have been setup properly, no output
    // will appear on the screen for the child command.
    char *argv[] = {
      "grep",
      "-i",
      "the ",
      "test_data/gettysburg.txt",
      NULL
    };
    cmd_t *cmd = cmd_new(argv);
    assert_pointers_unique(argv, cmd);
    cmd_start(cmd);
    test_print_cmd(cmd);
    cmd_free(cmd);
  } // ENDTEST


  else if( strcmp( test_name, "cmd_start_3" )==0 ) {
    PRINT_TEST;
    // Tests cmd_start() to properly adjust data in
    // a cmd_t after forking a child process to run
    // the command. Fields such as str_status should
    // reflect that the command has started running.
    // If the pipes have been setup properly, no output
    // will appear on the screen for the child command.
    char *argv[] = {
      "ls",
      "-F",
      "-a",
      "-1",
      "test_data/stuff",
      NULL
    };
    cmd_t *cmd = cmd_new(argv);
    assert_pointers_unique(argv, cmd);
    cmd_start(cmd);
    test_print_cmd(cmd);
    cmd_free(cmd);
  } // ENDTEST

  else if( strcmp( test_name, "read_all_1" )==0 ) {
    PRINT_TEST;
    // Tests the read_all() function to ensure that
    // it properly accumulates all data from an
    // arbitrary input FD including allocating memory
    // for the data.
    // 
    // NOTE: read_all() should null-terminate the
    // strings it reads and Valgrind might report
    // "Uninitialized Data" if no '\0' character is
    // placed in the character array that is
    // returned.
    int fd = open("test_data/quote.txt", O_RDONLY);
    int bytes_read = -1;
    char *actual_read = read_all(fd, &bytes_read);
    int result = close(fd);
    printf("result: %d\n", result);
    printf("bytes_read: %d\n", bytes_read);
    printf("actual_read:\n" );
    printf("--------------------\n" );
    printf("%s",actual_read);
    printf("--------------------\n" );
    free(actual_read);
  } // ENDTEST

  else if( strcmp( test_name, "read_all_2" )==0 ) {
    PRINT_TEST;
    // Tests the read_all() function to ensure that
    // it properly accumulates all data from an
    // arbitrary input FD including allocating memory
    // for the data. 
    int fd = open("./test_data/gettysburg.txt", O_RDONLY);
    int bytes_read = -1;
    char *actual_read = read_all(fd, &bytes_read);
    int result = close(fd);
    printf("result: %d\n", result);
    printf("bytes_read: %d\n", bytes_read);
    printf("actual_read:\n" );
    printf("--------------------\n" );
    printf("%s",actual_read);
    printf("--------------------\n" );
    free(actual_read);
  } // ENDTEST

  else if( strcmp( test_name, "read_all_3" )==0 ) {
    PRINT_TEST;
    // Tests the read_all() function to ensure that
    // it properly accumulates all data from an
    // arbitrary input FD including allocating memory
    // for the data. 
    int fd = open("./test_data/3K.txt", O_RDONLY);
    int bytes_read = -1;
    char *actual_read = read_all(fd, &bytes_read);
    int result = close(fd);
    printf("result: %d\n", result);
    printf("bytes_read: %d\n", bytes_read);
    printf("actual_read:\n" );
    printf("--------------------\n" );
    printf("%s",actual_read);
    printf("--------------------\n" );
    free(actual_read);
  } // ENDTEST

  else if( strcmp( test_name, "cmd_update_1" )==0 ) {
    PRINT_TEST;
    // Tests whether cmd_update() correctly checks
    // on a running command and blocks the calling
    // process until the cmd is finished. Then checks
    // that the cmd_t is updated correctly based on
    // completion of the cmd.
    char *argv[] = {
      "cat",
      "test_data/quote.txt",
      "test_data/gettysburg.txt",
      NULL
    };
    cmd_t *cmd = cmd_new(argv);
    cmd_start(cmd);                // start running
    cmd_update_state(cmd,DOBLOCK); // wait for completion
                                   // should see an alert
    test_print_cmd(cmd);           // show completed cmd
    cmd_free(cmd);
  } // ENDTEST

  else if( strcmp( test_name, "cmd_update_2" )==0 ) {
    PRINT_TEST;
    // Tests whether cmd_update() correctly checks
    // on a running command and blocks the calling
    // process until the cmd is finished. Then checks
    // that the cmd_t is updated correctly based on
    // completion of the cmd.
    char *argv[] = {
      "grep",
      "-i",
      "the ",
      "test_data/gettysburg.txt",
      NULL
    };
    cmd_t *cmd = cmd_new(argv);
    cmd_start(cmd);                // start running
    cmd_update_state(cmd,DOBLOCK); // wait for completion
                                   // should see an alert
    test_print_cmd(cmd);           // show completed cmd
    cmd_free(cmd);
  } // ENDTEST

  else if( strcmp( test_name, "cmd_update_3" )==0 ) {
    PRINT_TEST;
    // Tests whether cmd_update() correctly checks
    // on a running command and blocks the calling
    // process until the cmd is finished. Then checks
    // that the cmd_t is updated correctly based on
    // completion of the cmd.
    // NOTE: This command returns an exit code of 1 so
    // cmd->status=1  and  cmd->str_status="EXIT(1)"
    char *argv[] = {
      "grep",
      "-i",
      "schmeckles",
      "test_data/gettysburg.txt",
      NULL
    };
    cmd_t *cmd = cmd_new(argv);
    cmd_start(cmd);                // start running
    cmd_update_state(cmd,DOBLOCK); // wait for completion
                                   // should see an alert
    test_print_cmd(cmd);           // show completed cmd
    cmd_free(cmd);
  } // ENDTEST

  else if( strcmp( test_name, "cmd_print_output_1" )==0 ) {
    PRINT_TEST;
    // Tests whether cmd_print_output() correctly
    // captures and prints a completed command's output.
    char *argv[] = {
      "cat",
      "test_data/quote.txt",
      "test_data/gettysburg.txt",
      NULL
    };
    cmd_t *cmd = cmd_new(argv);
    cmd_start(cmd);                // start running
    cmd_update_state(cmd,DOBLOCK); // wait for completion
                                   // should see an alert
    cmd_print_output(cmd);
    cmd_free(cmd);
  } // ENDTEST

  else if( strcmp( test_name, "cmd_print_output_2" )==0 ) {
    PRINT_TEST;
    // Tests whether cmd_print_output() correctly
    // captures and prints a completed command's output.
    char *argv[] = {
      "grep",
      "-i",
      "the ",
      "test_data/gettysburg.txt",
      NULL
    };
    cmd_t *cmd = cmd_new(argv);
    cmd_start(cmd);                // start running
    cmd_update_state(cmd,DOBLOCK); // wait for completion
                                   // should see an alert
    cmd_print_output(cmd);
    cmd_free(cmd);
  } // ENDTEST

  else if( strcmp( test_name, "cmdcol_add_1" )==0 ) {
    PRINT_TEST;
    // Tests whether cmdcol_add() properly adds a cmd to
    // those tracked by a cmdcol and that cmdcol_freeall()
    // correctly frees memory associated with the collection.
    char *children[][5] = {
      {"cat","test_data/quote.txt",NULL},    
      {"test_data/sleep_print","1","wait for me",NULL},
      {NULL},
    };
    cmdcol_t cmdcol_actual = {};
    cmdcol_t *cmdcol = &cmdcol_actual;
    for(int i=0; children[i][0] != NULL; i++){
      cmd_t *cmd = cmd_new(children[i]);
      cmdcol_add(cmdcol, cmd);
    }
    printf("cmdcol->size: %d\n",cmdcol->size);
    for(int i=0; children[i][0] != NULL; i++){
      printf("cmdcol->cmd[%d]->name: %s\n",
             i, cmdcol->cmd[i]->name);
    }
    cmdcol_freeall(cmdcol);
  } // ENDTEST


  else if( strcmp( test_name, "cmdcol_add_2" )==0 ) {
    PRINT_TEST;
    // Tests whether cmdcol_add() properly adds a cmd to
    // those tracked by a cmdcol and that cmdcol_freeall()
    // correctly frees memory associated with the collection.
    char *children[][5] = {
      {"cat","test_data/quote.txt",NULL},    
      {"ls","-a","test_data/stuff",NULL},
      {"ls","-a","-F","test_data/stuff",NULL},
      {"gcc","-o","test_data/test_args","test_data/test_args.c",NULL},
      {"grep","-i", "flurbo ","test_data/gettysburg.txt",NULL},
      {NULL},
    };
    cmdcol_t cmdcol_actual = {};
    cmdcol_t *cmdcol = &cmdcol_actual;
    for(int i=0; children[i][0] != NULL; i++){
      cmd_t *cmd = cmd_new(children[i]);
      cmdcol_add(cmdcol, cmd);
    }
    printf("cmdcol->size: %d\n",cmdcol->size);
    for(int i=0; children[i][0] != NULL; i++){
      printf("cmdcol->cmd[%d]->name: %s\n",
             i, cmdcol->cmd[i]->name);
    }
    cmdcol_freeall(cmdcol);
  } // ENDTEST

  else if( strcmp( test_name, "cmdcol_update_state_1" )==0 ) {
    PRINT_TEST;
    // Adds several commands to a collection, starts those
    // commands, then blocks for updates on all commands
    // via cmdcol_update_state(). Should produce alerts
    // that all of the children have finished and there
    // should be alerts for all children.
    char *children[][5] = {
      {"cat","test_data/quote.txt",NULL},    
      {"ls","-a","test_data/stuff",NULL},
      {"ls","-a","-F","test_data/stuff",NULL},
      {NULL},
    };
    cmdcol_t cmdcol_actual = {};
    cmdcol_t *cmdcol = &cmdcol_actual;
    for(int i=0; children[i][0] != NULL; i++){
      cmd_t *cmd = cmd_new(children[i]);
      cmdcol_add(cmdcol, cmd);
      cmd_start(cmd);
    }
    cmdcol_update_state(cmdcol, DOBLOCK);
    cmdcol_freeall(cmdcol);
  } // ENDTEST

  else if( strcmp( test_name, "cmdcol_update_state_2" )==0 ) {
    PRINT_TEST;
    // Adds several commands to a collection, starts those
    // commands, then blocks for updates on all commands
    // via cmdcol_update_state(). Should produce alerts
    // that all of the children have finished and there
    // should be alerts for all children.
    char *children[][5] = {
      {"cat","test_data/quote.txt",NULL},    
      {"ls","-a","test_data/stuff",NULL},
      {"grep","-i", "flurbo ","test_data/gettysburg.txt",NULL},
      {"ls","-a","-F","test_data/stuff",NULL},
      {"gcc","-o","test_data/print_args",
                   "test_data/print_args.c",NULL},
      {NULL},
    };
    cmdcol_t cmdcol_actual = {};
    cmdcol_t *cmdcol = &cmdcol_actual;
    for(int i=0; children[i][0] != NULL; i++){
      cmd_t *cmd = cmd_new(children[i]);
      cmdcol_add(cmdcol, cmd);
      cmd_start(cmd);
    }
    cmdcol_update_state(cmdcol, DOBLOCK);
    cmdcol_freeall(cmdcol);
  } // ENDTEST

  else if( strcmp( test_name, "cmdcol_print_1" )==0 ) {
    PRINT_TEST;
    // Adds to a cmd collection then calls cmdcol_print()
    // to show the state of the cmds prior to running.
    char *children[][5] = {
      {"cat","test_data/quote.txt",NULL},    
      {"ls","-a","test_data/stuff",NULL},
      {"ls","-a","-F","test_data/stuff",NULL},
      {NULL},
    };
    cmdcol_t cmdcol_actual = {};
    cmdcol_t *cmdcol = &cmdcol_actual;
    for(int i=0; children[i][0] != NULL; i++){
      cmd_t *cmd = cmd_new(children[i]);
      cmdcol_add(cmdcol, cmd);
    }
    cmdcol_print(cmdcol);
    cmdcol_freeall(cmdcol);
  } // ENDTEST

  else if( strcmp( test_name, "cmdcol_print_2" )==0 ) {
    PRINT_TEST;
    // Adds to a cmd collection, runs commands, and
    // the prints showing most commands in the RUN
    // state. Then waits for them to complete via
    // cmdcol_update_state(). Calls cmdcol_print() to
    // show most commands in EXIT(..) state.
    char *children[][5] = {
      {"cat","test_data/quote.txt",NULL},    
      {"ls","-a","test_data/stuff",NULL},
      {"grep","-i", "flurbo ","test_data/gettysburg.txt",NULL},
      {"ls","-a","-F","test_data/stuff",NULL},
      {"gcc","-o","test_data/print_args",
                   "test_data/print_args.c",NULL},
      {NULL},
    };
    cmdcol_t cmdcol_actual = {};
    cmdcol_t *cmdcol = &cmdcol_actual;
    for(int i=0; children[i][0] != NULL; i++){
      cmd_t *cmd = cmd_new(children[i]);
      cmdcol_add(cmdcol, cmd);
      cmd_start(cmd);
    }
    printf("Before cmdcol_update_state()\n");
    cmdcol_print(cmdcol);       // RUN

    cmdcol_update_state(cmdcol, DOBLOCK);

    printf("\n");
    printf("After cmdcol_update_state()\n");
    cmdcol_print(cmdcol);       // EXIT(..)
    cmdcol_freeall(cmdcol);
  } // ENDTEST

  else{
    printf("No test named '%s' found\n",test_name);
    return 1;
  }
  return 0;
}

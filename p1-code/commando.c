#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "commando.h"

int main(int argc, char *argv[]) {
  setvbuf(stdout, NULL, _IONBF, 0); // Turn off output buffering
  cmdcol_t * cmdcol = cmdcol_new();
  int echo = 0;
  char * env = getenv("COMMANDO_ECHO"); // Find value of COMMANDO_ECHO
  if(argv[1] != NULL){ // Check for --echo if argv[1] is set
    echo = strncmp(argv[1], "--echo", 6) == 0;
  }
  echo = echo || env; // final echo value

  while(1) {
    printf("@> ");
    int numtok = 0;                                         // initialize number of tokens
    char * input = malloc(sizeof(char) * MAX_LINE);         // declare and allocate input buffer
    strcpy(input, "");                                      // initialize input buffer in case of no input
    char * tokens[ARG_MAX];                                 // declare token storage
    fgets(input, MAX_LINE, stdin);                          // fill input buffer from stdin
    if(echo){                                               // print input buffer if echo is true
      printf("%s", input);
    }
    parse_into_tokens(input, tokens, &numtok);              // split input buffer into tokens

    if(strncmp(input, "", 0) == 0) {                        // check for end of input, not just space
      printf("\nEnd of input\n");
      free(input);
      break;
    }
    else if(numtok == 0) { /* do nothing */ }               // just space, do not end loop
    else if(strncmp(tokens[0], "help", 4) == 0) {           // help command, print help message
      printf("COMMANDO COMMANDS\n");
      printf("help               : show this message\n");
      printf("exit               : exit the program\n");
      printf("list               : list all jobs that have been started giving information on each\n");
      printf("pause nanos secs   : pause for the given number of nanseconds and seconds\n");
      printf("output-for int     : print the output for given job number\n");
      printf("output-all         : print output for all jobs\n");
      printf("wait-for int       : wait until the given job number finishes\n");
      printf("wait-all           : wait for all jobs to finish\n");
      printf("command arg1 ...   : non-built-in is run as a job\n");
    }
    else if(strncmp(tokens[0], "exit", 4) == 0) {          // exit command, exit from loop
      free(input);
      break;
    }
    else if(strncmp(tokens[0], "list", 4) == 0) {          // list command, makes use of cmdcol_print
      cmdcol_print(cmdcol);
    }
    else if(strncmp(tokens[0], "pause", 5) == 0) {         // pause command, makes use of atoi() and pause_for()
      int nano = atoi(tokens[1]), secs = atoi(tokens[2]);
      pause_for(nano, secs);
    }
    else if(strncmp(tokens[0], "output-for", 10) == 0) {   // output-for int command, makes use of cmd_fetch_output
      int proc = atoi(tokens[1]);
      printf("@<<< Output for %s[#%d] (%d bytes):\n", cmdcol->cmd[proc]->name,
                                                      cmdcol->cmd[proc]->pid,
                                                      cmdcol->cmd[proc]->output_size);
      printf("----------------------------------------\n");
      cmd_print_output(cmdcol->cmd[proc]);
      printf("----------------------------------------\n");
    }
    else if(strncmp(tokens[0], "output-all", 10) == 0) {  // output-all command, makes use of cmd_fetch_output
      for(int i = 0; i<cmdcol->size; i++){
        printf("@<<< Output for %s[#%d] (%d bytes):\n", cmdcol->cmd[i]->name,
                                                        cmdcol->cmd[i]->pid,
                                                        cmdcol->cmd[i]->output_size);
        printf("----------------------------------------\n");
        cmd_print_output(cmdcol->cmd[i]);
        printf("----------------------------------------\n");
      }
    }
    else if(strncmp(tokens[0], "wait-for", 8) == 0) {     // wait-for int command, makes use of cmd_update_state
      int proc = atoi(tokens[1]);                         // with blocking behavior to finish int process
      cmd_update_state(cmdcol->cmd[proc], DOBLOCK);
    }
    else if(strncmp(tokens[0], "wait-all", 8) == 0) {     // wait-all command, makes use of cmd_update_state
      for(int i = 0; i < cmdcol->size; i++) {             // with blocking behavior on each cmd_t in cmdcol
        cmd_update_state(cmdcol->cmd[i], DOBLOCK);
      }
    }
    // A. check 0th token for "!"
    else if(strncmp(tokens[0], "!", 1) == 0){

      // B. assign pointer to remaining tokens excluding !

      char **rem_toks = tokens[1];

      // C. create a new command using remaining tokens

      cmd_t *curry = cmd_new(tokens);

      // D. add command to collection
      cmdcol_add(cmdol, curry);

      // E. print message 'Reporting command...'
      printf("Reporting command %s immeadlity\n", cmdcol->cmd[0]->name);

      // F. start command running 
      pid child_pid = fork();
      if(child_pid == 0){
          cmd_start(cmdol->cmd[0]);
          break;
      }
      

      // G. wait for command to finish
      wait(NULL);

      // H. print output for command
      cmd_print_output(cmdcol->cmd[0]);

    else {                                                // other process command, starts with cmd_start and
      cmd_t * cur = cmd_new(tokens);                      // adds to cmdcol using cmdcol_add
      cmdcol_add(cmdcol, cur);
      cmd_start(cmdcol->cmd[cmdcol->size-1]);
      free(input);
      continue;
    }
    free(input);                                          // free allocated input buffer
    cmdcol_update_state(cmdcol, NOBLOCK);                 // update state of all cmd in cmdcol, without blocking
  }                                                       // blocking behavior to allow commando to continue
  cmdcol_freeall(cmdcol);                                 // free memory allocated for each cmd_t
  free(cmdcol);                                           // free memory allocated for cmdcol_t
}

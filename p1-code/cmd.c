#include "commando.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

cmd_t *cmd_new(char *argv[]) { 
    cmd_t *cmd = malloc(sizeof(cmd_t));
    strcpy(cmd->name, argv[0]);
    char* s = "INIT";
    snprintf(cmd->str_status, 5, "%s\n", s);
    bool arr_done = 0;
    for (int i = 0; i < ARG_MAX; i++) {
      if (arr_done) {
        cmd->argv[i] = NULL; // argv must first have values initialized to null
      }
      else if (argv[i] == NULL) {
        cmd->argv[i] = NULL;
        arr_done = 1;
      } else {
      cmd->argv[i] = strdup(argv[i]);
      }
    }
    cmd->finished = 0;
    cmd->pid = -1;
    cmd->out_pipe[0] = -1; cmd->out_pipe[1] = -1;
    cmd->status = -1;
    cmd->output = NULL;
    cmd->output_size = -1;
    return cmd;
}

void cmd_free(cmd_t *cmd) { 
  for(int i = 0; i < ARG_MAX; i++) {
    if (cmd->argv == NULL) { //don't free anything that wasn't initialized
      break;
    }
    free(cmd->argv[i]);
  }
    if(cmd->output != NULL){ 
        free(cmd->output); 
    }
    free(cmd);

}

void cmd_start(cmd_t *cmd) { 
  snprintf(cmd->str_status, STATUS_LEN, "RUN");
  if (pipe(cmd->out_pipe) < 0) {  // open and check a pipe in cmd->out_pipe
    exit(1);
  }
  pid_t pid = fork();
  // child process
  if (pid == 0) {
    dup2(cmd->out_pipe[1], STDOUT_FILENO);    // redirect output to pipe
    close(cmd->out_pipe[0]);                  // close write end of pipe
    execvp(cmd->name, cmd->argv); // exec cmd args
  } else { // parent process
    cmd->pid = pid;
    close(cmd->out_pipe[1]);
  }
}

void cmd_update_state(cmd_t *cmd, int block) { 
  if(cmd->finished == 1) {
    return;
  } else {
    int status = -1;
    int ret = waitpid(cmd->pid, &status, block); //capture return value to catch errors
    if (ret == -1) {
      perror("waitpid() failed\n");
      exit(1);
    }
    else if(ret == 0) {
      
    }
    else if (ret == cmd->pid) {
      if (WIFEXITED(status)) {
        int exitstat = WEXITSTATUS(status);
        cmd->finished = 1;
        cmd->status = exitstat; //satus comes from the WEXISTATUS macro
        sprintf(cmd->str_status, "EXIT(%d)", exitstat);
        printf("@!!! %s[#%d]: %s\n", cmd->name, cmd->pid, cmd->str_status);
        cmd_fetch_output(cmd);
      }
    }
  }
}

char *read_all(int fd, int *nread) { 
  // Strong influence from HW3's append_all.c

  // Allocate space for buffer
  int cur_pos = 0;
  int max_size = 4;
  char *buffer = NULL;
  buffer = realloc(buffer, max_size*sizeof(char));

  // read from file in buffer, reallocating if needed
  while(1) {
    if(cur_pos >= max_size) {
      max_size = max_size * 2;
      buffer = realloc(buffer, max_size);
      if(buffer == NULL){
        free(buffer);
        exit(1);
      }
    }

    int max_read = max_size - cur_pos;
    *nread = read(fd, buffer+cur_pos, max_read);

    if(*nread == 0) {
      break;
    } else if (*nread == -1) {
      exit(1);
    }
    cur_pos += *nread;
  }

  // set nread to number of bytes read
  *nread = cur_pos;

  // null terminate string and return
  buffer[*nread] = '\0';
  return(buffer);
}

void cmd_fetch_output(cmd_t *cmd) { 
  // Check that cmd is finished
  if(cmd->finished == 0) {
    printf("%s[#%d] not finished yet\n", cmd->name, cmd->pid);
  } else {
    int size;
    //Stores output from the outpipe into a buffer so it can be put into output.
    char *buffer = read_all(cmd->out_pipe[0], &size); 
    cmd->output_size = size;
    cmd->output = buffer;
    //Close pipe.
    close(cmd->out_pipe[0]);
  }
}

void cmd_print_output(cmd_t *cmd) { 
  // if output is NULL. The message includes the command name and PID.
  if (cmd->output == NULL) {
    printf("%s[#%d] : output not ready\n",cmd->name, cmd->pid);
  } else {
    // otherwise write output to the output fields
    write(STDOUT_FILENO, cmd->output, cmd->output_size);
  }
}

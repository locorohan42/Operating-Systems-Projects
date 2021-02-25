#include "commando.h"
#include <errno.h>
#include <stdbool.h>


// cmdcol.c: functions related to cmdcol_t collections of commands.
cmdcol_t * cmdcol_new() { // allocates memory for a new cmdcol_t object, free'd in main
  cmdcol_t * col = malloc(sizeof(cmdcol_t));
  col->size = 0;
  return col;
}

void cmdcol_add(cmdcol_t *col, cmd_t *cmd) { // add cmd pointer to col at the current next avaliable position
  if(col->size == 1024) { // check if col is full
    perror("too many commands");
    exit(1);
  }
  else { // if not full append cmd to col
    col->cmd[col->size] = cmd;
    col->size++;
  }
}

void cmdcol_print(cmdcol_t *col) {
  printf("JOB  #PID      STAT   STR_STAT OUTB COMMAND\n");
  for(int i = 0; i < col->size; i++) {
    cmd_t *cur = col->cmd[i];
    // using format specifiers for printing
    printf("%-5d#%-9d%4d%11s%5d ", i, cur->pid, cur->status, cur->str_status, cur->output_size);
    // print all of argv[] for each command without printing null
    bool arr_done = 0;
    for(int j = 0; j < 256; j++) {
      if (arr_done) { /* do nothing */ }
      else if (cur->argv[j] == NULL) {
        arr_done = 1;
      }
      else {
        printf("%s ", cur->argv[j]);
      }
    }
    printf("\n");
  }

}

void cmdcol_update_state(cmdcol_t *col, int block) {
  // update each cmd_t in col with block blocking behavior
  for(int i = 0; i < col->size; i++) {
    cmd_update_state(col->cmd[i], block);
  }
}

void cmdcol_freeall(cmdcol_t *col) {
  // Call cmd_free() on all of the constituent cmd_t's.
  for(int i = 0; i < col->size; i++) {
    cmd_free(col->cmd[i]);
  }
}

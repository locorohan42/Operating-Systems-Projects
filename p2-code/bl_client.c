#include "blather.h"

simpio_t simpio_actual = {};
simpio_t *simpio = &simpio_actual;

client_t client_actual = {};
client_t *client = &client_actual;

pthread_t client_thread;          // thread managing user input
pthread_t server_thread;

join_t join_actual = {};
join_t *join = &join_actual;
int to_server_fd, to_client_fd;

sem_t *sem;

// Client thread to manage user input
void *client_worker(void *arg){
  while(!(simpio->end_of_input)){
    simpio_reset(simpio);
    iprintf(simpio, "");
    while(!(simpio->line_ready) && !(simpio->end_of_input)){
      simpio_get_char(simpio);
    }
    if(simpio->line_ready){
      // iprintf(simpio, "You entered: %s\n",simpio->buf);
      mesg_t mesg_act = {};
      mesg_t *mesg = &mesg_act;
      mesg->kind = BL_MESG;
      strncpy(mesg->name, join->name, MAXNAME);
      strncpy(mesg->body, simpio->buf, MAXLINE);
      dbg_printf("[%s]", mesg->body);
      // iprintf(simpio, "[%s]", mesg->body);
      sem_wait(sem);
      write(to_server_fd, mesg, sizeof(mesg_t));
      sem_post(sem);
    }
  }
  iprintf(simpio, "End of Input, Departing\n");
  mesg_t dep_act = {};
  mesg_t *dep = &dep_act;
  dep->kind = BL_DEPARTED;
  sem_wait(sem);
  int err = write(to_server_fd, dep, sizeof(mesg_t));
  sem_post(sem);
  if(err != sizeof(mesg_t)) {
    iprintf(simpio, "Write in client thread had error, %d\n", err);
  }
  pthread_cancel(server_thread); // kill the background thread
  return NULL;
}

// Server thread to listen to the info from the server.
void *server_worker(void *arg){
  while(1) {
    mesg_t cli_act = {};
    mesg_t *client = &cli_act;
    int err = read(to_client_fd, client, sizeof(mesg_t));
    if(err != sizeof(mesg_t)) {
      iprintf(simpio, "Read in server thread had error, %d\n", err);
    }
    if(client->kind == BL_MESG) {
      iprintf(simpio, "[%s] : %s\n", client->name, client->body);
    } else if(client->kind == BL_JOINED) {
      iprintf(simpio, "-- %s JOINED --\n", client->name);
    } else if(client->kind == BL_DEPARTED) {
      iprintf(simpio, "-- %s DEPARTED --\n", client->name);
    } else if(client->kind == BL_SHUTDOWN) {
      iprintf(simpio, "!!! server is shutting down !!!\n");
      break;
    }
  }
  pthread_cancel(client_thread);
  return NULL;
}

int main(int argc, char *argv[]){
  if(argc < 3) {
    printf("usage is %s <server_name> <user_name>\n", argv[0]);
    exit(1);
  }

  char *sem_name = "/the_semaphore";   // global name for semaphore

  sem = sem_open(sem_name, O_CREAT, S_IRUSR | S_IWUSR);
  // Note file-like semantics with sem_open(): name, flags, permissions;

  // printf("initializing\n");       // before it can be used
  sem_init(sem, 1, 1);

  char prompt[MAXNAME];
  snprintf(prompt, MAXNAME, "%s>> ",argv[2]); // create a prompt string
  simpio_set_prompt(simpio, prompt);         // set the prompt
  simpio_reset(simpio);                      // initialize io
  simpio_noncanonical_terminal_mode();       // set the terminal into a compatible mode

  char *server_name = argv[1];
  char *user_name = argv[2];

  snprintf(join->to_client_fname, MAXPATH, "to-%s%d.fifo", user_name, getpid());
  snprintf(join->to_server_fname, MAXPATH, "to-%s%d.fifo", server_name, getpid());
  snprintf(join->name, MAXNAME, "%s", user_name);

  char *fifo_name = strcat(server_name, ".fifo");

  int server_fd = open(fifo_name, O_RDWR);
  write(server_fd, join, sizeof(join_t));

  mkfifo(join->to_client_fname, DEFAULT_PERMS);
  mkfifo(join->to_server_fname, DEFAULT_PERMS);

  to_client_fd = open(join->to_client_fname, O_RDWR);
  to_server_fd = open(join->to_server_fname, O_RDWR);

  pthread_create(&client_thread, NULL, client_worker, NULL);     // start user thread to read input
  pthread_create(&server_thread, NULL, server_worker, NULL);
  pthread_join(client_thread, NULL);
  pthread_join(server_thread, NULL);

  simpio_reset_terminal_mode();
  printf("\n");                 // newline just to make returning to the terminal prettier
  sem_close(sem);
  return 0;
}
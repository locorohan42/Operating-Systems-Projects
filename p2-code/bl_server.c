#include "blather.h"

server_t server_actual;
server_t *server = &server_actual;

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("usage is %s <server_name> \n", argv[0]);
        exit(1);
    }

    server_start(server, argv[1], O_RDWR);   // start the server


    int KEEP_GOING = 1;                   // control variable to continue loop
    void shut_down(){
        KEEP_GOING = 0;
    }

    struct sigaction my_sa = {};               // portable signal handling setup with sigaction()
    sigemptyset(&my_sa.sa_mask);               // don't block any other signals during handling
    my_sa.sa_flags = SA_RESTART;               // always restart system calls on signals possible

    my_sa.sa_handler = shut_down;              // run function shut_down
    sigaction(SIGTERM, &my_sa, NULL);          // register SIGTERM with given action

    my_sa.sa_handler = shut_down;              // run function shut_down
    sigaction(SIGINT,  &my_sa, NULL);          // register SIGINT with given action

    while(KEEP_GOING){
        server_check_sources(server);
        server_handle_join(server);
        for(int i = 0;i < server->n_clients; i++){
            server_handle_client(server, i);
        }
    }

    server_shutdown(server);                   // code that gets executed after signal handler
    return 0;

}
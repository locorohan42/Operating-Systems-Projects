#include "blather.h"

// Gets a pointer to the client_t struct at the given index. If the
// index is beyond n_clients, the behavior of the function is
// unspecified and may cause a program crash.
client_t *server_get_client(server_t *server, int idx){
    // client_t *client;

    if(idx < server->n_clients){
        return &server->client[idx];
    } else {
        dbg_printf("Client is not in the array\n");
        mesg_t mesg_act;
        mesg_t *mesg = &mesg_act;
        mesg->kind = BL_MESG;
        strncpy(mesg->name, "get client error", MAXNAME);
        strncpy(mesg->body, "Client cannot be accessed", MAXLINE);
        server_broadcast(server, mesg);                      //Error handling so program doesn't crash
        exit(0);
    }
}


// Initializes and starts the server with the given name. A join fifo
// called "server_name.fifo" should be created. Removes any existing
// file of that name prior to creation. Opens the FIFO and stores its
// file descriptor in join_fd.
//
// ADVANCED: create the log file "server_name.log" and write the
// initial empty who_t contents to its beginning. Ensure that the
// log_fd is position for appending to the end of the file. Create the
// POSIX semaphore "/server_name.sem" and initialize it to 1 to
// control access to the who_t portion of the log.
void server_start(server_t *server, char *server_name, int perms){
    log_printf("BEGIN: server_start()\n");
    char *fifo_name = strcat(server_name, ".fifo");

    remove(fifo_name);

    mkfifo(fifo_name, DEFAULT_PERMS);

    int join_fd = open(fifo_name, perms); //perms given as an int
    server->join_fd = join_fd;

    strncpy(server->server_name, server_name, MAXNAME);
    log_printf("END: server_start()\n");
}

// Shut down the server. Close the join FIFO and unlink (remove) it so
// that no further clients can join. Send a BL_SHUTDOWN message to all
// clients and proceed to remove all clients in any order.
//
// ADVANCED: Close the log file. Close the log semaphore and unlink
// it.
void server_shutdown(server_t *server){
    log_printf("BEGIN: server_shutdown()\n");
    mesg_t mesg_act = {};
    mesg_t *message = &mesg_act;
    message->kind = 40;
    strncpy(message->name, server->server_name, MAXNAME);
    strncpy(message->body, "BL_SHUTDOWN", MAXLINE);                           // create message

    // write(server->join_fd, message.body, strlen(message.body));
    server_broadcast(server, message);                              // let all clients know of shutdown

    close(server->join_fd);
    char *fifo_name = strcat(server->server_name, ".fifo");
    remove(fifo_name);                                              // close and remove join FIFO

    for(int i = 0;i < server->n_clients; i++){
        server_remove_client(server, i);                            // remove all clients
    }
    // free(server->client);

    log_printf("END: server_shutdown()\n");
}

// Adds a client to the server according to the parameter join which
// should have fileds such as name filed in.  The client data is
// copied into the client[] array and file descriptors are opened for
// its to-server and to-client FIFOs. Initializes the data_ready field
// for the client to 0. Returns 0 on success and non-zero if the
// server as no space for clients (n_clients == MAXCLIENTS).
int server_add_client(server_t *server, join_t *join){
    // information about client is in the join_t
    // use that information to store the client in the servers client arr at nclients and then inc nclients
    log_printf("BEGIN: server_add_client()\n");

    if(server->n_clients < MAXCLIENTS){    // check if there is space for the new client to be added

    client_t client;
    strncpy(client.name, join->name, MAXNAME);
    strncpy(client.to_client_fname, join->to_client_fname, MAXNAME);
    strncpy(client.to_server_fname, join->to_server_fname, MAXNAME);
    client.data_ready = 0;                // extract info about client from join_t into a client struct and initilizae data_ready to 0

    int to_client_fd = open(client.to_client_fname, O_RDWR);  // Open file descriptor for the to-server and to-client FIFOs
    int to_server_fd = open(client.to_server_fname, O_RDWR);

    client.to_client_fd = to_client_fd;
    client.to_server_fd = to_server_fd;

    server->client[server->n_clients] = client;              // put client into the servers array of clients and update nclients
    server->n_clients = server->n_clients+1;
    log_printf("END: server_add_client()\n");
    return 0;                                                //client has been succesfully added so return 0
    }
    else {
        dbg_printf("Clients array is already full.");
        return 1;                                            //not enough space
    }

}

// Remove the given client likely due to its having departed or
// disconnected. Close fifos associated with the client and remove
// them.  Shift the remaining clients to lower indices of the client[]
// preserving their order in the array; decreases n_clients. Returns 0
// on success, 1 on failure.
int server_remove_client(server_t *server, int idx){
    if(idx < server->n_clients){

    client_t *client = server_get_client(server, idx);        // Get the client as well as its FIFO fds
    int to_client_fd = client->to_client_fd;
    int to_server_fd = client->to_server_fd;

    char to_client_fname[MAXPATH];
    char to_server_fname[MAXPATH];
    strncpy(to_client_fname, client->to_client_fname, MAXNAME);         // get client FIFO names
    strncpy(to_server_fname, client->to_server_fname, MAXNAME);

    int err = close(to_client_fd); // Close and remove the client's fifos
    if(err != 0) {
    //   log_printf("Close of client_fd in server_remove_client had an error!\n");
      dbg_printf("Close of client_fd in server_remove_client had an error!\n");
    }
    err = close(to_server_fd);
    if(err != 0) {
    //   log_printf("Close of server_fd in server_remove_client had an error!\n");
      dbg_printf("Close of client_fd in server_remove_client had an error!\n");
    }
    remove(to_client_fname);
    remove(to_server_fname);

    for(int i = idx; i < server->n_clients - 1; i++){
        server->client[i] = server->client[i + 1];           // Delete the client and reduce n_clients in the server
    }
    server->n_clients = server->n_clients-1;
    return 0;                                                //return 0 on sucess, 1 on failure

    } else {
        dbg_printf("Client idx out of bounds!\n");
        return 1;
    }
}

// Send the given message to all clients connected to the server by
// writing it to the file descriptors associated with them.
//
// ADVANCED: Log the broadcast message unless it is a PING which
// should not be written to the log.
void server_broadcast(server_t *server, mesg_t *mesg){
    for(int i = 0; i < server->n_clients; i++){
        // log_printf("mesg: %d\n", mesg->kind);
        client_t client = server->client[i];              // for all clients
        // log_printf("Client name: %s, fd: %d\n", client.name, client.to_client_fd);
        int to_client_fd = client.to_client_fd;           // get the client FIFO fd
        int err = write(to_client_fd, mesg, sizeof(mesg_t));        //kind write mesg to the FIFO
        if(err != sizeof(mesg_t)) {
        //   log_printf("Write in server_broadcast had an error, %d!\n", err);
          dbg_printf("Write in server_broadcast had an error, %d!\n", err);
        }
    }

}

// Checks all sources of data for the server to determine if any are
// ready for reading. Sets the servers join_ready flag and the
// data_ready flags of each of client if data is ready for them.
// Makes use of the poll() system call to efficiently determine which
// sources are ready.
//
// NOTE: the poll() system call will return -1 if it is interrupted by
// the process receiving a signal. This is expected to initiate server
// shutdown and is handled by returning immediagely from this function.
void server_check_sources(server_t *server){
    log_printf("BEGIN: server_check_sources()\n");

    int numberOfClients = server->n_clients + 1;
    log_printf("poll()'ing to check %d input sources\n",numberOfClients);
    struct pollfd pfds[numberOfClients];                               // array of structures for poll, 1 per fd to be monitored
    for(int i = 0; i < numberOfClients; i++) { pfds[i].fd = 0; pfds[i].events = 0; pfds[i].revents = 0;}
    pfds[0].fd = server->join_fd;
    pfds[0].events = POLLIN;
    for(int i = 1;i < numberOfClients; i++){
        client_t client = {};                                            //struct or pointer to struct?
        client = server->client[i - 1];
        pfds[i].fd = client.to_server_fd;
        pfds[i].events = POLLIN;
        // mesg_t mesg = {};
        // read(client.to_server_fd, &mesg, sizeof(mesg));
        // log_printf("Client mesg type: %d", mesg.kind);
    }
    int ret = poll(pfds, numberOfClients, -1);
    if(ret < 0){
        log_printf("poll() completed with return value %d\n", ret);
        log_printf("poll() interrupted by a signal\n");            //poll interuppted by signal
        log_printf("END: server_check_sources()\n");
        server_shutdown(server);                                   //shutdown the server and exit
        exit(0);
    }
    log_printf("poll() completed with return value %d\n", ret);
    if(pfds[0].revents & POLLIN){ // join clause           //check if one of the clients is ready to for reading
        server->join_ready = 1;
      }
    for(int i = 1; i < numberOfClients; i++){
        if (pfds[i].revents & POLLIN) { // message clause
            server->client[i-1].data_ready = 1;                      //set client flag
        }
    }
    log_printf("join_ready = %d\n",server->join_ready);
    for(int i = 0; i < numberOfClients - 1; i++) {
      log_printf("client %d '%s' data_ready = %d\n", i, server->client[i].name, server->client[i].data_ready);
    }
    log_printf("END: server_check_sources()\n");
}


// Return the join_ready flag from the server which indicates whether
// a call to server_handle_join() is safe.
int server_join_ready(server_t *server){
    return server->join_ready;
}

// Call thisserver_check_sources function only if server_join_ready() returns true. Read a
// join request and add the new client to the server. After finishing,
// set the servers join_ready flag to 0.
void server_handle_join(server_t *server){

    if(server_join_ready(server) == 1){
        log_printf("BEGIN: server_handle_join()\n");
        join_t join_act;
        join_t *join = &join_act;
        int nread = read(server->join_fd, join, sizeof(join_t));  // get info about client from join_t struct
        log_printf("join request for new client '%s'\n",join->name);
        server_add_client(server, join);                           // add client into server's client array

        if(nread != sizeof(join_t)){                               // in case bad data read
            log_printf("SERVER: read %d bytes from join.fifo; empty pipe, exiting\n",nread);
        }
        client_t *client = server_get_client(server, server->n_clients-1);

        mesg_t mesg_act = {};
        mesg_t *message = &mesg_act;

        message->kind = BL_JOINED;
        strncpy(message->name, client->name, MAXNAME);
        strncpy(message->body, "BL_JOINED", MAXLINE);                           // create message
        server_broadcast(server, message);

        char name[MAXPATH];
        strncpy(name, client->name, MAXNAME);                               // get the client name
        log_printf("END: server_handle_join()\n");
    } else {
        //do nothing
        // log_printf("flag not set\n");
    }
    server->join_ready = 0;                                         // set join_ready flag to 0

}

// Return the data_ready field of the given client which indicates
// whether the client has data ready to be read from it.
int server_client_ready(server_t *server, int idx){
    client_t *client = server_get_client(server, idx);
    return client->data_ready;
}

// Process a message from the specified client. This function should
// only be called if server_client_ready() returns true. Read a
// message from to_server_fd and analyze the message kind. Departure
// and Message types should be broadcast to all other clients.  Ping
// responses should only change the last_contact_time below. Behavior
// for other message types is not specified. Clear the client's
// data_ready flag so it has value 0.
//
// ADVANCED: Update the last_contact_time of the client to the current
// server time_sec.
//
// LOG Messages:
// log_printf("BEGIN: server_handle_client()\n");           // at beginning of function
// log_printf("client %d '%s' DEPARTED\n",                  // indicates client departed
// log_printf("client %d '%s' MESSAGE '%s'\n",              // indicates client message
// log_printf("END: server_handle_client()\n");             // at end of function
void server_handle_client(server_t *server, int idx){


    if(server_client_ready(server, idx) == 1){
        log_printf("BEGIN: server_handle_client()\n");
        mesg_t mesg_act= {};
        mesg_t *mesg = &mesg_act;
        client_t *client = server_get_client(server, idx);
        // int to_server_fd =
        read(client->to_server_fd, mesg, sizeof(mesg_t));  // put message into mesg_t struct
        // log_printf("mesg->kind = %d\n", mesg->kind);
        if(mesg->kind == BL_MESG){           // mesg or departed type
            dbg_printf("[%s]", mesg->body);
            log_printf("client %d '%s' MESSAGE '%s'\n", idx, client->name, mesg->body);
            // log_printf("'%s'", mesg->body);
            server_broadcast(server, mesg);                 // send message to all
            client->data_ready = 0;                         // set flag
            // log_printf("client %d '%s' MESSAGE '%s'\n", idx, client->name, mesg->body);
        } else if(mesg->kind == BL_DEPARTED) {
            client->data_ready = 0;                         // set flag
            log_printf("client %d '%s' DEPARTED\n", idx, client->name);
            // char departing_client[MAXNAME];
            strncpy(mesg->name, client->name, MAXNAME);
            server_remove_client(server, idx);
            server_broadcast(server, mesg);                 // send message to all
        } else {
            //advanced features
        }
        log_printf("END: server_handle_client()\n");
    } else {
        //do nothing
    }
}


void server_tick(server_t *server);
// ADVANCED: Increment the time for the server

void server_ping_clients(server_t *server);
// ADVANCED: Ping all clients in the server by broadcasting a ping.

void server_remove_disconnected(server_t *server, int disconnect_secs);
// ADVANCED: Check all clients to see if they have contacted the
// server recently. Any client with a last_contact_time field equal to
// or greater than the parameter disconnect_secs should be
// removed. Broadcast that the client was disconnected to remaining
// clients.  Process clients from lowest to highest and take care of
// loop indexing as clients may be removed during the loop
// necessitating index adjustments.

void server_write_who(server_t *server);
// ADVANCED: Write the current set of clients logged into the server
// to the BEGINNING the log_fd. Ensure that the write is protected by
// locking the semaphore associated with the log file. Since it may
// take some time to complete this operation (acquire semaphore then
// write) it should likely be done in its own thread to preven the
// main server operations from stalling.  For threaded I/O, consider
// using the pwrite() function to write to a specific location in an
// open file descriptor which will not alter the position of log_fd so
// that appends continue to write to the end of the file.

void server_log_message(server_t *server, mesg_t *mesg);
// ADVANCED: Write the given message to the end of log file associated
// with the server.
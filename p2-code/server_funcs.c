#include "blather.h"

int main(int argc, char *argv[]) {
}

// Gets a pointer to the client_t struct at the given index. If the
// index is beyond n_clients, the behavior of the function is
// unspecified and may cause a program crash.
client_t *server_get_client(server_t *server, int idx){   
    client_t *client;
    if(idx < server->n_clients){
        client_t client = server->client[idx];
    } else {       
        strcpy(client->name, "client out of bounds");
    }
    
    return client;
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
// 
// LOG Messages:
// log_printf("BEGIN: server_start()\n");              // at beginning of function
// log_printf("END: server_start()\n");                // at end of function
void server_start(server_t *server, char *server_name, int perms){
    // log_printf("BEGIN: server_start()\n");
    char *fifo_name = strcat(server_name, ".fifo");

    remove(fifo_name); 
    mkfifo(fifo_name, S_IRUSR | S_IWUSR);

    int join_fd = open(fifo_name, perms); //perms given as an int
    server->join_fd = join_fd;
    // log_printf("END: server_start()\n");  
}

// Shut down the server. Close the join FIFO and unlink (remove) it so
// that no further clients can join. Send a BL_SHUTDOWN message to all
// clients and proceed to remove all clients in any order.
// 
// ADVANCED: Close the log file. Close the log semaphore and unlink
// it.
//
// LOG Messages:
// log_printf("BEGIN: server_shutdown()\n");           // at beginning of function
// log_printf("END: server_shutdown()\n");             // at end of function
void server_shutdown(server_t *server){
    // log_printf("BEGIN: server_shutdown()\n"); 

    mesg_t message; 
    message.kind = 40;
    strcpy( message.name, server->server_name);
    // message.name = server->server_name;
    // message.body = "BL_SHUTDOWN";
    strcpy(message.body, "BL_SHUTDOWN");

    write(server->join_fd, message.body, strlen(message.body)); 
    close(server->join_fd);
    free(server->client);

    // log_printf("END: server_shutdown()\n"); 
}

// Adds a client to the server according to the parameter join which
// should have fileds such as name filed in.  The client data is
// copied into the client[] array and file descriptors are opened for
// its to-server and to-client FIFOs. Initializes the data_ready field
// for the client to 0. Returns 0 on success and non-zero if the
// server as no space for clients (n_clients == MAXCLIENTS).
//
// LOG Messages:
// log_printf("BEGIN: server_add_client()\n");         // at beginning of function
// log_printf("END: server_add_client()\n");           // at end of function
int server_add_client(server_t *server, join_t *join){
    // information about client is in the join_t
    // use that information to store the client in the servers client arr at nclients and then inc nclients
    // log_printf("BEGIN: server_add_client()\n"); 

    // check if there is space for the new clien to be added
    if(server->n_clients < MAXCLIENTS){    
    // extract info about client from join_t into a client struct and initilizae data_ready to 0
    client_t client;
    strcpy(client.name, join->name);
    strcpy(client.to_client_fname, join->to_client_fname);
    strcpy(client.to_server_fname, join->to_server_fname);
    client.data_ready = 0;

    // Open file descriptor for the to-server and to-client FIFOs
    int to_client_fd = open(client.to_client_fname, O_RDWR);
    int to_server_fd = open(client.to_server_fname, O_RDWR);
    

    // put client into the servers array of clients and update nclients
    server->client[server->n_clients] = client;
    server->n_clients = server->n_clients+1;

    return 0; //client has been succesfully added so return 0
    } else {
        //not enough space
        return 1;
    }
    // log_printf("END: server_add_client()\n"); 
}

// Remove the given client likely due to its having departed or
// disconnected. Close fifos associated with the client and remove
// them.  Shift the remaining clients to lower indices of the client[]
// preserving their order in the array; decreases n_clients. Returns 0
// on success, 1 on failure.
int server_remove_client(server_t *server, int idx){
    // Get the client as well as its fifos and fds
    client_t *client = server_get_client(server, idx);
    int to_client_fd = client->to_client_fd;  
    int to_server_fd = client->to_server_fd;    

    char to_client_fname[MAXPATH]; 
    char to_server_fname[MAXPATH]; 
    strcpy(to_client_fname, client->to_client_fname);
    strcpy(to_server_fname, client->to_server_fname);

    // Close and remove the client's fifos
    close(to_client_fd);
    close(to_server_fd);
    remove(to_client_fname);       
    remove(to_server_fname); 

    // Delete the client and reduce n_clients in the server
    for(int i = idx; i < server->n_clients - 1; i++){
        server->client[i] = server->client[i + 1];
    } 
    server->n_clients = server->n_clients-1;
    return 0; //return 0 on sucess, 1 on failure
}

// Send the given message to all clients connected to the server by
// writing it to the file descriptors associated with them.
//
// ADVANCED: Log the broadcast message unless it is a PING which
// should not be written to the log.
void server_broadcast(server_t *server, mesg_t *mesg){
    // For all clients
    for(int i = 0; i < server->n_clients; i++){
        // get the client FIFO fd 
        // int to_client_fd = open(request.client_fifo, O_WRONLY);
        client_t client = server->client[i];
        int to_client_fd = client.to_client_fd;

        // get the body of the mesg and write it to the FIFO
        char body[MAXLINE];  
        strcpy(body, mesg->body);
        write(to_client_fd, body, strlen(body));
    }
    
}


void server_check_sources(server_t *server);
// Checks all sources of data for the server to determine if any are
// ready for reading. Sets the servers join_ready flag and the
// data_ready flags of each of client if data is ready for them.
// Makes use of the poll() system call to efficiently determine which
// sources are ready.
// 
// NOTE: the poll() system call will return -1 if it is interrupted by
// the process receiving a signal. This is expected to initiate server
// shutdown and is handled by returning immediagely from this function.
// 
// LOG Messages:
// log_printf("BEGIN: server_check_sources()\n");             // at beginning of function
// log_printf("poll()'ing to check %d input sources\n",...);  // prior to poll() call
// log_printf("poll() completed with return value %d\n",...); // after poll() call
// log_printf("poll() interrupted by a signal\n");            // if poll interrupted by a signal
// log_printf("join_ready = %d\n",...);                       // whether join queue has data
// log_printf("client %d '%s' data_ready = %d\n",...)         // whether client has data ready
// log_printf("END: server_check_sources()\n");               // at end of function

int server_join_ready(server_t *server);
// Return the join_ready flag from the server which indicates whether
// a call to server_handle_join() is safe.

void server_handle_join(server_t *server);
// Call this function only if server_join_ready() returns true. Read a
// join request and add the new client to the server. After finishing,
// set the servers join_ready flag to 0.
//
// LOG Messages:
// log_printf("BEGIN: server_handle_join()\n");               // at beginnning of function
// log_printf("join request for new client '%s'\n",...);      // reports name of new client
// log_printf("END: server_handle_join()\n");                 // at end of function

int server_client_ready(server_t *server, int idx);
// Return the data_ready field of the given client which indicates
// whether the client has data ready to be read from it.

void server_handle_client(server_t *server, int idx);
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


#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BACKLOG 10

typedef struct client_t {
    int new_socket_fd;
    char client_name[100];
} client_t;

typedef struct pthread_arg_t {
    struct client_t client;
    struct sockaddr_in client_address;
} pthread_arg_t;

client_t client_list[100];
    int client_count = 0; 

/* Thread routine to serve connection to client. */
void *pthread_routine(void *arg);

/* Signal handler to handle SIGTERM and SIGINT signals. */
void signal_handler(int signal_number);

int main(int argc, char *argv[]) {
    int port, socket_fd, new_socket_fd;
    struct sockaddr_in address;
    pthread_attr_t pthread_attr;
    pthread_arg_t *pthread_arg;
    pthread_t pthread;
    socklen_t client_address_len;
    
    char buffer[1000];

    /* Get port from command line arguments or stdin. */
    port = argc > 1 ? atoi(argv[1]) : 0;
    if (!port) {
        printf("Enter Port: ");
        scanf("%d", &port);
    }

    /* Initialise IPv4 address. */
    memset(&address, 0, sizeof address);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    /* Create TCP socket. */
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    /* Bind address to socket. */
    if (bind(socket_fd, (struct sockaddr *)&address, sizeof address) == -1) {
        perror("bind");
        exit(1);
    }

    /* Listen on socket. */
    if (listen(socket_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    /* Assign signal handlers to signals. */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    /* Initialise pthread attribute to create detached threads. */
    if (pthread_attr_init(&pthread_attr) != 0) {
        perror("pthread_attr_init");
        exit(1);
    }
    if (pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED) != 0) {
        perror("pthread_attr_setdetachstate");
        exit(1);
    }

    while (1) {
        /* Create pthread argument for each connection to client. */
        /* TODO: malloc'ing before accepting a connection causes only one small
         * memory when the program exits. It can be safely ignored.
         */
        pthread_arg = (pthread_arg_t *)malloc(sizeof *pthread_arg);
        if (!pthread_arg) {
            perror("malloc");
            continue;
        }

        /* Accept connection to client. */
        client_address_len = sizeof pthread_arg->client_address;
        new_socket_fd = accept(socket_fd, (struct sockaddr *)&pthread_arg->client_address, &client_address_len);
        if (new_socket_fd == -1) {
            perror("accept");
            free(pthread_arg);
            continue;
        }
	
	//First message sent by client is always name of the client
	read( new_socket_fd , buffer, 1024);
    	printf("Client connected - %s\n",buffer );

        client_list[client_count].new_socket_fd = new_socket_fd;
        strcpy(client_list[client_count].client_name, buffer);

        

        /* Initialise pthread argument. */
	pthread_arg->client = client_list[client_count];

        client_count++;

        

        /* Create thread to serve connection to client. */
        if (pthread_create(&pthread, &pthread_attr, pthread_routine, (void *)pthread_arg) != 0) {
            perror("pthread_create");
            free(pthread_arg);
            continue;
        }

	memset(buffer, 0, strlen(buffer));
    }

    /* close(socket_fd);
     * TODO: If you really want to close the socket, you would do it in
     * signal_handler(), meaning socket_fd would need to be a global variable.
     */
    return 0;
}

void *pthread_routine(void *arg) {
    pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
    int valread;
    char buffer[1024] = {0};
   
    struct sockaddr_in client_address = pthread_arg->client_address;
    struct client_t client = pthread_arg->client;
    
    printf("Thread created for client %s\n", client.client_name);
    
    free(arg);

    while(valread = read( client.new_socket_fd , buffer, 1024))
    {
    	
	printf("Message received from %s : %s\n", client.client_name, buffer);
        char *client_to_forward = strtok (buffer, "%");
	char *message_to_forward = strtok (NULL, "%");
        int socket_fd_to_forward;

	printf("%s sent a message to forward to %s\n",client.client_name, client_to_forward );
        
        int client_found = 0, i;

	for(i=0; i < client_count; i++)
        {
	    if(strcmp(client_to_forward, client_list[i].client_name) == 0)
            {
		client_found = 1;
		socket_fd_to_forward = client_list[i].new_socket_fd;
	    }
	}
	
	if(client_found)
		send(socket_fd_to_forward , message_to_forward , strlen(message_to_forward) , 0 );
	else
		printf("client %s not found in the connected clients list\n", client_to_forward);
        
        memset(buffer, 0, sizeof(buffer));
        
    }

}

void signal_handler(int signal_number) {
    /* TODO: Put exit cleanup code here. */
    exit(0);
}


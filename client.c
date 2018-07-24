

#include <pthread.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_NAME_LEN_MAX 255

typedef struct pthread_arg_t {
    int new_socket_fd
} pthread_arg_t;

void *pthread_routine(void *arg);

int main(int argc, char *argv[]) {
    char server_name[SERVER_NAME_LEN_MAX + 1] = { 0 };
    char client_name[1000];
    int server_port, socket_fd;
    struct hostent *server_host;
    struct sockaddr_in server_address;

    pthread_attr_t pthread_attr;
    pthread_arg_t *pthread_arg;
    pthread_t pthread;
	
    char message[1000];

    /* Get server name from command line arguments or stdin. */
    if (argc > 1) {
        strncpy(server_name, argv[1], SERVER_NAME_LEN_MAX);
    } else {
        printf("Enter Server Name: ");
        scanf("%s", server_name);
    }

    /* Get server port from command line arguments or stdin. */
    server_port = argc > 2 ? atoi(argv[2]) : 0;
    if (!server_port) {
        printf("Enter Port: ");
        scanf("%d", &server_port);
    }

    /* Get name of the client */
    if (argc > 3) {
        strcpy(client_name, argv[3]);
    } else {
        printf("Enter Client Name: ");
        scanf("%s", client_name);
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

    /* Get server host from server name. */
    server_host = gethostbyname(server_name);

    /* Initialise IPv4 server address with server host. */
    memset(&server_address, 0, sizeof server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    memcpy(&server_address.sin_addr.s_addr, server_host->h_addr, server_host->h_length);

    /* Create TCP socket. */
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    /* Connect to socket with server address. */
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof server_address) == -1) {
		perror("connect");
        exit(1);
	}

    pthread_arg = (pthread_arg_t *)malloc(sizeof *pthread_arg);		
    if (!pthread_arg) {
        perror("malloc");
    }

    /* Initialise pthread argument. */
    pthread_arg->new_socket_fd = socket_fd;

    if (pthread_create(&pthread, &pthread_attr, pthread_routine, (void *)pthread_arg) != 0) {
            perror("pthread_create");
            free(pthread_arg);
    }

    //First message sent by client is always name of client
    send(socket_fd , client_name , strlen(client_name) , 0 );    

    while(1)
    {

	    scanf("%[^\n]%*c", message);
	    send(socket_fd , message , strlen(message) , 0 );
	    memset(message, 0, sizeof(message));

    }

    close(socket_fd);
    return 0;
}


void *pthread_routine(void *arg) {
    pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
    int valread,new_socket_fd = pthread_arg->new_socket_fd;
    char buffer[1024] = {0};
  
    /* TODO: Get arguments passed to threads here. See lines 22 and 116. */
    
    free(arg);

    while(valread = read( new_socket_fd , buffer, 1024))
    {
    	printf("%s\n",buffer );
	memset(buffer, 0, strlen(buffer));
    }


    close(new_socket_fd);
    return NULL;
}



// #TODO: translate net-byte-order


// Omar Juma
// Updated last: February 17, 2015
// CSCI 367, Winter 2015
// Program 2

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int get_user_input(char *message){
    fgets(message, 1024, stdin);
    return 0;
}

main( int argc, char **argv) {
    struct hostent *ptrh; /* pointer to a host table entry */
    struct protoent *ptrp; /* pointer to a protocol table entry */
    struct sockaddr_in sad; /* structure to hold an IP address */
    int sd; /* socket descriptor */
    int port; /* protocol port number */
    char *host; /* pointer to host name */
    char message[1024]; /* User input guess  */
    char buf[1024]; /* buffer for data from the server */

    memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
    sad.sin_family = AF_INET; /* set family to Internet */

    if( argc != 3 ) {
        fprintf(stderr,"Error: Wrong number of arguments\n");
        fprintf(stderr,"usage:\n");
        fprintf(stderr,"./client server_address server_port\n");
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[2]); /* convert to binary */
    if (port > 0) /* test for legal value */
        sad.sin_port = htons((u_short)port);
    else {
        fprintf(stderr,"Error: bad port number %s\n",argv[2]);
        exit(EXIT_FAILURE);
    }

    host = argv[1]; /* if host argument specified */

    /* Convert host name to equivalent IP address and copy to sad. */
    ptrh = gethostbyname(host);
    if ( ptrh == NULL ) {
        fprintf(stderr,"Error: Invalid host: %s\n", host);
        exit(EXIT_FAILURE);
    }

    memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length); // not needed

    /* Map TCP transport protocol name to protocol number. */
    if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
        fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
        exit(EXIT_FAILURE);
    }

    /* Create a socket. */
    sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sd < 0) {
        fprintf(stderr, "Error: Socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    /* Connect the socket to the specified server. */
    if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        fprintf(stderr,"connect failed\n");
        exit(EXIT_FAILURE);
    }

    printf("Connected to chat server: %s\n", host);
    int len;
    while(1){
        printf("TEXT: ");
        len = get_user_input(message);
        len = strlen(message);
        printf("Message: %s\n strlen: %d\n", message, len);
        write(sd, message, strlen(message));
        memset(message, 0, 1024);
        sleep(1);
    }
    /*
        if (( scanf("%[^\n]s", message) < 0)) {
            printf("Error in input: %s\n", message);
            exit(EXIT_FAILURE);
        }
        printf("\nYou entered: %s\n", message);
        write(sd, message, strlen(message));
        printf("Sent\n");
        memset(message, '0', 1024);
    } */

    close(sd);
    exit(EXIT_SUCCESS);
}

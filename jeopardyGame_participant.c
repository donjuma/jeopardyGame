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

fd_set readfds;


main( int argc, char **argv) {
    struct hostent *ptrh; /* pointer to a host table entry */
    struct protoent *ptrp; /* pointer to a protocol table entry */
    struct sockaddr_in sad; /* structure to hold an IP address */
    int sd; /* socket descriptor */
    int port; /* protocol port number */
    char *host; /* pointer to host name */
    char message[512]; /* User input guess  */
    char buf[1024]; /* buffer for data from the server */
    char nick[16];
    char messaget[512];
    int n;
    char question[1024];
    char serial[100];
    char topic[1024];
    char value[16];


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

    memset(message, 0, 512);
    printf("Please select a nickname: ");
    scanf("%s", nick);
    printf("\n");
    printf("%s\n", nick);
    snprintf(message, sizeof(message), "N\n%s\n\n", nick);
    printf("%s\n", message);
    /* Connect the socket to the specified server. */
    if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        fprintf(stderr,"connect failed\n");
        exit(EXIT_FAILURE);
    }

    printf("Connected to game server: %s\n", host);
    sleep(1);
    write(sd, &message, strlen(message));


    while(1){
        n = read(sd, buf, sizeof(buf));
        memset(message, 0, 512);
        memset(messaget, 0, 512);
        memset(question, 0, 1024);
        memset(serial, 0, 1024);
        memset(topic, 0, 1024);
        memset(value, 0, 1024);
        switch(buf[0]){
            case 'R' :
                printf("Unable to join.\n");
                close(sd);
                exit(EXIT_FAILURE);
            case 'Q' :
                sscanf(buf, "Q\n%[^\n]\n%[^\n]\n%[^\n]\n%[^\n]\n\n", serial, topic, value, question);
                printf("serial: %s\n", serial);
                printf("TOPIC: %s\nQUESTION: %s\nAnswer: ", topic, question);
                /*
                FD_ZERO(&readfds);
                FD_SET(sd, &readfds);
                FD_SET(0, &readfds);
                printf("Answer: ");
                if (select(sd+1, &readfds, NULL, NULL, NULL) < 0) {
                    error("ERROR: Error in select!");
                }
                printf("Select returned\n");
                if (FD_ISSET(0, &readfds)){ */
                //fgets(messaget, 512, stdin);
                scanf("%s", messaget);
                printf("Message: %s\n", messaget);
                snprintf(message, 512, "A\n%s\n%s\n\n", serial, messaget);
                printf("Message: %s\n", message);
                write(sd, message, strlen(message));
                /*
                }

                if (FD_ISSET(sd, &readfds)){
                    break;
                } */
                break;
            case 'L' :
                printf("You took too long!\n");
                break;
            case 'K' :
                sscanf(buf, "K\n%s\n\n", value);
                printf("Resulting point difference: %s\n", value);
                break;
            default :
                break;

        }
        memset(buf, 0, 1024);
    }
    close(sd);
    exit(EXIT_SUCCESS);
}

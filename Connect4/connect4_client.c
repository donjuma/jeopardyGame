// Omar Juma
// Updated last: January 21, 2015
// CSCI 367, Winter 2015
// Program 1

/* client.c - code for example client program that uses TCP */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/*------------------------------------------------------------------------
 * Program: client
 *
 * Purpose: allocate a socket, connect to a server, and print all output
 *
 * Syntax: client [ host [port] ]
 *
 * host - name of a computer on which server is executing
 * port - protocol port number server is using
 *
 * Note: Both arguments are optional. If no host name is specified,
 * the client uses "localhost"; if no protocol port is
 * specified, the client uses the default given by PROTOPORT.
 *
 *------------------------------------------------------------------------
 */

void print_board(char *board){
    int maxRows = 6;
    int maxColumns = 7;
    int row = 0;
    int col = 0;

    //This will print it upside down from intuition
    for(row = 0; row < maxRows; row++){
        for(col = 0; col < maxColumns; col++){
            printf("%c ",  board[maxColumns * row + col]);
        }
        printf("\n");  //New line after every row is printed
    }
    //Bottom of display
    printf("-------------\n");
    printf("0 1 2 3 4 5 6\n");
}

main( int argc, char **argv) {
    struct hostent *ptrh; /* pointer to a host table entry */
    struct protoent *ptrp; /* pointer to a protocol table entry */
    struct sockaddr_in sad; /* structure to hold an IP address */
    int sd; /* socket descriptor */
    int port; /* protocol port number */
    char *host; /* pointer to host name */
    char userMove[32]; /* User input guess  */
    int n; /* number of characters read */
    char buf[1000]; /* buffer for data from the server */
    int maxRows = 6;
    int maxColumns = 7;
    char board[42];
    typedef enum { false, true } bool;
    bool game_status = false;

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

    //Read game mode from server
    read(sd, &buf, sizeof(buf));
    if (buf[0] == 'S'){
        printf("STANDARD MODE\n");
    } else if(buf[0] == 'P'){
        printf("Popout Mode\n");
    } else if(buf[0] == 'K'){
        printf("Antistack Mode\n");
    } else{
        printf("INVALID Game mode %c\n", buf[0]);
        exit(EXIT_FAILURE);
    }
    game_status = true; //The game is now live.
    sleep(.5);

    //The game communication logic
    while((n = read(sd, &buf, sizeof(buf))) > 0){
        switch(buf[0]){
            case '2' :
                printf("Waiting for another player to join...\n");
                break;
            case 'Y' :
                if ((n = read(sd, &board, sizeof(board))) > 0){
                    print_board(board);
                    printf("Please enter a move: ");
                    scanf("%s", userMove);
                    write(sd, &userMove, sizeof(userMove));
                }
                break;
            case 'H' :
                if ((n = read(sd, &board, sizeof(board))) > 0){
                    print_board(board);
                    printf("Please wait for other player to make a move.\n");
                } else{
                    fprintf(stderr, "Error: Server sent invalid response! %d\n", buf[0]);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'W' :
                printf("You've won the game!");
                close(sd);
                exit(0);
                break;
            case 'L' :
                printf("You've lost the game!");
                close(sd);
                exit(0);
                break;
            case 'T' :
                printf("The game has tied!");
                close(sd);
                exit(0);
                break;
            case 'I' :
                printf("Invalid move, please try again: ");
                scanf("%s", userMove);
                write(sd, &userMove, sizeof(userMove));
                break;
            default :
                break;
        }
        if (!game_status) break;
    }
    close(sd);
    exit(EXIT_SUCCESS);
}

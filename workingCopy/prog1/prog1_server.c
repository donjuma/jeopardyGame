// Omar Juma
// Updated last: January 21, 2015
// CSCI 367, Winter 2015
// Program 1

/* server.c - code for example server program that uses TCP */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define QLEN 6 /* size of request queue */
int visits = 0; /* counts client connections */

/*------------------------------------------------------------------------
 * Program: server
 *
 * Purpose: allocate a socket and then repeatedly execute the following:
 * (1) wait for the next connection from a client
 * (2) send a short message to the client
 * (3) close the connection
 * (4) go back to step (1)
 *
 * Syntax: server [ port ]
 *
 * port - protocol port number to use
 *
 * Note: The port argument is optional. If no port is specified,
 * the server uses the default given by PROTOPORT.
 *
 *------------------------------------------------------------------------
 */


void swap(int *a, int *b) {
    int c = *a;
    *a = *b;
    *b = c;
}

int connect4(char *board, int a, int b, int c, int d){
    return (board[a] == board[b] && board[b] == board[c] && board[c] == board[d] && board[a] != '0');

}

int _checkWinCol(char *board){
    int row;
    int col;
    int position;

    for(row = 0; row < 3; row++){
       for(col = 0; col < 7; col++){
          position = 7 * row + col;
          if(connect4(board, position, position + 7, position + 14, position + 21)){
              return 1;
          }
       }
    }
    return 0;

}
int _checkWinDia(char *board){
   int row;
   int col;
   int position;
   int i;

   for(row = 0; row < 3; row++){
      for(col = 0; col < 7; col++){
         position = 7 * row + col;
         if ( ( (i < 4) && connect4(board, position, position + 8, position + 16, position + 24 )) ||
                 ( (i >= 3) && connect4(board, position, position + 6, position + 12,position + 18) )){
            return 1;
         }
         ++i;
      }
      i = 0;
   }
   return 0;

}

int _checkWinRow(char *board){
    int row;
    int col;
    int position;

    for(row = 0; row < 6; row++){
       for(col = 0; col < 4; col++){
          position = 7 * row + col;
          if(connect4(board, position, position + 1 , position + 2, position+3)){
             return 1;
          }
       }
    }
    return 0;
}

int _KcheckWinDia(char *board){
   int row;
   int col;
   int position;
   int i;

   for(row = 0; row < 3; row++){
      for(col = 0; col < 8; col++){
         position = 7 * row + col;
         if ( ( (i < 4) && connect3(board, position, position + 8, position + 16 )) ||
                 ( (i >= 2) && connect3(board, position, position + 6, position + 12) )){
            return 1;
         }
         ++i;
      }
      i = 0;
   }
   return 0;

}

int _KcheckWinRow(char *board){
    int row;
    int col;
    int position;

    for(row = 0; row < 6; row++){
       for(col = 0; col < 6; col++){
          position = 7 * row + col;
          if(connect3(board, position, position + 1 , position + 2)){
             return 1;
          }
       }
    }
    return 0;
}

int connect3(char *board, int a, int b, int c){
    return (board[a] == board[b] && board[b] == board[c] && board[a] != '0');

}

int _KcheckWinCol(char *board){
    int row;
    int col;
    int position;

    for(row = 0; row < 3; row++){
       for(col = 0; col < 8; col++){
          position = 7 * row + col;
          if(connect3(board, position, position + 7, position + 14)){
              return 1;
          }
       }
    }
    return 0;

}

int popout_mode(int col, char *board, char move, char action){
    int row;
    if (action == 'A'){
        for(row = 5; row >= 0; row--){
            if(board[7 * row + col] == '0'){
                board[7 * row + col] = move;
                return 1;
            }
        }
        return 0;
    }else if (action == 'P'){
        if(board[35 + col] == move){
            board[35+col] = '0';
            return 1;
        }
        return 0;
    }
    return 0;
}

int standard_mode(int col, char *board, char move){
    int row;
    for(row = 5; row >= 0; row--){
        if(board[7 * row + col] == '0'){
            board[7 * row + col] = move;
            return 1;
        }
    }
    return 0;
}

int processInput(char *buf, char *game_type, char *board, int playerMove){
    char action = buf[0]; //Retrieve the action type ([A]dd / [P]opout)
    int col = buf[1] - '0'; //Convert it to an integer
    char move = (char)(((int)'0')+playerMove); //Convert the token into a char (1/2)

    switch(game_type[0]){
        case 'S' :
            if ((action != 'A') || col > 6 || col < 0){
                return 0;
            }
            return standard_mode(col, board, move);
            break;

        case 'P' :
            if ( (action == 'A') || (action == 'P')){
                if (col > 6 || col < 0){
                    return 0;
                }
                else{
                    return popout_mode(col, board, move, action);
                }
            }
            else return 0;
            break;

        case 'K' :
            if ((action != 'A') || col > 6 || col < 0){
                return 0;
            }
            return standard_mode(col, board, move);
            break;

        default :
            break;
    }

}

int checkWin(char *board, char *game_type){
    if ( (game_type[0] == 'S') || (game_type[0] == 'P')){
        return (_checkWinRow(board) || _checkWinCol(board) || _checkWinDia(board));
    }
    else if (game_type[0] == 'K'){
        return (_KcheckWinRow(board) || _KcheckWinCol(board) || _KcheckWinDia(board));
    }
    return 0;
}

int checkTie(char *board){
    int i;
    for (i = 0; i <42; ++i){
        if ( (board[i] == '0') ){
            return 0;
        }
    }
    return 1;
}

void game(int sd2, int sd3, char *game_type){
    char buf[1000];     //Initialize buffer for communication
    int activePlayer = sd2;
    int unactivePlayer = sd3;
    int activePlayerMove = 1;
    int unactivePlayerMove = 2;
    int n, v;
    char board[42];
    memset(board, '0', 42); //Initialize the board to all zeros!

    while(1){
        buf[0] = 'H';
        write(unactivePlayer, &buf[0], sizeof(buf[0]));
        buf[0] = 'Y';
        write(activePlayer, &buf[0], sizeof(buf[0]));

        //Send board information
        write(unactivePlayer, board, sizeof(board));
        write(activePlayer, board, sizeof(board));

        while((n = read(activePlayer, &buf, sizeof(buf))) > 0){
            if (v = processInput(buf, game_type, board, activePlayerMove) <= 0){
                buf[0] = 'I';
                write(activePlayer, &buf[0], sizeof(buf[0]));
            }
            else{
                break;
            }
        }

        //Check if player 1 caused player 2 to win
        if (checkWin(board, game_type)){
            if (game_type[0] == 'K'){
                swap(&activePlayer, &unactivePlayer);
            }
            buf[0] = 'W';
            write(activePlayer, &buf[0], sizeof(buf[0]));
            close(activePlayer);
            buf[0] = 'L';
            write(unactivePlayer, &buf[0], sizeof(buf[0]));
            close(unactivePlayer);
            exit(0);
        }

        if (checkTie(board)){
            buf[0] = 'T';
            write(activePlayer, &buf[0], sizeof(buf[0]));
            close(activePlayer);
            write(unactivePlayer, &buf[0], sizeof(buf[0]));
            close(unactivePlayer);
            exit(0);
        }

        swap(&activePlayer, &unactivePlayer);
        swap(&activePlayerMove, &unactivePlayerMove);
    }
    close(sd2);
    close(sd3);
    exit(0);
}


int main(int argc, char **argv) {
    struct protoent *ptrp; /* pointer to a protocol table entry */
    struct sockaddr_in sad; /* structure to hold server's address */
    struct sockaddr_in cad; /* structure to hold client's address */
    int sd, sd2, sd3; /* socket descriptors */
    int port; /* protocol port number */
    int alen; /* length of address */
    char buf[1000]; /* buffer for string the server sends */
    char game_type[4]; //Identifier of the selected game mode
    pid_t pid;
    if( argc != 3 ) {
        fprintf(stderr,"Error: Wrong number of arguments\n");
        fprintf(stderr,"usage:\n");
        fprintf(stderr,"./server server_port game_type\n");
        exit(EXIT_FAILURE);
    }

    memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
    sad.sin_family = AF_INET; /* set family to Internet */
    sad.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */

    port = atoi(argv[1]); /* convert argument to binary */

    if (strcmp(argv[2], "standard") == 0) {
        game_type[0] = 'S';
    } else if (strcmp(argv[2], "popout") == 0) {
        game_type[0] = 'P';
    } else if (strcmp(argv[2], "antistack") == 0) {
        game_type[0] = 'K';
    } else{
        fprintf(stderr, "Error: Invalid Game Type %s\n", argv[2]);
        exit(0);
    }

    if (port > 0) { /* test for illegal value */
        sad.sin_port = htons((u_short)port);
    } else { /* print error message and exit */
        fprintf(stderr,"Error: Bad port number %s\n",argv[1]);
        exit(EXIT_FAILURE);
    }

    /* Map TCP transport protocol name to protocol number */
    if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
        fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
        exit(EXIT_FAILURE);
    }

    /* Create a socket */
    sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sd < 0) {
        fprintf(stderr, "Error: Socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    /* Bind a local address to the socket */
    if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        fprintf(stderr,"Error: Bind failed\n");
        exit(EXIT_FAILURE);
    }

    /* Specify size of request queue */
    if (listen(sd, QLEN) < 0) {
        fprintf(stderr,"Error: Listen failed\n");
        exit(EXIT_FAILURE);
    }

    /* Main server loop - accept and handle requests */
    for (;;) {
        buf[0] = 0;
        alen = sizeof(cad);
        if ( (sd2=accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
            fprintf(stderr, "Error: Accept failed\n");
            exit(EXIT_FAILURE);
        }
        write(sd2, &game_type[0], sizeof(game_type[0])); //Send player 1 game mode
        buf[0] = '2';
        write(sd2, &buf[0], sizeof(buf[0])); //Inform Player 1 we are waiting for player 2

        if ( (sd3=accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
            fprintf(stderr, "Error: Accept failed\n");
            exit(EXIT_FAILURE);
        }
        write(sd3, &game_type[0], sizeof(game_type[0])); //Send player 2 game mode
        signal(SIGCHLD,SIG_IGN);
        /*Child process logic  */
        pid = fork();

        if (pid < 0){
            printf("Error in fork\n");
        }

        else if (pid == 0) {
            close(sd);
            game(sd2, sd3, game_type);
            close(sd2);
            close(sd3);
            exit(0);
        }
        else{
            close(sd2);
            close(sd3);
        }
    }
}

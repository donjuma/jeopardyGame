//TODO: Make sure to support net-byte-order


// Omar Juma
// Updated last: February 17, 2015
// CSCI 367, Winter 2015
// Program 2

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define QLEN 64 /* size of request queue */
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

int participantsList[64];
int observersList[64];
fd_set participants;
fd_set part_fds;
fd_set observers;
fd_set obsv_fds;
int topParticipant;
int topObserver;
int observer_socket = 0;
int participant_socket = 0;
struct sockaddr_in cad;
int alen;


void select_init() {
    //ZERO OUT ALL OF OUR DESCRIPTOR SETS
    FD_ZERO(&participants);
    FD_ZERO(&observers);
    FD_ZERO(&part_fds);
    FD_ZERO(&obsv_fds);


    //Now add listening sockets to respective sets
    FD_SET(participant_socket, &participants);
    FD_SET(observer_socket, &participants);

    //TODO: Make sure this inits at zero!
    memset((char *) &participantsList, 0, sizeof(participantsList));
    memset((char *) &observersList, 0, sizeof(observersList));

    topParticipant = 0;
    topObserver = 0;

    //Initialize the top most sockets so far
    printf("Participant Socket: %d\nObserver Socket: %d\n", participant_socket, observer_socket);
    topParticipant = participant_socket;
    topObserver = observer_socket;

}

void reset_select(){
    int i;
    int bounds;

    //Clear all of the file descriptor sets
    FD_ZERO(&participants);
    FD_ZERO(&observers);
    FD_ZERO(&part_fds);
    FD_ZERO(&obsv_fds);


    FD_SET(participant_socket, &participants);
    FD_SET(observer_socket, &participants);

    //Add 2 just in case for buffer. Shouldn't cause that much overhead.
    bounds = MAX(topParticipant, topObserver) + 20;

    for (i=0; i < 64; i++){
        if (participantsList[i] != 0){
            FD_SET(participantsList[i], &participants);
            //Check if new connection...
            topParticipant = MAX(topParticipant, i);
        }
        if (observersList[i] != 0){
            FD_SET(observersList[i], &observers);
            topObserver = MAX(topObserver, i);
        }
    }
}

void send_msg(char buffer[]) {
    int i;
    //TODO: STRIP NULL TERMINATOR &&& APPEND MISSING NEW LINE CHAR.
    printf("Top observer: %d\nMessage: %s\n", topObserver, buffer);

    for (i = 0; i <= topObserver; i++){
        //Check of observer is ready to write to...
        //if (FD_ISSET(observersList[i], &obsv_fds)){
            //Again, check for new observers... REDUNDANT
        if (observersList[i] != observer_socket){
            printf("Writing: %s, to %d\n", buffer, observersList[i]);
            if (write(observersList[i], buffer, strlen(buffer)) < 0){
                //Connection closed or errored out...
                printf("Observer: %d connection closed\n", observersList[i]);
                close(observersList[i]);
                FD_CLR(observersList[i], &observers);
                observersList[i] = 0;
            }
        }
        //}
    }
}

void new_participant_connection() {
    int newParticipant;
    int i;
    char JOIN_MSG[100];

    if ( (newParticipant = accept(participant_socket, (struct sockaddr *)&cad, &alen)) < 0) {
        fprintf(stderr, "Error accepting participant: %d\n", participant_socket);
        exit(EXIT_FAILURE);
    }
    //Iterate over participantsList to find lowest free spot!
    for (i = 0; ((i < 64) && (newParticipant != -1)); i++){
        //Check if free spot in participantsList
        if (participantsList[i] == 0){
            printf("Free participants slot: %d\n", i);
            printf("New Participant socket: %d\n", newParticipant);
            //Check for disconnect...
            if (newParticipant != -1){
                participantsList[i] = newParticipant;
                newParticipant = -1; //Mark socket as added to set

                //Add new participant to set
                FD_SET(participantsList[i], &participants);
                topParticipant = MAX(topParticipant, i); //Recalc top most socket

                //Update observers that new participant has joined
                snprintf(JOIN_MSG, sizeof(JOIN_MSG), "USER %d has joined.\n", i);
                send_msg(JOIN_MSG);
                reset_select();
                break;
            }
        }
    }

    //If the participant was never marked/added to the set...
    if (newParticipant != -1){
        close(newParticipant);
    }
}


void new_observer_connection() {
    int newObserver;
    int i;
    char JOIN_MSG[100];

    printf("New observer connection has been called\n");
    if ( (newObserver = accept(observer_socket, (struct sockaddr *)&cad, &alen)) < 0) {
        //fprintf(stderr, "Error accepting observer: %d\n", observer_socket);
        //exit(EXIT_FAILURE);
        return;
    }
    printf("Just accepted observer: %d\n", newObserver);
    //Iterate over observersList to find lowest free spot!
    for (i = 0; ((i < 64) && (newObserver != -1)); i++){
        //Check if free spot in observersList
        if (observersList[i] == 0){
            printf("Free observers slot: %d\n", i);
            printf("New observer socket: %d\n", newObserver);
            //Check for disconnect...
            if (newObserver != -1){
                observersList[i] = newObserver;
                newObserver = -1; //Mark socket as added to set

                //Add new participant to set
                FD_SET(observersList[i], &observers);
                topObserver = MAX(topObserver, i) + 2; //Recalc top most socket

                //Update observers that new participant has joined
                snprintf(JOIN_MSG, sizeof(JOIN_MSG), "A new observer has joined.\n");
                send_msg(JOIN_MSG);
                reset_select();
                break;
            }
        }
    }

    if (newObserver != -1){
        close(newObserver);
    }
}

int main(int argc, char **argv) {
    struct protoent *ptrp; /* pointer to a protocol table entry */
    struct sockaddr_in sad_p; /* structure to hold server's address */
    struct sockaddr_in sad_o; /* structure to hold server's address */
    //struct sockaddr_in cad; /* structure to hold client's address */
    //int participant_socket; /* socket descriptors */
    //int observer_socket; /* socket descriptors */
    int participant_port; /* protocol port number for participant clients */
    int observer_port; /* protocol port number for observer clients*/
    int conn_status; /* integer to hold most recent connection status */
    //int alen; /* length of address */
    int i;
    //fd_set part_fds;
    //fd_set obsv_fds;
    char buf[1024]; /* buffer for string the server sends */

    if( argc != 3 ) {
        fprintf(stderr,"Error: Wrong number of arguments\n");
        fprintf(stderr,"usage:\n");
        fprintf(stderr,"./server participant_port observer_port\n");
        exit(EXIT_FAILURE);
    }

    memset((char *)&sad_p,0,sizeof(sad_p)); /* clear sockaddr structure */
    memset((char *)&sad_o,0,sizeof(sad_o)); /* clear sockaddr structure */
    sad_p.sin_family = AF_INET; /* set family to Internet */
    sad_p.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */
    sad_o.sin_family = AF_INET; /* set family to Internet */
    sad_o.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */

    participant_port = atoi(argv[1]); /* convert argument to binary */
    observer_port = atoi(argv[2]);    /* convert argument to binary */

    printf("P-PORT: %d\nO_PORT: %d\n", participant_port, observer_port);

    if ( (participant_port > 0) && (observer_port > 0)) { /* test for illegal value */
        sad_p.sin_port = htons((u_short)participant_port);
        sad_o.sin_port = htons((u_short)observer_port);
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
    observer_socket = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    participant_socket = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);

    if ( (participant_socket < 0) || (observer_socket < 0) ) {
        fprintf(stderr, "Error: Socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    //setnonblocking(observer_socket);
    //setnonblocking(participant_socket);

    /* Bind a local address to the socket */
    if (bind(participant_socket, (struct sockaddr *)&sad_p, sizeof(sad_p)) < 0) {
        fprintf(stderr,"Error: Bind failed (Participant)\n");
        exit(EXIT_FAILURE);
    }
    if (bind(observer_socket, (struct sockaddr *)&sad_o, sizeof(sad_o)) < 0) {
        fprintf(stderr,"Error: Bind failed (observer)\n");
        exit(EXIT_FAILURE);
    }

    if (listen(observer_socket, QLEN) < 0) {
        fprintf(stderr,"Error: Listen failed\n");
        exit(EXIT_FAILURE);
    }

    if (listen(participant_socket, QLEN) < 0) {
        fprintf(stderr,"Error: Listen failed\n");
        exit(EXIT_FAILURE);
    }

    select_init();
    /* Main server loop - accept and handle requests */
    for (;;) {

        printf("Memset\n");
        memset(buf, 0, 1024);
        //select_init(participant_socket, observer_socket);

        //Create temp sets that will be altered every iteration
        part_fds = participants;
        obsv_fds = observers; //Might not be needed afterall
        int numfds = MAX(topParticipant, topObserver) + 10;

        //new_observer_connection();
        printf("Calling Select...\n");
        if (select(numfds, &part_fds, NULL, NULL, NULL) == -1) {
            fprintf(stderr, "Error using select() call\n");
            exit(EXIT_FAILURE);
        }
        printf("Checking new connections\n");
        if (FD_ISSET(observer_socket, &part_fds)){
            printf("%d is a new observer connection\n", observer_socket);
            new_observer_connection();
        }
        if (FD_ISSET(participant_socket, &part_fds)){
            printf("%d is a new participant connection\n", participant_socket);
            new_participant_connection();
        }
        printf("Checking new connections\n");
        printf("Top participant: %d\n", topParticipant);
        printf("Top observer: %d\n", topObserver);
        printf("Entering for loop\n");
        //Iterate through all readable sockets (i.e., ready participants)
        for (i=0; i <= topParticipant; i++) {
            printf("Checking participant: %d - i:%d\n", participantsList[i], i);
            //Check if this [i] socket is ready to read...
            if (FD_ISSET(participantsList[i], &part_fds)){
                printf("%d is ready to read..\n", participantsList[i]);
                //IF THERE ARE ANY ERROR HERE CHANGE TO '&buf'
                //TODO: change to while. keep reading until = 0;
                if ((conn_status = read(participantsList[i], buf, sizeof(buf))) <= 0){
                    //Connection has ended or errored out.
                    //Either way, we need to close socket and remove from set
                    close(participantsList[i]);
                    FD_CLR(participantsList[i], &participants);
                    participantsList[i] = 0;
                    snprintf(buf, sizeof(buf), "USER %d has quit!\n", participantsList[i]);
                    send_msg(buf);
                }

                else {
                    printf("%d has written: %s\n", participantsList[i], buf);
                    send_msg(buf);
                }
            }
            else{
                continue;
            }
        }
        printf("Out of foor loop\n");
        reset_select();
    }
    return 0;
}

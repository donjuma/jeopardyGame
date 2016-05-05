//TODO: Make sure to support net-byte-order


// Omar Juma
// Updated last: February 17, 2015
// CSCI 367, Winter 2015
// Program 2

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define QLEN 64 /* size of request queue */
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

int participantsList[16];
int observersList[64];
char *nicks[16];
int scores[16];
fd_set participants;
fd_set part_fds;
fd_set observers;
fd_set obsv_fds;
fd_set connections;
fd_set conn_fds;
fd_set temp_fds;
int topParticipant;
int topObserver;
int observer_socket = 0;
int participant_socket = 0;
struct sockaddr_in cad;
int alen;
int part_count = 0;
struct timeval tv;
struct timeval tvTWO;


void select_init() {
    //ZERO OUT ALL OF OUR DESCRIPTOR SETS
    FD_ZERO(&participants);
    FD_ZERO(&observers);
    FD_ZERO(&connections);
    FD_ZERO(&part_fds);
    FD_ZERO(&obsv_fds);
    FD_ZERO(&conn_fds);
    FD_ZERO(&temp_fds);

    //Now add listening sockets to respective sets
    FD_SET(participant_socket, &connections);
    FD_SET(observer_socket, &connections);

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
    FD_ZERO(&connections);
    FD_ZERO(&conn_fds);
    FD_ZERO(&part_fds);
    FD_ZERO(&obsv_fds);
    FD_ZERO(&temp_fds);

    FD_SET(participant_socket, &conn_fds);
    FD_SET(observer_socket, &conn_fds);

    //Add 2 just in case for buffer. Shouldn't cause that much overhead.
    bounds = MAX(topParticipant, topObserver) + 20;

    for (i=0; i < 16; i++){
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
    for (i=16; i < 64; i++){
        if (observersList[i] != 0){
            FD_SET(observersList[i], &observers);
            topObserver = MAX(topObserver, i);
        }
    }
}


void participantDisconnect(int i){
    FD_CLR(participantsList[i], &participants);
    free(nicks[i]);
    scores[i] = 0;
    part_count--;
    close(participantsList[i]);
    participantsList[i] = 0;
}

void printScores(){
    int i;
    int j;
    char buffer[1024];
    char name[16];
    int score;

    for (i = 0; i <= topObserver; i++){
        if ( (observersList[i] != observer_socket) && (observersList[i] != 0) ){
            for (j=0; j<16; j++){
                snprintf(name, sizeof(name), "%s", nicks[i]);
                score = scores[i];
                snprintf(buffer, sizeof(buffer), "%s: %d\n", name, score);
                if (write(observersList[i], buffer, strlen(buffer)) < 0){
                    close(observersList[i]);
                    observersList[i] = 0;
                }
            }
        }
    }
}

void send_msg(char buffer[]) {
    int i;

    for (i = 0; i <= topParticipant; i++){
        //Check of observer is ready to write to...
        //if (FD_ISSET(observersList[i], &obsv_fds)){
            //Again, check for new observers... REDUNDANT
        if ( (participantsList[i] != participant_socket)  ){
            printf("Writing: %s, to %d\n", buffer, participantsList[i]);
            if (write(participantsList[i], buffer, strlen(buffer)) < 0){
                //Connection closed or errored out...
                printf("Participants: %d connection closed\n", participantsList[i]);
                //participantDisconnect(i);
            }
        }
    }
}

int processName(char name[]){
    char buf[120];
    int i;
    memset(buf, 0, 120);

    if ( (strlen(name) < 1) || (strlen(name) > 16) ){
        //snprintf(buf, sizeof(buf), "R\nInvalid Nick\n\n");
        //send(sock, buf, strlen(buf), MSG_DONTWAIT);
        return 0;
    }

    for (i=0; i < 16; i++){
        if (strcmp(name, nicks[i]) == 0){
            snprintf(buf, sizeof(buf), "R\nInuse Nick\n\n");
            //send(sock, buf, strlen(buf), MSG_DONTWAIT);
            return 0;
        }
    }
    return 1;
}
void new_participant_connection() {
    int newParticipant = 0;
    int i;
    char name[16];
    char buf[120];
    memset(buf, 0, 120);
    memset(name, 0, 16);


    if ( (newParticipant = accept(participant_socket, (struct sockaddr *)&cad, &alen)) < 0) {
        fprintf(stderr, "Error accepting participant: %d\n", participant_socket);
        exit(EXIT_FAILURE);
    }
    printf("Accepted new participant\n");
    if (part_count > 15){
        close(newParticipant);
        part_count--;
        return;
    }
    printf("Setting new part to tempfds\n");
    FD_ZERO(&temp_fds);
    FD_SET(newParticipant, &temp_fds);
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    printf("Calling nickname select\n");
    /*if (select(3, &temp_fds, NULL, NULL, NULL) == -1) {
        fprintf(stderr, "Error using select() call\n");
        part_count--;
        return;
    }
    printf("Select returned\n");
    if(FD_ISSET(newParticipant, &temp_fds)){
        printf("Name was set\n"); */
    if ((i = read(newParticipant, buf, sizeof(buf))) <= 0){
        printf("Error could not read name\n");
        close(newParticipant);
        part_count--;
        return;
    }
    sscanf(buf, "N\n%[^\n]\n\n", name);
    printf("Name: %s\n", name);
    /*
    if (processName(name) < 0){
        printf("Process name error\n");
        close(newParticipant);
        part_count--;
        return;
    }

    else{
        printf("else close\n");
        close(newParticipant);
        part_count--;
        return;
    }
*/
    //Iterate over participantsList to find lowest free spot!
    for (i = 0; ((i < 16) && (newParticipant != -1)); i++){
        //Check if free spot in participantsList
        if (participantsList[i] == 0){
            printf("Free participants slot: %d\n", i);
            printf("New Participant socket: %d\n", newParticipant);
            //Check for disconnect...
            if (newParticipant != -1){
                participantsList[i] = newParticipant;
                nicks[i] = strdup(name);
                scores[i] = 0;
                newParticipant = -1; //Mark socket as added to set

                break;
            }
        }
    }

    //If the participant was never marked/added to the set...
    if (newParticipant != -1){
        printf("nP -1\n");
        close(newParticipant);
        part_count--;
    }
}


void new_observer_connection() {
    int newObserver;
    int i;

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

                //Update observers that new participant has joined
                reset_select();
                break;
            }
        }
    }

    if (newObserver != -1){
        close(newObserver);
    }
}

int score(int index, char response[], char answer[], int value, int qSerial){
    int result;
    char code;
    char thisSerial[16];
    char guess[512];
    char buf[100];
    char points[16];

    printf("Response: %s\n", response);
    result = sscanf(response, "%s\n%s\n%s\n\n", code, thisSerial, guess);
    printf("Code: %s\n", code);
    printf("Serial: %s\n", thisSerial);
    printf("Guess: %s\n", guess);

    //Convert thisSerial into an int
    int serial = atoi(thisSerial);
    printf("Serial: %d\nthisSerial: %s\nqSerial: %d\n", serial, thisSerial, qSerial);
    if (serial != qSerial){
        snprintf(buf, sizeof(buf), "L\n\n");
        if (send(participantsList[index], buf, strlen(buf), MSG_DONTWAIT) < 0){
                //Connection closed or errored out...
                participantDisconnect(index);
                return -2;
        }
        return 1;
    }

    //Correct Answer
    if (!(strcmp(answer, guess) == 0)){
        value = (-1)*value;
    }
    snprintf(points, 16, "%d", value);
    snprintf(buf, sizeof(buf), "K\n%s\n\n");
    if (send(participantsList[index], buf, strlen(buf), MSG_DONTWAIT) < 0){
            //Connection closed or errored out...
            participantDisconnect(index);
            return -2;
    }
    return 1;

}

int main(int argc, char **argv) {
    struct protoent *ptrp; /* pointer to a protocol table entry */
    struct sockaddr_in sad_p; /* structure to hold server's address */
    struct sockaddr_in sad_o; /* structure to hold server's address */
    int participant_port; /* protocol port number for participant clients */
    int observer_port; /* protocol port number for observer clients*/
    int conn_status; /* integer to hold most recent connection status */
    int i;
    char buf[1024]; /* buffer for string the server sends */
    char topic[1024];
    char value[1024];
    char question[1024];
    char answer[1024];

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

    int qSerial = 0;
    int pointVal;
    char strSerial[16];
    select_init();
    /* Main server loop - accept and handle requests */
    while(!feof(stdin)) {

        printf("New Round...\n");
        fscanf( stdin, "%[^$]$%[^\t]\t%[^\t]\t%[^\n]\n", topic, value, question, answer );
        //Formatting
        topic[strlen(topic)-1] = 0;
        pointVal = atoi(value);
        qSerial++;
        snprintf(strSerial, 16, "%d", qSerial);
        //itoa(qSerial, strSerial, 10);
        memset(buf, 0, 1024);

        //Create temp sets that will be altered every iteration
        conn_fds = connections;
        part_fds = participants;
        obsv_fds = observers; //Might not be needed afterall
        int numfds = MAX(topParticipant, topObserver) + 10;

        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if(!part_count){
            tv.tv_sec = 300;
            tv.tv_usec = 0;
        }

//#TODO:Keep checking select over and over for 5 seconds...
        printf("Waiting for 5 seconds");
        sleep(5);
        printf("Calling Select...\n");
        if (select(numfds, &conn_fds, NULL, NULL, &tv) == -1) {
            fprintf(stderr, "Error using select() call\n");
            exit(EXIT_FAILURE);
        }
        printf("Checking new connections\n");
        if (FD_ISSET(observer_socket, &conn_fds)){
            printf("%d is a new observer connection\n", observer_socket);
            new_observer_connection();
        }
        if (FD_ISSET(participant_socket, &conn_fds)){
            printf("%d is a new participant connection\n", participant_socket);
            new_participant_connection();
            part_count++;
        }
        reset_select();
        part_fds = participants;
        printf("Checking new connections\n");
        printf("Top participant: %d\n", topParticipant);
        printf("Top observer: %d\n", topObserver);
        printf("Entering for loop\n");

        //Send question to all participants
        snprintf(buf, sizeof(buf), "Q\n%s\n%s\n%s\n%s\n\n", strSerial, topic, value, question);
        sleep(2);
        send_msg(buf);

        //Wait 10 Seconds for round
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        if (select(0, NULL, NULL, NULL, &tv) < 0) perror("select");

        tv.tv_sec = 0;
        tv.tv_usec = 0;
        if (select(numfds, &part_fds, NULL, NULL, &tv) == -1) {
            fprintf(stderr, "Error using select() call\n");
            exit(EXIT_FAILURE);
        }

        //Iterate through all readable sockets (i.e., ready participants)
        for (i=0; i <= topParticipant+2; i++) {
            printf("Checking participant: %d - i:%d\n", participantsList[i], i);
            //Check if this [i] socket is ready to read...
            if (FD_ISSET(participantsList[i], &part_fds)){
                printf("%d is ready to read..\n", participantsList[i]);
                //IF THERE ARE ANY ERROR HERE CHANGE TO '&buf'
                //TODO: change to while. keep reading until = 0;
                memset(buf, 0, 1024);
                if ((conn_status = read(participantsList[i], buf, sizeof(buf))) <= 0){
                    participantDisconnect(i);
                }

                else {
                    if(score(i, buf, answer, pointVal, qSerial) < 1){
                        continue;
                    }
                }
            }
            else{
                snprintf(buf, sizeof(buf), "L\n0\n\n");
                send(participantsList[i], buf, strlen(buf), MSG_DONTWAIT);
                continue;
            }
        }
        printScores();
        printf("Out of foor loop\n");
        reset_select();
    }
    return 0;
}

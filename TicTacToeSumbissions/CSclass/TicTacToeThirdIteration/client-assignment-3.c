/*
** Group 4: Henry Lembo, Anthony Lucchese, Prisco Pocius
**
** a simple web browser
**
** gcc -o client client-assignment-3.c client-thread-2021.c && ./client freebsd1.cs.scranton.edu 17400 c*.h
**
** the server is supposed to start on port 4000 and
** make sure the server has been started before this.
**
** Basic steps for client to communicate with a server
** 1: get a socket
** 2: connect to the server using ip and port
** 3: read and/or write as many times as needed
** 4: close the socket.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "client-thread-2021.h"

// you must change the port to your team's assigned port. 
#define BUFFERSIZE 256 

#define NORM "\x1B[0m"
#define RED "\x1B[31m"
#define CYN "\x1B[36m"

#define TYPE 1

void printBoard(char board[3][3]);
void receiveBoard(char board[3][3], int sock);

void endGameState(int state);
void inputMove(char board[3][3], int sock);
void receiveScoreBoard(int sock);

int initGame();

int main(int argc, char *argv[]) {
    int web_server_socket;  

    if (argc != 4) {
        printf("usage: client HOST HTTPORT webpage\n");
        exit(1);
    }
    web_server_socket = initGame(argv);
    int op = 0;
    char board[3][3];
    int buff = 0;
    while (op == 0) {  // can change it to read in chars
        recv(web_server_socket, &op, sizeof(int), 0);;
        if (op == 0) {
            printf("Its your turn!\n");
            receiveBoard(board, web_server_socket);
            inputMove(board, web_server_socket);
        } else {
            endGameState(op);
            receiveBoard(board, web_server_socket);
            printf("Game ending board: \n");
            printBoard(board);
            receiveScoreBoard(web_server_socket);
        }
    }   
    close(web_server_socket);
}

int initGame(char *argv[]) {
    char user[33];
    char pass[33];
    char opponent[33];

    int sock;

    printf("Enter Username: ");
    scanf("%s", user);
    printf("Enter Password: ");
    scanf("%s", pass);
   
    if ((sock = get_server_connection(argv[1], argv[2])) == -1) {
        printf("connection error\n");
        exit(1);
    }

    int isPassCorrect = -1;
    //printf("sending username and password...\n");
    send(sock, &user, sizeof(user), 0);
    send(sock, &pass, sizeof(pass), 0);

    // recieving if the password is correct, and if not, user is disconnected
    recv(sock, &isPassCorrect, sizeof(int), 0);
    
    if (isPassCorrect == -1) {
        printf("%s\nWrong Password!!!%s\nDisconnected...\n", RED, NORM);
        close(sock);
        exit(1);
    } else {
        printf("User has been authenticated!\n");
    }

    recv(sock, &opponent, sizeof(opponent), 0);
    printf("Your Name: %s | Opponent Name: %s \n", user, opponent);

    return sock;
}

void receiveBoard(char board[3][3], int sock) {
    for(int i = 0; i <= 2; i++) {
        for(int j = 0; j <=2; j++) {
            recv(sock, &board[i][j], sizeof(char), 0);
        }
    }
}

void endGameState(int state) {
    if (state == 10){
        printf("It's a Tie.\n");
    } else if (state == -1) {
        printf("You Lost...\n");
    } else {
        printf("You Win!\n");
    }
}

void inputMove(char board[3][3], int sock){
    int invalid = 1;
    int nBytes = 0;
    int coords[2];
    char player;

    printBoard(board);
    recv(sock, &player, sizeof(char), 0);
    while (invalid) {
        printf("Format: x y\n");
        scanf("%d%d", &coords[0], &coords[1]);
        if (((0 <= coords[0] && coords[0] <= 2) && (0 <= coords[1] && coords[1] <= 2 )) && board[coords[0]][coords[1]] == ' ') { //
            send(sock, &coords[0], sizeof(int), 0);
            send(sock, &coords[1], sizeof(int), 0);
            board[coords[0]][coords[1]] = player;
            printf("New Board: \n");
            printBoard(board);
            printf("%sSent Move!\n%s", CYN, NORM);
            invalid = 0;
         } else {
            printf("%sInvalid placement!%s\n", RED, NORM);
      }
   }
}

/* dont change */
void printBoard(char board[3][3]) {
    if (board == NULL) {
        printf("There is no current game!\n");
    } else {
        printf("\n");
        for(int i = 0; i <= 2; i++ ){
           printf("		 %c | %c | %c \n", board[i][0], board[i][1], board[i][2]);
           if (i < 2) {
               printf("		-----------\n");
         } 
      }
   }
}

void receiveScoreBoard(int sock) {
    char printscoreboard[100] = "";
    for(int i = 0; i < 99; i++){
        recv(sock, &printscoreboard[i], sizeof(char), 0);
    }
    printf("%s \n",printscoreboard);
} 

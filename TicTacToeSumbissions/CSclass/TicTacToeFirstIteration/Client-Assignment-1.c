/*
** GROUP 4
** Henry Lembo, Anthony Lucchese, Prisco Pocius
**
** a simple web browser
** compile: gcc -o client Client-Assignment-1.c client-thread-2020.c
** run: client freebsd().cs.scranton.edu 17400 c*.h
** gcc -o client Client-Assignment-1.c client-thread-2021.c && ./client freebsd1.cs.scranton.edu 17400 c*.h
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
 
//just some colors :)
#define NORM "\x1B[0m"
#define RED "\x1B[31m"
#define CYN "\x1B[36m"

#define TYPE 1

void printBoard(char board[3][3]);
void receiveBoard(char board[3][3], int sock);

void endGameState(int state);
void inputMove(char board[3][3], int sock);
// void help();

int main(int argc, char *argv[]) {
      int web_server_socket;     
      if (argc != 4) {
            printf("usage: client HOST HTTPORT webpage\n");
            exit(1);
      }
      if ((web_server_socket = get_server_connection(argv[1], argv[2])) == -1) {
            printf("connection error\n");
            exit(1);
      }
      int op = 0;
      char board[3][3];
      while (op == 0) {  // can change it to read in chars
            recv(web_server_socket, &op, sizeof(int), 0);
            // printf("\n%d\n\n", op);
            if (op == 0) {
                  printf("Its your turn!\n");
                  receiveBoard(board, web_server_socket);
                  inputMove(board, web_server_socket);
            } else {
                  endGameState(op);
                  receiveBoard(board, web_server_socket);
                  printf("Game ending board: \n");
                  printBoard(board);
            }
      }   
       close(web_server_socket);
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
            printf("It's a Tie\n");
      } else if (state == -1) {
            printf("You Lost!\n");
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
            if (((0 <= coords[0] && coords[0] <= 2) && (0 <= coords[1] && coords[1] <= 2 ))
                                                    && board[coords[0]][coords[1]] == ' ') { 
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
                  printf("                %c | %c | %c \n", board[i][0], board[i][1], board[i][2]);
                  if (i < 2) {
                        printf("               -----------\n");
                  } 
            }
      }
}

/* 
void help() {
   // printf("\nUse number 1-9 to indicate the grid position.\n");
   printf("%s1%s : to do the next move in tic tac toe\n", CYN, NORM);
   // printf("%sc%s : to chat to the other player\n", CYN, NORM);
   // printf("%sp%s : print the board\n\n", CYN, NORM);
}
*/

/*
** a simple web browser
** compile: gcc -o client client-thread-main-2020.c client-thread-2020.c
** run: client freebsd().cs.scranton.edu 17400 c*.h
**
** gcc -o client client-thread-main-2021.c client-thread-2021.c && ./client freebsd1.cs.scranton.edu 17400 c*.h
**
** the server is supposed to start on port 4000 and
** make sure the server has been started before this.
**
** Basic steps for client to communicate with a servr
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

//just some colors :)

typedef struct TICTACTOE {
   char board[3][3];
   char player;
   int rounds;
   int gameOver;
} Game;

typedef struct SOCKETINFO {
   int socket_id;
   int port_list[2];
   Game *currGame;
   char allPlayers[2];
} socket_info;

void callPrintBoard(int sock);
void newMove(int sock);

void inputMove(int sock);
void printBoard(char board[3][3]);
void receiveBoard(char board[3][3], int sock);
int checkIfWin(int sock);

// void help();

int main(int argc, char *argv[]) {
   int web_server_socket;  

   char board[3][3];
   
   if (argc != 4) {
      printf("usage: client HOST HTTPORT webpage\n");
      exit(1);
   }

   if ((web_server_socket = get_server_connection(argv[1], argv[2])) == -1) {
      printf("connection error\n");
      exit(1);
   }

   int op;

   printf("1: send a new move, 2: print current board 3: quit\n");
   scanf("%d", &op);
   while (op != 3) {  // can change it to read in chars
      if (op == 1) {
         newMove(web_server_socket);
      } else {
         printf("1: send a new move, 2: print current board 3: quit\n");
         printf("Invalid opcode\n");
      }
      printf("1: send a new move, 2: print current board 3: quit\n");
      scanf("%d", &op);
   }       
   close(web_server_socket);
}
/* make sure not to remove the first send statement, it sends the operation */ 
void newMove (int sock) {
   int canMove = 0;

   send(sock, &TYPE, sizeof(int), 0);
   
   recv(sock, &canMove, sizeof(int), 0);
   if (canMove) {
      inputMove(sock);
   } else {
      printf("%sIt isnt your move!%s\n", RED, NORM);
   }
}

void inputMove(int sock){
   int invalid = 1;
   int nBytes = 0;
   char board[3][3];
   int coords[2];

   receiveBoard(board, sock);
   printBoard(board);

   while (invalid) {
      printf("Format: x y\n");
      scanf("%d%d", &coords[0], &coords[1]);
      if (((0 <= coords[0] && coords[0] <= 2) || (0 <= coords[1] && coords[1] <= 2 )) && board[coords[0]][coords[1]] == ' ') { //
         send(sock, &coords[0], sizeof(int), 0);
         send(sock, &coords[1], sizeof(int), 0);
         printf("%sSent Move!\n%s", CYN, NORM);
         invalid = 0;
      } else {
         printf("%sInvalid placement!%s\n", RED, NORM);
      }
   }
   invalid = checkIfWin(sock);
   if (invalid) {
      printf("You Win!!!\n");
   } 
}

int checkIfWin(int sock) {
   int result = 0;
   recv(sock, &result, sizeof(int), 0);
   return result;
}

/*
void callPrintBoard(int sock) {
   char board[3][3];
   int type = 2;
   send(sock, &type, sizeof(int), 0);
   receiveBoard(board, sock);
   printBoard(board);
} */

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
/* dont change */
void receiveBoard(char board[3][3], int sock) {
   for(int i = 0; i <= 2; i++) {
      for(int j = 0; j <=2; j++) {
         recv(sock, &board[i][j], sizeof(char), 0);
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

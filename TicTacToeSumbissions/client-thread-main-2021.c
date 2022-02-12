/*
** a simple web browser
** compile: gcc -o client client-thread-main-2020.c client-thread-2020.c
** run: client freebsd().cs.scranton.edu 17400 c*.h
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
} socket_info;

void compose_http_request(char *http_request, char *filename);
void web_browser(int web_server_socket, char *http_request);
void compose_greeting(char *http_request); 
void send_greeting(int web_server_socket, char * greeting);
//void *message (void *socket);

void callPrintBoard();
void newMove(int sock);
void validNextMove();
void help();
void printBoard(char board[3][3]);

int main(int argc, char *argv[]) {
   int web_server_socket;  
   char http_request[BUFFERSIZE];
   char greeting[BUFFERSIZE];
   
   if (argc != 4) {
      printf("usage: client HOST HTTPORT webpage\n");
      exit(1);
   }

   if ((web_server_socket = get_server_connection(argv[1], argv[2])) == -1) {
      printf("connection error\n");
      exit(1);
   }
   int op = 1;
   printf("1: send URL, 2: send move, 3: send message, 0: quit\n");
   scanf("%d", &op);
   while (op > 0) {  // can change it to read in chars
      memset(http_request, '\0', BUFFERSIZE);
      memset(greeting, '\0' , BUFFERSIZE);
      if (op == 1) {
         newMove(web_server_socket);
      } else if (op == 2) {
         callPrintBoard();
      } else if (op == 3) {
         //idk whatever we need
      } else {
         printf("1: send URL, 2: send move, 0: quit\n");
         printf("Invalid opcode\n");
      }
       printf("1: send URL, 2: send move, 0: quit\n");
       scanf("%d", &op);
   }       
   close(web_server_socket);
}

void newMove (int sock) {
   int type = 2;
   //send the type of 
   printf("sending type...\n");
   send(sock, &type, sizeof(int), 0);
   printf("type has been sent...\n");

   validNextMove(sock);
}

void validNextMove(int sock){
   int invalid = 1;
   int nBytes = 0;
   char board[3][3];
   //char player;
   int coords[2];

   for(int i = 0; i <= 2; i++) {
      for(int j = 0; j <=2; j++) {
         recv(sock, &board[i][j], sizeof(char), 0);
      }
   }

   printBoard(board);
   while (invalid) {
      printf("Format: (x, y)\n");
      scanf("%d%d", &coords[0], &coords[1]);
      //printf("(%d, %d)\n", coords[0], coords[1]);
      if (((0 <= coords[0] && coords[0] <= 2) || (0 <= coords[1] && coords[1] <= 2 )) && board[coords[0]][coords[1]] == ' ') { //
         send(sock, &coords[0], sizeof(int), 0);
         send(sock, &coords[1], sizeof(int), 0);
         printf("%sSent Move!\n%s", CYN, NORM);
         invalid = 0;
      } else {
         printf("%sInvalid placement!%s\n", RED, NORM);
      }
   }
}

void help() {
   printf("\nUse number 1-9 to indicate the grid position.\n");
   printf("%sn%s : to do the next move in tic tac toe\n", CYN, NORM);
   printf("%sc%s : to chat to the other player\n", CYN, NORM);
   printf("%sp%s : print the board\n\n", CYN, NORM);
}

/*
for printing the board

- send a send() to the server, asking for the board
- recv() the board back from the server
- format the incoming board into printf(), (kind of written already)
*/

void callPrintBoard() {
   //TODO
}

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

/*void  compose_greeting(char *http_request) {
    strcpy(http_request, "Hello World!");
}

void send_greeting(int http_conn, char *greeting) {
   int type = 3;
   int nBytes = strlen(greeting) + 1;
   char helloworld[BUFFERSIZE];
   printf("msg: %s\n", greeting);
   printf("param %d - %d\n", type, nBytes);
   send(http_conn, &type, sizeof(int), 0);
   send(http_conn, &nBytes, sizeof(int), 0);
   send(http_conn, greeting, nBytes, 0);

   recv(http_conn, &nBytes, sizeof(int), 0);
   recv(http_conn, helloworld, nBytes, 0); // recieves the message back from the server
   printf("Server sent: %s\n", helloworld);
}

void  compose_http_request(char *http_request, char *filename) {
   strcpy(http_request, "GET /");
   strcpy(&http_request[5], filename);
   strcpy(&http_request[5+strlen(filename)], " ");
}

void web_browser(int http_conn, char *http_request) {
   int numbytes = 0;
   char buf[256]; // step 4.1: send the HTTP request
   int type = 1;
   int nBytes = strlen(http_request) + 1;
   send(http_conn, &type, sizeof(int), 0);
   send(http_conn, &nBytes, sizeof(int), 0);
   send(http_conn, http_request, nBytes, 0);

   // step 4.2: receive message from server
   recv(http_conn, &nBytes, sizeof(int), 0);
   while (nBytes > 0) {
      numbytes = recv(http_conn, buf, nBytes,  0);
      // step 4.3: the received may not end with a '\0' 
      buf[numbytes] = '\0';
      printf("%s",buf);
      recv(http_conn, &nBytes, sizeof(int), 0);
   }
}
*/




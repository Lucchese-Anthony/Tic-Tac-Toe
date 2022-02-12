/*
** A simple Server
   This program runs together with client-thread-main-2021.c as one system.

   To demo the whole system, you must:
   1. compile the server program:
        gcc -lpthread -o server server-thread-2021.c server-thread-main-2021.c
   2. run the program: server 41000 &
   3. compile the cliient program:
        gcc -o client client-thread-2021.c client-thread-main-2021.c
   4. run the client on another machine: client HOST 41000 webpage
         replacing HOST and HTTPPORT with the host the server runs
         on and the port # the server runs at.

   FOR STUDNETS:
     If you want to try this demo, you MUST use the port # assigned
     to you by your instructor. Since each http port can be associated
     to only one process (or one server), if another student tries to
     run the server with the default port, it will be rejected.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netdb.h>
#include <pthread.h>
#include "server-thread-2021.h"

#define HOST "freebsd1.cs.scranton.edu"
#define BACKLOG 10
#define BUFFERSIZE 256

#define NORM "\x1B[0m"
#define RED "\x1B[31m"
#define CYN "\x1B[36m"

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

//Bi methods

void *start_subserver(void *reply_sock_fd);
void *subserver(void * reply_sock_fd_as_ptr);
void handle_http_request(int reply_sock_fd);
void handle_greeting(int reply_sock_fd);

// Anthony methods

void callPrintBoard();
socket_info *iterateGame (socket_info *sockInfo);
Game *createGame();
int checkWinCondition(char board[3][3], char player);
int checkIfWin(char player, char board[3][3], int possibleWins[8][3][2], int i);
void nextMove (socket_info *sockInfo);
void validNextMove(socket_info *sockInfo);
void setBoard(Game *currGame);
void printBoard(char board[3][3]);

int main(int argc, char *argv[]) {
   int http_sock_fd;			// http server socket
   int reply_sock_fd;  	                // client connection 
   Game *currentGame = createGame();
   if (argc != 2) {
      printf("Run: program port#\n");
      return 1;
   }
   http_sock_fd = start_server(HOST, argv[1], BACKLOG);
   if (http_sock_fd ==-1) {
      printf("start server error\n");
      exit(1);
   }
   while(1) {
      if ((reply_sock_fd = accept_client(http_sock_fd)) == -1) {
         continue;
      }
      pthread_t thread_id;
      socket_info *sockInfo = (socket_info*)malloc(sizeof(socket_info));
      sockInfo -> socket_id = reply_sock_fd;
      sockInfo->currGame = currentGame; // init above

      if( pthread_create( &thread_id , NULL ,  start_client, (void*) sockInfo) < 0) {
         perror("could not create thread");
         return 1;
      }
   }
} 

void* start_client(void *incomingInfo) {
   int type = 0;
   socket_info *sockInfo = (void*)incomingInfo;
   int read_count = recv(sockInfo->socket_id, &type, sizeof(int), 0);
   while (type > 0 && read_count != 0) {
      if (type == 1) {
         iterateGame(sockInfo);
      } else if (type == 2) { // new game and iterate game
         callPrintBoard();
      } else if (type == 3){
         //handle_greeting(sockInfo->socket_id);
      }
      read_count = recv(sockInfo->socket_id, &type, sizeof(int), 0);
   }
   close(sockInfo->socket_id);
   printf("Client closed the connection\n");
}

void callPrintBoard() {
   //TODO 
}

socket_info *iterateGame (socket_info *sockInfo) {
	if (sockInfo->currGame == NULL) {
		sockInfo->currGame = createGame(sockInfo);
      printf("New game!\n");
   }
   nextMove(sockInfo);
	return sockInfo;
}

void nextMove (socket_info *sockInfo){
	validNextMove(sockInfo);
	printBoard(sockInfo->currGame->board);
	sockInfo->currGame->gameOver = checkWinCondition(sockInfo->currGame->board, sockInfo->currGame->player);
	if (sockInfo->currGame->gameOver){
      printf("Player %c Wins!", sockInfo->currGame->player);
      sockInfo->currGame == NULL;
	} else {
		sockInfo->currGame->player = (sockInfo->currGame->player == 'X') ? 'O' : 'X';
		sockInfo->currGame->rounds++;
      printf("Next Player: %c || Next Round: %d\n", sockInfo->currGame->player, sockInfo->currGame->rounds);
   }
}

void validNextMove(socket_info *sockInfo){
	int coords[2]; //init location of board u wanna use
   int nBytes = 0; //socket stuff
   for(int i = 0; i <= 2; i++) {
      for(int j = 0; j <=2; j++) {
         send(sockInfo->socket_id, &sockInfo->currGame->board[i][j], sizeof(char), 0);
      }
   }
   recv(sockInfo->socket_id, &coords[0], sizeof(int), 0);
   recv(sockInfo->socket_id, &coords[1], sizeof(int), 0);
   // confirmed that the spot is valid
   sockInfo->currGame->board[ coords[0] ][ coords[1] ] = sockInfo->currGame->player;
   printf("(%d, %d)\n", coords[0], coords[1]);
}

Game *createGame() {
	Game *newGame = (Game*)malloc(sizeof(Game));
	setBoard(newGame);
	newGame->player = 'X';
	newGame->rounds = 1;
	newGame->gameOver = 0;

	return newGame;
}

void setBoard(Game *currGame) {
   for(int i = 0; i <= 2; i++){
      currGame->board[i][0] = ' ';
      currGame->board[i][1] = ' ';
      currGame->board[i][2] = ' ';
   }
}

int checkIfWin(char player, char board[3][3], int possibleWins[8][3][2], int i) {
 	return (board[possibleWins[i][0][0]][possibleWins[i][0][1]] == player && board[possibleWins[i][1][0]][possibleWins[i][1][1]]== player 
      && board[possibleWins[i][2][0]][possibleWins[i][2][1]] == player) ? 1 : 0; 
}

int checkWinCondition(char board[3][3], char player) {
	int possibleWins[8][3][2] = {{{0,0}, {0,1} , {0,2}}, {{1,0},{1,1},{1,2}}, {{2,0},{2,1},{2,2}} , {{0,0},{1,0},{2,0}}, {{0,1},{1,1},{2,1}}, 
      {{0,2},{1,2},{2,2}}, {{0,0},{1,1},{2,2}}, {{0,2},{1,1},{2,0}}};
	int result = 0;
	for (int i = 0; i <= 7; i++) {
		if (checkIfWin(player, board, possibleWins, i)) {
			result = 1;
			break;
		}
	}
	return result;
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
/*
      Group 4
      Henry Lembo, Anthony Lucchese, Prisco Pocius
      Team Assignemnt 1 Server

      A simple server for a tic tac toe game
      This program runs together with Client-Assignment-1.c as one system.

      To demo the whole system, you must:
      compile and run: gcc -lpthread -o server server-thread-2021.c
                       Server-Assignment-1.c && ./server 17400
      compile client: gcc -o client client-thread-2021.c Client-Assignment-1.c
      run client: ./client freebsd1.cs.scranton.edu 17400 c*.h
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

//text colors
#define NORM "\x1B[0m"
#define RED "\x1B[31m"
#define CYN "\x1B[36m"

//game conditions
#define STARTGAME 0
#define WIN 1
#define TIE 10
#define LOSE -1

typedef struct TICTACTOE {
      char board[3][3];
      char player;
      int rounds;
      int gameOver;
} Game;

Game *createGame();

//winning state methods
int checkWinCondition(Game *currGame);
int checkIfWin(char player, char board[3][3], int possibleWins[8][3][2], int i);

//board state methods
void setBoard(Game *currGame);
void printBoard(char board[3][3]);
void sendBoard(int sock, Game *currGame);

//other methods
void playerMove(Game *currGame, int currPlayer, int otherPlayer, char userChar);
void receiveNewMove(Game *currGame, int sock, char userChar);
void sendState(Game *currGame, int currPlayer, int otherPlayer, char userChar);
int getGameState(Game *currGame, int playerSock, char userChar);

int main(int argc, char *argv[]) {
      int http_sock_fd;    // http server socket
      if (argc != 2) {
            printf("Run: program port#\n");
            return 1;
      }
      http_sock_fd = start_server(HOST, argv[1], BACKLOG);
      if (http_sock_fd ==-1) {
            printf("start server error\n");
            exit(1);
      }
      while(1){
            int player1, player2;
            player1 = accept_client(http_sock_fd);
            player2 = accept_client(http_sock_fd);
            printf("Player 2 connected: %d\n", player2);
            Game *currentGame = createGame();
            int start = 0;
            send(player1, &start, sizeof(int), 0); 
            while (currentGame->gameOver == 0) {
                  playerMove(currentGame, player1, player2, 'X');
                  playerMove(currentGame, player2, player1, 'O');
            }
      }
      //close(http_sock_fd);
} 

void playerMove(Game *currGame, int currPlayer, int otherPlayer, char userChar) {
      if (currGame->gameOver == 0) {
            sendBoard(currPlayer, currGame);
            printBoard(currGame->board);
            receiveNewMove(currGame, currPlayer, currGame->player); // locally this will be saved as
            currGame->gameOver = checkWinCondition(currGame);
            sendState(currGame, currPlayer, otherPlayer, userChar);
      }
}

void receiveNewMove(Game *currGame, int sock, char userChar) {
      int coords[2] = {0 , 0};
      send(sock, &userChar, sizeof(char), 0);
      recv(sock, &coords[0], sizeof(int), 0);
      recv(sock, &coords[1], sizeof(int), 0);
      printf("\n(%d, %d)\n", coords[0], coords[1]);
      currGame->board[coords[0]][coords[1]] = userChar;
}

void sendState(Game *currGame, int currPlayer, int otherPlayer, char userChar) {
      char otherUserChar = (userChar == 'X') ? 'O' : 'X';
      if (currGame->gameOver) {
            int p1state = getGameState(currGame, currPlayer, userChar);
            int p2state = getGameState(currGame, otherPlayer, otherUserChar);
            printf("sending player 1 state: %d", p1state);
            send(currPlayer, &p1state, sizeof(int), 0);
            sendBoard(currPlayer, currGame);
            printf("sending player 2 state: %d", p2state);
            send(otherPlayer, &p2state, sizeof(int), 0);
            sendBoard(otherPlayer, currGame);
      } else {
            currGame->player = otherUserChar;
            currGame->rounds++;
            int gameState = getGameState(currGame, otherPlayer, otherUserChar);
            send(otherPlayer, &gameState, sizeof(int), 0);	
      }
}

int getGameState(Game *currGame, int playerSock, char userChar) {
      printf("current Player: %c | userChar: %c | rounds: %d\n", 
                  currGame->player, userChar, currGame->rounds);
      int currGameState = 0;
      if (currGame->rounds > 9) {
            currGameState = TIE;
      } else if (currGame->gameOver) {
            if (currGame->player != userChar) {
                  currGameState = LOSE;
            } else {
                  currGameState = WIN;
            }
      }
      return currGameState;
}

/* dont change */
Game *createGame() {
      printf("New game has been created...\n");
      Game *newGame = (Game*)malloc(sizeof(Game));
      setBoard(newGame);
      newGame->player = 'X';
      newGame->rounds = 1;
      newGame->gameOver = 0;
      return newGame;
}

/* dont change */
void setBoard(Game *currGame) {
      for(int i = 0; i <= 2; i++){
            currGame->board[i][0] = ' ';
            currGame->board[i][1] = ' ';
            currGame->board[i][2] = ' ';
      }
}

/* dont change */
int checkIfWin(char player, char board[3][3], int possibleWins[8][3][2], int i) {
      return (board[possibleWins[i][0][0]][possibleWins[i][0][1]] == player && 
              board[possibleWins[i][1][0]][possibleWins[i][1][1]] == player &&
              board[possibleWins[i][2][0]][possibleWins[i][2][1]] == player) ? 1 : 0; 
}

/* dont change, unless different output */
int checkWinCondition(Game *currGame) {
      int possibleWins[8][3][2] = {{{0,0}, {0,1} , {0,2}}, {{1,0},{1,1},{1,2}}, 
                                  {{2,0},{2,1},{2,2}}, {{0,0},{1,0},{2,0}}, 
                                  {{0,1},{1,1},{2,1}}, {{0,2},{1,2},{2,2}}, 
                                  {{0,0},{1,1},{2,2}}, {{0,2},{1,1},{2,0}}};
      int result = 0;
      if (currGame->rounds > 9) {
            return 1;
      }
      for (int i = 0; i <= 7; i++) {
            if (checkIfWin(currGame->player, currGame->board, possibleWins, i)) {
                  return 1;
            }
      }
      return result;
}

void sendBoard(int sock, Game *currGame) {
      for(int i = 0; i <= 2; i++) {
            for(int j = 0; j <=2; j++) {
                  send(sock, &currGame->board[i][j], sizeof(char), 0);
            }
      }
}

/* dont change */
void printBoard(char board[3][3]) {
      if (board == NULL) {
            printf("There is no current game!\n");
      } else {
            printf("Current Board: \n");
            for(int i = 0; i <= 2; i++ ){
                  printf("               %c | %c | %c \n", 
                  board[i][0], board[i][1], board[i][2]);
                  if (i < 2) {
                        printf("              -----------\n");
                  }
            }
      }
}

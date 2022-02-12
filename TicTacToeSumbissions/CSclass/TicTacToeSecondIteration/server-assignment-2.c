/*
	A simple Server
	This program runs together with client-thread-main-2021.c as one system.

	To demo the whole system, you must:

	compile and run: gcc -lpthread -o server server-thread-2021.c server-assignment-2.c && ./server 17400
	compile client: gcc -o client client-thread-2021.c client-Assignment-1.c
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

#define NORM "\x1B[0m"
#define RED "\x1B[31m"
#define CYN "\x1B[36m"

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

typedef struct SCOREBOARD {
	char name[21];
	int wins;
	int losses;
	int ties;
} Scoreboard;

typedef struct GAME_THREAD {
	int player1;
	int player2;
	Game *currGame;
	int player1_ID; //Player Game ID
	int player2_ID; //Player Game ID
	Scoreboard *sb;
} Thread;
/*
 //created a struct that hold the struct scoreboard inside of it as a record
typedef struct RECORD{
	 struct Scoreboard Score_board;
}record;
*/
// Anthony methods
Game *createGame();
void *subserver(void* gameInfo);

// winning state methods
int checkWinCondition(Game *currGame);
int checkIfWin(char player, char board[3][3], int possibleWins[8][3][2], int i);

// board state methods
void setBoard(Game *currGame);
void printBoard(char board[3][3]);
void sendBoard(int sock, Game *currGame);

void playerMove(int userID, int currPlayer, int otherPlayer, char userChar, Thread *gameInfo);
void receiveNewMove(Game *currGame, int sock, char userChar);
void sendState(int userID, int currPlayer, int otherPlayer, char userChar, Thread *gameInfo);
int getGameState(int playerSock, char userChar, int playerID, Thread *gameInfo);

int findUserScoreboard(int sock, Scoreboard *sb);
int checkIfInScoreboard(char username[20], Scoreboard *sb);
void initScoreboard(Scoreboard *sb);
void endGameScoreBoard(Thread *gameInfo);
/*
//binaary file methods
record *readRecordAt(int fd, int index);
int writeRecordAt(int fd, record *Score_board, int index)
int initRecord(int argc, record *record)
*/
int main(int argc, char *argv[]) {
	int http_sock_fd;    // http server socket
	Scoreboard *sb = (Scoreboard*)malloc(sizeof(Scoreboard) * 10);
	initScoreboard(sb);
	int numOfGames = 1;

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
		pthread_t newGame;
		Thread *gameInfo = (Thread*)malloc(sizeof(Thread));
		gameInfo->sb = sb;
		gameInfo->player1 = accept_client(http_sock_fd);
		gameInfo->player1_ID = findUserScoreboard(gameInfo->player1, sb);
		printf("Waiting for player 2...\n");
		gameInfo->player2 = accept_client(http_sock_fd);
		gameInfo->player2_ID = findUserScoreboard(gameInfo->player2, sb);
 
		pthread_create(&newGame, NULL, subserver, (void*)gameInfo);

		char player2name[21];
		char player1name[21];
		strcpy(player2name, gameInfo->sb[gameInfo->player2_ID].name);
		send(gameInfo->player1, &player2name, sizeof(player2name), 0);

		strcpy(player1name, gameInfo->sb[gameInfo->player1_ID].name);
		send(gameInfo->player2, &player1name, sizeof(player1name), 0);

		printf("Game #%d created!\n", numOfGames++);
	}
	//to close, CTRL + C
}

void initScoreboard(Scoreboard *sb) {
	for(int i = 0; i <= 9; i++) {
		strcpy(sb[i].name, "");
		sb[i].wins = 0;
		sb[i].losses = 0;
		sb[i].ties = 0;
	}
}

Game *createGame() {
	Game *newGame = (Game*)malloc(sizeof(Game));
	setBoard(newGame);
	newGame->player = 'X';
	newGame->rounds = 1;
	newGame->gameOver = 0;

	return newGame;
}

void *subserver(void* incomingInfo) {
	Thread *gameInfo = incomingInfo;
	gameInfo->currGame = createGame();
	int start = 0;
	send(gameInfo->player1, &start, sizeof(int), 0);
	while (gameInfo->currGame->gameOver == 0) {
		playerMove(gameInfo->player1_ID, gameInfo->player1, gameInfo->player2, 'X', gameInfo);
		if (gameInfo->currGame->gameOver == 0){		
			playerMove(gameInfo->player2_ID, gameInfo->player2, gameInfo->player1, 'O', gameInfo);
		}
	}
}

void playerMove(int userID, int currPlayer, int otherPlayer, char userChar, Thread *gameInfo) {
	if (gameInfo->currGame->gameOver == 0) {
		sendBoard(currPlayer, gameInfo->currGame);
		printBoard(gameInfo->currGame->board);
		receiveNewMove(gameInfo->currGame, currPlayer, gameInfo->currGame->player);
		gameInfo->currGame->gameOver = checkWinCondition(gameInfo->currGame);
		sendState(userID, currPlayer, otherPlayer, userChar, gameInfo);
	}
}

void receiveNewMove(Game *currGame, int sock, char userChar) {
	int coords[2] = {0 , 0};
	send(sock, &userChar, sizeof(char), 0);
	recv(sock, &coords[0], sizeof(int), 0);
	recv(sock, &coords[1], sizeof(int), 0);
	printf("\nuser inputted coords: (%d, %d)\n", coords[0], coords[1]);
	currGame->board[coords[0]][coords[1]] = userChar;
}

void sendState(int userID, int currPlayer, int otherPlayer, char userChar, Thread *gameInfo) {
	char otherUserChar = (userChar == 'X') ? 'O' : 'X';
	int p1ID = gameInfo->player1_ID;
	int p2ID = gameInfo->player2_ID;
	int otherUserID = ( p1ID == userID) ? p2ID : p1ID;
	if (gameInfo->currGame->gameOver) {
		int p1state = getGameState(currPlayer, userChar, userID, gameInfo);
		int p2state = getGameState(otherPlayer, otherUserChar, otherUserID, gameInfo);
		printf("sending player 1 state: %d", p1state);
		send(currPlayer, &p1state, sizeof(int), 0);
		sendBoard(currPlayer, gameInfo->currGame);

		printf("sending player 2 state: %d", p2state);
		send(otherPlayer, &p2state, sizeof(int), 0);
		sendBoard(otherPlayer, gameInfo->currGame);
		endGameScoreBoard(gameInfo);
	} else {
		gameInfo->currGame->rounds++;
		int gameState = getGameState(otherPlayer, otherUserChar, otherUserID, gameInfo);
		
		if(gameState != 0){
			int currState = getGameState(currPlayer, userChar, userID, gameInfo);
			gameInfo->currGame->gameOver = 1;
	//		sendState(currGame, currPlayer, otherPlayer, userChar, gameInfo);
			send(otherPlayer, &gameState, sizeof(int), 0);
			sendBoard(otherPlayer, gameInfo->currGame);

			send(currPlayer, &currState, sizeof(int), 0);
			sendBoard(currPlayer, gameInfo->currGame);

			endGameScoreBoard(gameInfo);
		}else {
			gameInfo->currGame->player = otherUserChar;
			send(otherPlayer, &gameState, sizeof(int), 0);
		}

	}
}

int getGameState(int playerSock, char userChar, int playerID, Thread *gameInfo){
	printf("current Player: %c | userChar: %c | rounds: %d\n", 
	gameInfo->currGame->player, userChar, gameInfo->currGame->rounds);
	Scoreboard *sb;
	sb = &gameInfo->sb[playerID];
	int currGameState = 0;
	if (gameInfo->currGame->rounds > 9) {
		sb->ties++;
		currGameState = TIE;
	} else if (gameInfo->currGame->gameOver) {
		if (gameInfo->currGame->player != userChar) {
			sb->losses++;
			currGameState = LOSE;
		} else {
			sb->wins++;
			currGameState = WIN;
		}
	}
	return currGameState;
}

/* dont change */
void setBoard(Game *currGame) {
   for(int i = 0; i <= 2; i++){
	  currGame->board[i][0] = ' ';
	  currGame->board[i][1] = ' '; // change to empty spaces
	  currGame->board[i][2] = ' ';
   }
}
/* dont change */
int checkIfWin(char player, char board[3][3], int possibleWins[8][3][2], int i) {
 	return (board[possibleWins[i][0][0]][possibleWins[i][0][1]] == player &&
	 		board[possibleWins[i][1][0]][possibleWins[i][1][1]]== player
	  		&& board[possibleWins[i][2][0]][possibleWins[i][2][1]] == player) ? 1 : 0;
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

int findUserScoreboard(int sock, Scoreboard *sb){ 
	//return playerID we want to set
	int buffer = 0;
	char username[21];
	int playerID;

	recv(sock, &username, sizeof(username), 0);
	playerID = checkIfInScoreboard(username,sb);
	return playerID;
}

int checkIfInScoreboard(char username[21], Scoreboard *sb){
	int i = 0;
	while(strcmp(sb[i].name, username) != 0){
		if(strcmp(sb[i].name,"") == 0){
			strcpy(sb[i].name, username);
			printf("Created New User: %s\nUser ID: %d\n", username, i);
		}else{
			i++;
		}
	}
	return i; 

}
//use this method it is not used currently
void endGameScoreBoard(Thread *gameInfo){
	char printscoreboard[100];
	Scoreboard sb = gameInfo->sb[gameInfo->player1_ID];
	Scoreboard sb2 = gameInfo->sb[gameInfo->player2_ID];
	sprintf(printscoreboard, "%s: %dW/%dL/%dT - %s: %dW/%dL/%dT", 
	sb.name, sb.wins, sb.losses , sb.ties, sb2.name, sb2.wins, sb2.losses,
	sb2.ties);
	send(gameInfo->player1, &printscoreboard, sizeof(printscoreboard), 0);
	send(gameInfo->player2, &printscoreboard, sizeof(printscoreboard), 0);
}
/*
record *readRecordAt(int fd, int index){
	record *record = (record *)malloc(sizeof(record));
	if(lseek(fd, index *sizeOf(record), SEEK_SET) < 0){
		return NULL;
	}

	if(red(fd, record, sizeof(record))<= 0){
		return NULL;
	}
	return record;
}

int writeRecordAt(int fd, record *record, int index){
	if(lseek(fd, index *sizeOf(record), SEEK_SET) < 0){
		return -1;
	}

	if(wrtie(fd, record, sizeof(record))<= 0){
		return -1;
	}
	return 0;
}

int initRecord(int argc, record *record){
	record *record;
	int fd = -1;
	if(argc !=2){
		printf("usage: read-binary-data binary-file\n");
		exit(1)
	}

	fd = open(argc[1], O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
	char username[40];
	record = (record *)malloc(sizeof(record));
	//looping mechanism needs to be implemented here to go throught each record

	free(record);
	i = 0;
	while((record = readRecordAt(fd, i)) != NULL){
			//Print the record for the user
		}
	int noRecord = 5;
	record = readRecordAt(fd, noRecord);
	if (record == NULL){
		printf("no such record!\n");
	}else{
		//record would be printed here
	}
	close(fd);
	)
}

*/
/* dont change */
void printBoard(char board[3][3]) {
	if (board == NULL) {
		printf("There is no current game!\n");
	} else {
		printf("Current Board: \n");
		for(int i = 0; i <= 2; i++ ){
			printf("		 %c | %c | %c \n", board[i][0], board[i][1], board[i][2]);
			if (i < 2) {
				printf("		-----------\n");
			}
		}
	}
}

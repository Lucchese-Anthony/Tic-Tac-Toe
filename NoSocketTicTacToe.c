#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <struct.h>

static char* NORM = "\x1B[0m";
static char* RED = "\x1B[31m";
static char* CYN = "\x1B[36m";

typedef struct TICTACTOE {
	char board[9];
	char player;
	int rounds;
	int gameOver;
} Game;

int checkIfWin(char player, char *board, int possibleWins[8][3], int i) {
 	return (board[possibleWins[i][0]] == player && board[possibleWins[i][1]] == player && board[possibleWins[i][2]] == player) ? 1 : 0; 
}

int checkWinCondition(char *board, char player) {
	int possibleWins[8][3] = {{0,1,2}, {3,4,5}, {6,7,8} , {0,3,6}, {1,4,7}, {2,5,8}, {0,4,8}, {2,4,6}};
	int result = 0;
	for (int i = 0; i <= 7; i++) {
		if (checkIfWin(player, board, possibleWins, i)) {
			result = 1;
			break;
		}
	}
	return result;
}

void printBoard(Game *currGame) {
	if (currGame == NULL) {
		printf("There is no current game!\n");
	} else {
		printf("\n");
		printf("		 %c | %c | %c \n		-----------\n", currGame->board[0], currGame->board[1], currGame->board[2]); 
		printf("		 %c | %c | %c \n		-----------\n", currGame->board[3], currGame->board[4], currGame->board[5]);
		printf("		 %c | %c | %c \n",    		             currGame->board[6], currGame->board[7], currGame->board[8]);
	}
}

Game *createGame() {
	Game *newGame = (Game*)malloc(sizeof(Game));
	memcpy(newGame->board,"         ", 9);
	newGame->player = 'X';
	newGame->rounds = 1;
	newGame->gameOver = 0;

	return newGame;
}

void validNextMove(Game *currGame){
	int boardLoc;
	int invalid = 1;
	while (invalid) {
		printf("Player %c: ", currGame->player);
		scanf("%d", &boardLoc);
		if (boardLoc <= 9 && boardLoc >=1 && currGame->board[boardLoc-1] == ' ') {
			currGame->board[boardLoc-1] = currGame->player;
			invalid = 0;
		} else {
			printf("%sInvalid placement!%s\n", RED, NORM);
		}
	}
}

void nextMove (Game *currGame){
	validNextMove(currGame);
	printBoard(currGame);
	currGame->gameOver = checkWinCondition(currGame->board, currGame->player);
	if (currGame->gameOver == 1){
		currGame->gameOver = 1;
	} else {
		currGame->player = (currGame->player == 'X') ? 'O' : 'X';
		currGame->rounds++;
	}
}

void chat(/*char player*/){
	char message[100];
	scanf("%s", message);
	printf("> %s\n", message);
	// printf("%c > %s", player, message);
}

void help() {
	printf("\n%sn%s : to do the next move in tic tac toe\n", CYN, NORM);
	printf("%sc%s : to chat to the other player\n", CYN, NORM);
	printf("%scp%s : print the board\n\n", CYN, NORM);
}

Game *iterateGame (Game *currGame) {
	char input = 'n';
	if (currGame == NULL) {
		currGame = createGame();
		nextMove(currGame);
	} else {
		nextMove(currGame);
	}
	return currGame;
}

int main() {
	int wantNewGame = 1;
	while(wantNewGame) {
		char input;
		Game *currGame = NULL;
		printf("%sWhat action would you like to perform? (h for help)%s: ", CYN, NORM);
		while(input != 'z') {
			scanf("%c", &input);
			if (input == 'c') { // chat
				// chat(currGame->player); use this when you can use sockets
				chat();
			} else if (input == 'h') {
				help();
			} else if (input == 'n') {
				currGame = iterateGame(currGame);
				if (currGame->gameOver == 1) {
					printf("PLayer %s%c%s has won!\n", CYN, currGame->player, NORM);
					free(currGame);
					input = 'z';
				} else if (currGame->rounds > 9) {
					printf("Its a tie!");
					free(currGame);
					input = 'z';
		}
			} else if (input == 'p') {
				printBoard(currGame);
			}
		}
		printf("Do you want to play another game (0 / 1): ");
		scanf("%d", &wantNewGame);
	}
	return 0;
}

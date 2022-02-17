/*
    Group 4: Henry Lembo, Anthony Lucchese, Prisco Pocius
    A simple Server
    This program runs together with client-thread-main-2021.c as one system.

    To demo the whole system, you must:

    compile and run: gcc -lpthread -o server server-thread-2021.c server-assignment-3.c && ./server 17400
    compile client: gcc -o client client-thread-2021.c client-assignment-3.c
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
#define _OPEN_THREADS

pthread_mutex_t userInfoLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sbLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t incScoreLock = PTHREAD_MUTEX_INITIALIZER;

typedef struct PLAINTEXT {
    char user[33];
    char pass[33];
} passBin;

typedef struct SCORE {
    char name[33];	// name of user
    int wins;
    int losses;
    int ties;
} Score;

typedef struct TICTACTOE {
    char board[3][3];
    char player;
    int rounds;
    int gameOver;
} Game;

typedef struct PLAYERINFO {
    int portNum;	// num to send to user
    int locInMem;	// location to search in the bin files
    Score *score;
} Player;

typedef struct GAME_THREAD {
    Game *currGame;
    Player *player1;
    Player *player2;
} Thread;

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

// player moving states
void playerMove(Player *curr, Player *next, char userChar, Thread *gameInfo);
void receiveNewMove(Game *currGame, int sock, char userChar);
void decideState(Player *p1, Player *p2, char userChar, Thread *gameInfo);
int getGameState(char userChar, Thread *gameInfo, Player *p1);
void sendUserState(char userChar, Thread *gameInfo, Player *p);

// user authentication
int verifyUser(Player *player);
Player *initPlayer();

// scoreboard files
void readStoredScore(Player *p);
void writeStoredScore(Player *p);
void endGameScoreBoard(Thread *gameInfo);


int main(int argc, char *argv[]) {
    int http_sock_fd;    // http server socket
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
        Thread *gameInfo = (Thread*)malloc(sizeof(Thread));
        gameInfo->player1 = initPlayer();	
        gameInfo->player2 = initPlayer();

        while (gameInfo->player1->locInMem == -1){
            gameInfo->player1->portNum = accept_client(http_sock_fd);		// accept the clients connection
            gameInfo->player1->locInMem = verifyUser(gameInfo->player1);	// authenticates their user/pass
        }
        printf("Waiting for player 2...\n");

        while (gameInfo->player2->locInMem == -1){
           gameInfo->player2->portNum = accept_client(http_sock_fd);		// accept the clients connection
           gameInfo->player2->locInMem = verifyUser(gameInfo->player2);	// authenticates their user/pass
        }
	// readStoredScore(gameInfo); // initialized here because both users must be connected to reach ehre

        pthread_t newGame;
        pthread_create(&newGame, NULL, subserver, (void*)gameInfo);			// creates the nth game thread

        char p2n[33];
        char p1n[33];
        strcpy(p2n, gameInfo->player2->score->name);
        send(gameInfo->player1->portNum, &p2n, sizeof(p2n), 0);// sends name to other player
        strcpy(p1n, gameInfo->player1->score->name);
        send(gameInfo->player2->portNum, &p1n, sizeof(p1n), 0);//sends name to other player

        printf("Game #%d created!\n", numOfGames++);			// # game that is created (not needed)
        printf("Your Name: %s | Opponent Name: %s \n", p1n, p2n);
    }
    //to close, CTRL + C
}

int verifyUser(Player *player) {
    passBin *info = (passBin*)malloc(sizeof(passBin));
    int sock = player->portNum;
    char username[33];
    char password[33];
    recv(sock, &username, sizeof(username), 0);
    recv(sock, &password, sizeof(password), 0);
    //printf("recieved:\n->%s\n->%s\n", username, password);
    int fd = -1;
    char *filename = "userinfo.bin";
    // lock mutex
    pthread_mutex_lock(&userInfoLock);
    fd = open(filename, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
    int connection = LOSE;
	
    int stillReading = -1;
    int i = 0;
    while(stillReading != 0) {
        lseek(fd, sizeof(passBin) * i, SEEK_SET);
        stillReading = read(fd, info, sizeof(passBin));
        if(strcmp(username, info->user) == 0) {
            if(strcmp(password, info->pass) == 0) {
                connection = WIN;
                send(sock, &connection, sizeof(int), 0); 
                printf("storing user/pass with location %d\n", i);
                strcpy(player->score->name, username);
                break;
            } else {
                printf("incorrect password! Disconnecting user...\n");
                send(sock, &connection, sizeof(int), 0); 
                i = -1;
                break;
                // client sent wrong pass, correct user 
            }
        }
        if (stillReading == 0){
            connection = WIN;
            strcpy(info->user, username);
            strcpy(info->pass, password); 
            strcpy(player->score->name, username);
            printf("Creating New User info with Username: <%s> and Password: <%s>\n", username, password);
            write(fd, info, sizeof(passBin));
            send(sock, &connection, sizeof(int), 0);
            break;
        }
    i++;
    }
    pthread_mutex_unlock(&userInfoLock);
    close(fd);
    free(info);
    return i;
}

Player *initPlayer() {
    Player *p = (Player*)malloc(sizeof(Player));
    Score *s = (Score*)malloc(sizeof(Score));
    p->score = s;
    p->locInMem = -1;	// 0 is a location
    p->portNum = -1;	// 0 is a port
    return p;
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
    send(gameInfo->player1->portNum, &start, sizeof(int), 0);

    Player *p1 = gameInfo->player1;
    Player *p2 = gameInfo->player2;

    while (gameInfo->currGame->gameOver == 0) {
        playerMove(p1, p2, 'X', gameInfo);
        if (gameInfo->currGame->gameOver == 0){		
            playerMove(p2, p1, 'O', gameInfo);
        }
    }
    free(gameInfo);
}

void playerMove(Player *curr, Player *next, char userChar, Thread *gameInfo) {
    if (gameInfo->currGame->gameOver == 0) {
        sendBoard(curr->portNum, gameInfo->currGame);
        printBoard(gameInfo->currGame->board);
        receiveNewMove(gameInfo->currGame, curr->portNum, gameInfo->currGame->player);
        gameInfo->currGame->gameOver = checkWinCondition(gameInfo->currGame);
        decideState(curr, next, userChar, gameInfo);
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

void decideState(Player *p1, Player *p2, char userChar, Thread *gameInfo) {
    char otherUserChar = (userChar == 'X') ? 'O' : 'X';
    
    if (gameInfo->currGame->gameOver) {
        int p2state = getGameState(otherUserChar, gameInfo, p2);
        sendUserState(userChar, gameInfo, p1);
        sendUserState(otherUserChar, gameInfo, p2); // issue 

        endGameScoreBoard(gameInfo);
    } else {
        gameInfo->currGame->rounds++;
        int gameState = getGameState(otherUserChar, gameInfo, p2);
        if(gameState != 0){
            int currState = getGameState(userChar, gameInfo, p1);
            gameInfo->currGame->gameOver = 1;
            
            send(p2->portNum, &gameState, sizeof(int), 0);
            sendBoard(p2->portNum, gameInfo->currGame);

            send(p1->portNum, &currState, sizeof(int), 0);
            sendBoard(p1->portNum, gameInfo->currGame);

            endGameScoreBoard(gameInfo);
        } else {
            gameInfo->currGame->player = otherUserChar;
            send(p2->portNum, &gameState, sizeof(int), 0);
        }
    }
}

void sendUserState(char userChar, Thread *gameInfo, Player *p) {
    int p1state = getGameState(userChar, gameInfo, p);
    send(p->portNum, &p1state, sizeof(int), 0);
    sendBoard(p->portNum, gameInfo->currGame);
}

int getGameState(char userChar, Thread *gameInfo, Player *p1){
    printf("current Player: %c | userChar: %c | rounds: %d\n", 
          gameInfo->currGame->player, userChar, gameInfo->currGame->rounds);
    int currGameState = 0;
    pthread_mutex_lock(&incScoreLock);
    
    if (gameInfo->currGame->gameOver || gameInfo->currGame->rounds > 9) {
        // two if statements, so that the file isnt being opened each 
        // iteration, only when the game is over
        readStoredScore(p1);
        if (gameInfo->currGame->rounds > 9){
            p1->score->ties++;
            currGameState = TIE;
        } else if (gameInfo->currGame->player != userChar) {
            p1->score->losses++;
            currGameState = LOSE;
        } else {
            p1->score->wins++;
            currGameState = WIN;
        }
        writeStoredScore(p1);
    } 
    pthread_mutex_unlock(&incScoreLock);
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

//use this method it is not used currently
void endGameScoreBoard(Thread *gameInfo){
    char psb[100];
    Score *p1 = gameInfo->player1->score;
    Score *p2 = gameInfo->player2->score;
    sprintf(psb, "%s: %dW/%dL/%dT - %s: %dW/%dL/%dT", 
    p1->name, p1->wins, p1->losses, p1->ties, p2->name, p2->wins, p2->losses,
    p2->ties);
    send(gameInfo->player1->portNum, &psb, sizeof(psb), 0);
    send(gameInfo->player2->portNum, &psb, sizeof(psb), 0);
}

void readStoredScore(Player *p) {
    int fd = -1;
    char *filename = "scoreboard.bin";
    fd = open(filename, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);

    lseek(fd, sizeof(Score) * p->locInMem, SEEK_SET);
    read(fd, p->score, sizeof(Score));
    close(fd);
}

void writeStoredScore(Player *p) {
    int fd = -1;
    char *filename = "scoreboard.bin";

    //lock
    pthread_mutex_lock(&sbLock);
    fd = open(filename, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
    
    lseek(fd, sizeof(Score) * p->locInMem, SEEK_SET);
    write(fd, p->score, sizeof(Score));
	
    close(fd);
    pthread_mutex_unlock(&sbLock);
}

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

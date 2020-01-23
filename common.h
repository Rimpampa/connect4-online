#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define COLUMNS 7
#define ROWS 6
#define PLAYER_1 'x'
#define PLAYER_2 'o'
#define EMPTY    '.'

typedef unsigned char byte;

// MESSAGE CODES
const byte GO            =  1;
const byte WIN           =  2;
const byte LOSE          =  3;
const byte DRAW          =  4;
const byte WAIT          =  5;
const byte COLUMN_FULL   =  6;
const byte OUT_OF_BOUNDS =  7;
const byte FIRST         =  8;
const byte SECOND        =  9;
const byte OTHER_LEFT    = 10;

bool insert_disc(char** board, byte column, bool side) {
    byte row = 0;
    while(row < ROWS && board[row][column] == EMPTY) row++;
    if(row-- == 0) return false;
    else board[row][column] = side ? PLAYER_1 : PLAYER_2;
    return true;
}

// Allocates the memory for the game board and initilizes it
char** create_board() {
    char** board = malloc(sizeof(char*) * ROWS);
    for(byte i = 0; i < ROWS; ++i) {
        board[i] = malloc(COLUMNS);
        memset(board[i], EMPTY, COLUMNS);
    }
    return board;
}

// Frees the memory occupied by the board
void free_board(char** board) {
    for(byte i = 0; i < ROWS; ++i) free(board[i]);
    free(board);
}
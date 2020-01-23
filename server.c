// Author: Riccardo Ripanti <github.com/Rimpampa>
// Date  : xx/01/2020 (gg/mm/yyyy)

// Header that includes resurces that both the server and client need
#include "common.h"
#include <time.h>

/*
 GAME PROTOCOL:

 1] Decide randomly who, of the two players, is the one to play for first
    
    .1] Send to the first player the FIRST flag and to the second the SECOND flag

 2] Wait for the player to send a number(one byte long) which indicates the column where he wants
    to insert his disc
    
    - Check if the player is still connected and in case he's not send to the other player the 
      OTHER_LEFT flag and close the game

    - If that column is greater than or equal to COLUMNS then send him the OUT_OF_BOUNDS flag and
      repeat point 2
      
    - If that column is filled up to the top then send him the COLUMN_FULL flag and repeat point 2

    - If the previous statements aren't true, place the disc in the specified column

 3] Check if the player won or if the board is full
    
    - If the player won send to that player the WIN flag and send to the other player the LOSE flag
      followed by the column selected by the current player in this turn and the game finishes

    - If the game board is full then send to both players the DRAW flag as noone won and the game
      finishes

    - If the previous statements aren't true then the turn chenges so to the current player is sent
      the WAIT flag and to the other player the GO flag is sent followed by the column selected by
      the current player in this turn
    
 4] Repeat point 2 but with the other player until someone wins
*/
DWORD WINAPI connect_four(LPVOID agrs);
bool is_full(char** board);
bool is_win(char** field, byte column, bool turn);

int main()
{
    srand(time(NULL));
    // Initializing the Windows Socket library (verion 2.2)
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        printf("\nWINDOWS SOCKET LIBRARY INITIALIZATION FAILED! ERROR: %d\n", result);
        exit(1);
    }

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
        printf("\nSOCKET CREATION FAILED! ERROR: %d\n", WSAGetLastError());
        WSACleanup();
        exit(2);
    }
    // Setup the socket informations
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4730); // Service port number is 4730
    server_addr.sin_addr.s_addr = 0;    // Listen on all interfaces
    
    // Ask the OS to bind the service
    result = bind(server, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in));
    if(result == -1) {
        printf("\nSERVICE BINDING FAILED! ERROR: %d\n", WSAGetLastError());
        WSACleanup();
        exit(3);
    }   
    
    // Start listening and reserver a queue for five clients that wants to connect
    if(listen(server, 5) == SOCKET_ERROR) {
        printf("\nLISTENING INITIALIZATION FAILED! ERROR: %d\n", WSAGetLastError());
        WSACleanup();
        exit(4);
    }
    // Variables that hold the informations of the client
    struct sockaddr_in client_addr;
    int client_len;
    SOCKET* client_sock;

    while(true) {
        client_sock = malloc(sizeof(SOCKET) * 2);

        do {
            // Reset the values of the client
            memset(&client_addr, 0, sizeof(struct sockaddr_in));
            client_len = sizeof(struct sockaddr_in);

            // Wait for a client to connect
            printf("\nWAITING FOR THE FIRST PLAYER TO JOIN\n");
            client_sock[0] = accept(server, (struct sockaddr*)&client_addr, &client_len);
            // Check if there was any error
            if(client_sock[0] == SOCKET_ERROR)
                printf("\nCONNECTION FAILED! ERROR: %d\n", WSAGetLastError());
        } // Repeat until a connection is established successfully
        while(client_sock[0] == SOCKET_ERROR);
        printf("\nPLAYER JOINED: %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        do {
            // Reset the values of the client
            memset(&client_addr, 0, sizeof(struct sockaddr_in));
            client_len = sizeof(struct sockaddr_in);

            // Wait for a client to connect
            printf("\nWAITING FOR THE SECOND PLAYER TO JOIN\n");
            client_sock[1] = accept(server, (struct sockaddr*)&client_addr, &client_len);
            // Check if there was any error
            if(client_sock[1] == SOCKET_ERROR)
                printf("\nCONNECTION FAILED! ERROR: %d\n", WSAGetLastError());
        } // Repeat until a connection is established successfully
        while(client_sock[1] == SOCKET_ERROR);
        printf("\nPLAYER JOINED: %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Start the game thread
        CreateThread(0, 0, connect_four, client_sock, 0, NULL);
    }
}

// This functions writes one byte from the socket and returns true if there wasn't any error
bool send_byte(SOCKET sock, byte b) {
    int res = send(sock, &b, 1, 0);
    if(res == SOCKET_ERROR) return false;
    else return true;
}

// This functions reads one byte from the socket and returns true if there wasn't any error
bool recv_byte(SOCKET sock, byte* b) {
    int res = recv(sock, b, 1, 0);
    if(res == SOCKET_ERROR) return false;
    else return true;
}

// Game function that implements the protocol
DWORD WINAPI connect_four(LPVOID args) {
    printf("\nGAME THREAD STARTED\n");
    // Take the sockets
    SOCKET* player = args;
    // Create the game board
    char** board = create_board();
    // Select who starts first randomly
    bool turn = (rand() % 2) == 0;
    // Inform the player of who starts
    if(turn) {
        send_byte(player[0], FIRST);
        send_byte(player[1], SECOND);
    } else {
        send_byte(player[0], SECOND);
        send_byte(player[1], FIRST);
    }
    bool end = false;
    byte input;
    while(!end) {
        // Recive the player input (the column number) and check for errors
        if(recv_byte(player[turn ? 0 : 1], &input))
            // Check if the specified column exists
            if(input < COLUMNS)
                // Insert the disc in the specified column
                if(insert_disc(board, input, turn))
                    // Check if the player won
                    if(is_win(board, input, turn)) {
                        // The player won
                        send_byte(player[turn ? 0 : 1], WIN);
                        send_byte(player[turn ? 1 : 0], LOSE);
                        send_byte(player[turn ? 1 : 0], input);
                        end = true;
                    } // Check is the board is full
                    else if(is_full(board)) {
                        // No one won
                        send_byte(player[turn ? 0 : 1], DRAW);
                        send_byte(player[turn ? 1 : 0], DRAW);
                    } else {
                        // End the turn of this player and start the other player one
                        send_byte(player[turn ? 0 : 1], WAIT);
                        send_byte(player[turn ? 1 : 0], GO);
                        send_byte(player[turn ? 1 : 0], input);
                        turn = !turn;
                    }
                // If the column specified is full inform the player
                else send_byte(player[turn ? 0 : 1], COLUMN_FULL);
            // If the column doesn't exist inform the player
            else send_byte(player[turn ? 0 : 1], OUT_OF_BOUNDS);
        else {
            // If there was an error tell the other player the current one left
            send_byte(player[turn ? 1 : 0], OTHER_LEFT);
            end = true;
        }
    }
    printf("\nGAME THREAD ENDED\n");
    closesocket(player[0]);
    closesocket(player[1]);
    free(board);
    free(player);
}

// Function that checks if the player who just placed the disc won
bool is_win(char** field, byte column, bool turn) {
    const char PLAYER = turn ? PLAYER_1 : PLAYER_2;

    // Find the row at which the disc stopped
    byte row = 0;
    while(field[row][column] == EMPTY) row++;

    // Find the 7x7 area that has the disc at the center
    // (removing the area that goes outside the board)
    byte min_c, max_c, min_r, max_r, count;
    min_c = column < 4 ? 0 : column - 4;
    max_c = column > COLUMNS - 4 ? COLUMNS : column + 4;
    min_r = row < 4 ? 0 : row - 4;
    max_r = row > ROWS - 4 ? ROWS : row + 4;

    // Check if the discs are aligned horizontally
    for(byte row = min_r; row < max_r; row++) {
        count = 0;
        for(byte col = min_c; col < max_c && count < 4; col++)
            if(field[row][col] == PLAYER) count++;
            else count = 0;
        // If there are 4 discs aligned return true
        if(count == 4) return true;
    }
    // Check if the discs are aligned vertically
    for(byte col = min_c; col < max_c; col++) {
        count = 0;
        for(byte row = min_r; row < max_r && count < 4; row++)
            if(field[row][col] == PLAYER) count++;
            else count = 0;
        // If there are 4 discs aligned return true
        if(count == 4) return true;
    }
    // Check if the discs are aligned diagonally (first diagonal)
    for(byte row = min_r; row < max_r; row++)
        for(byte col = min_c; col < max_c; col++) {
            count = 0;
            byte c = col, r = row;
            while(c < max_c && r < max_r && count < 4) {
                if(field[r][c] == PLAYER) count++;
                else count = 0;
                r++;
                c++;
            }
            // If there are 4 discs aligned return true
            if(count == 4) return true;
        }
    // Check if the discs are aligned diagonally (second diagonal)
    for(byte row = min_r; row < max_r; row++)
        for(byte col = min_c; col < max_c; col++) {
            count = 0;
            byte c = col, r = row;
            while(c >= min_c && r < max_r && count < 4) {
                if(field[r][c] == PLAYER) count++;
                else count = 0;
                r++;
                c--;
            }
            // If there are 4 discs aligned return true
            if(count == 4) return true;
        }
    return false;
}

bool is_full(char** board) {
    for(byte i = 0; i < ROWS; i++)
        for(byte j = 0; j < COLUMNS; j++)
            if(board[i][j] != EMPTY) return false;
    return true;
}
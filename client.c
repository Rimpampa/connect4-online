// Author: Riccardo Ripanti <github.com/Rimpampa>
// Date  : xx/01/2020 (gg/mm/yyyy)

// Client for playing a connect four game

// Header that includes resurces that both the server and client need
#include "common.h"

void print_board(char** board);
void flush_stdin();

int main() {
    // Initialize the Windows Socket library
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    // Check for errors on the library startup
    if (result != 0) {
        printf("WINDOWS SOCKET LIBRARY INITIALIZATION FAILED, ERROR: %d\n", result);
        exit(1);
    }
    // Initialize the socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("SOCKET CREATION FAILED, ERROR: %d\n", WSAGetLastError());
        WSACleanup(); // terminate the use of the WS2_32 DLL
        exit(2);
    }
    char ip[17];
    printf("Insert the IP address of the server: ");
    scanf("%s", ip);

    // Fill in the structure describing the socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4730); // Insert the port number in network byte order
    addr.sin_addr.s_addr = inet_addr(ip);

    // Connect to the server
    result = connect(
        sock,
        (struct sockaddr*)&addr,
        sizeof(struct sockaddr_in)
    );
    if(result == SOCKET_ERROR) {
        printf("SOCKET CONNECTION FAILED, ERROR: %d\n", WSAGetLastError());
        WSACleanup();
        exit(3);
    }
    bool turn, end = false;
    byte rec = 0, sen;
    // Setup the game board
    char** board = create_board();
    // Ask the server who plays for first
    recv(sock, &rec, 1, 0);
    // I am the first to play
    if(rec == FIRST) {
        turn = true;
        // Print the board
        print_board(board);
        // Ask the user to select a column
        printf("> ");
        scanf("%hhu", &sen);
        // Send to the server the position of selected column
        send(sock, &sen, 1, 0);
    } else {
        turn = false;
        printf("Wait for the other player...\n");
    }
    while(!end) {
        // Get an answer from the server
        recv(sock, &rec, 1, 0);
        if(rec == WAIT) {
            // Insert the disc in the correct column
            insert_disc(board, sen, turn);
            // Print the board
            print_board(board);
            printf("Wait for the other player...\n");
        } else if(rec == WIN || rec == LOSE || rec == DRAW || rec == OTHER_LEFT)
            end = true;
        else if(rec != WIN && rec != LOSE && rec != DRAW && rec != OTHER_LEFT) {
            if(rec == GO) {
                // Get the position where the other player putted his disc
                recv(sock, &rec, 1, 0);
                // Insert the other player's disc
                insert_disc(board, rec, !turn);
            } else if(rec == COLUMN_FULL)
                printf("That column is full! Choose another one\n");
            else if(rec == OUT_OF_BOUNDS)
                printf("It's out of the board! Chose a value between 0 and %d\n", COLUMNS - 1);
            // Print the board
            print_board(board);
            // Ask the user to select a column
            printf("> ");
            scanf("%hhu", &sen);
            // Send to the server the position of selected column
            send(sock, &sen, 1, 0);
        }
    }
    // Check the outcome of the game
    if(rec == WIN) {
        insert_disc(board, sen, turn);
        print_board(board);
        printf("You won!\n");
    } else if(rec == LOSE) {
        recv(sock, &rec, 1, 0);
        insert_disc(board, rec, !turn);
        print_board(board);
        printf("You lost!\n");
    } else if(rec == DRAW) printf("It's a draw!\n");
    else if(rec == OTHER_LEFT) printf("The other player left!\n");

    flush_stdin();
    printf("Press enter to continue...");
    getchar();
    // Free the memory occupied by the board
    free_board(board);
    // Close the socket
    closesocket(sock);
    return 0;
}

void flush_stdin() {
    for(int c = getchar(); c != EOF && c != '\n'; c = getchar());
}

void print_board(char** board) {
    for(byte i = 0; i < ROWS; ++i) {
        for(byte j = 0; j < COLUMNS; ++j)
            printf("%c ", board[i][j]);
        printf("\n");
    }
    for(byte i = 0; i < COLUMNS; ++i)
        printf("%c ", i % 10 + '0');
    printf("\n");
}
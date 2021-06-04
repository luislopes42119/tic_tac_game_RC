#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 1300
#define LEN 255
#define SERVER_ADDR "127.0.0.1"


// Jogo
void print_ttt(char ttt[]) // prints the board
{
    int i,l = 0;

    for (i = 0; i < 9; i++) {
        if (ttt[i] == 2) printf(" %d ", i+1);
        else if (ttt[i] == 0) printf (" X ");
        else if (ttt[i] == 1) printf (" O ");

        if (i == 2 || i == 5 || i == 8) printf("\n");
        if (i == 2 || i == 5) printf("---+---+---\n");
        if (i == 0 || i == 1 || i == 3 || i == 4 || i == 6 || i == 7) printf("|");
    }
    printf("\n");
}

int try_play_client(char ttt[], int spot, char mark) // tries to make a play
{
    if (ttt[spot] == 2) { // blank spot, can play
        ttt[spot] = 0;
        return 1;
    }
    else {
        return 0;
    }
}

int try_play_server(char ttt[], int spot, char mark) // tries to make a play
{
    if (ttt[spot] == 2) { // blank spot, can play
        ttt[spot] = mark;
        return 1;
    }
    else {
        return 0;
    }
}

int check_win(char ttt[]) // checks if someone won
{
    int who = -1;

    if (
        //lines
            ((who = ttt[0]) == ttt[1] && ttt[1] == ttt[2] && who != 2) ||
            ((who = ttt[3]) == ttt[4] && ttt[4] == ttt[5] && who != 2) ||
            ((who = ttt[6]) == ttt[7] && ttt[7] == ttt[8] && who != 2) ||
            //columns
            ((who = ttt[0]) == ttt[3] && ttt[3] == ttt[6] && who != 2) ||
            ((who = ttt[1]) == ttt[4] && ttt[4] == ttt[7] && who != 2) ||
            ((who = ttt[2]) == ttt[5] && ttt[5] == ttt[8] && who != 2) ||
            //diagonals
            ((who = ttt[0]) == ttt[4] && ttt[4] == ttt[8] && who != 2) ||
            ((who = ttt[2]) == ttt[4] && ttt[4] == ttt[6] && who != 2)
            )
    {
        return who;
    }
    else return -1;
}

char get_move(char ttt[])
{
    char move = 0;
    char buf[10];

    while (move < '1' || move > '9') {
        printf("Your move? -> ");
        fgets(buf, 5, stdin);
        move = buf[0];

        if (!try_play_client(ttt, move - '1', 1)) {
            printf("Spot taken, choose another!\n");
            move = 0;
        }
    }

    return move;
}

char get_move_from_buffer(char ttt[], char buf[])
{
    char move = 0;


    move = buf[0];

    if (!try_play_server(ttt, move - '1', 1)) {
        move = 'X';
    }

    return move;
}

// board initialization
char tictactoe[9] = {2,2,2,2,2,2,2,2,2};

int main() {
    //server socket
    struct sockaddr_in server;

    //socket
    int sockfd;

    int len = sizeof(server);
    int slen;
    char a[] = "Game Over!";

    //receber buffer
    char buffer_in[LEN];
    //enviar buffer
    char buffer_out[LEN];

    printf("Starting client ...\n");

    //criando o socket
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1){
        perror("ERROR opening socket.");
        exit(1);
    }
    printf("Socket criado com sucesso com sockfd: %d\n", sockfd);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    server.sin_port = htons(PORT);
    memset(server.sin_zero, 0x0, 8);
    printf("Socket atrelado a port %d com sucesso\n", PORT);

    if(connect(sockfd, (struct sockaddr*) &server, len) == -1){
        perror("Can not conect to server");
        exit(1);
    }
    printf("Conectado com sucesso\n");

    if((slen = recv(sockfd, buffer_in, LEN, 0)) > 0){
        buffer_in[slen + 1] = '\0';
        printf("Server says: %s\n", buffer_in);
    }

    //conectar com o servidor ate a mensgem de exit
    while (buffer_out[0] != 'W') {
        print_ttt(tictactoe);
        //colocando os buffers a zero
        memset(buffer_in, 0x0, LEN);
        memset(buffer_out, 0x0, LEN);

        //guardando a jogada
        buffer_out[0] = get_move(tictactoe);

        //enviar a mensagem
        send(sockfd, buffer_out, strlen(buffer_out), 0);

        //receber resposta do servidor
        recv(sockfd, buffer_in, LEN, 0);
        while (buffer_in[0] == 'X') {
            //guardando a jogada
            buffer_out[0] = get_move(tictactoe);

            //enviar a mensagem
            send(sockfd, buffer_out, strlen(buffer_out), 0);

            //receber resposta do servidor
            recv(sockfd, buffer_in, LEN, 0);
            //se resposta for X enviamos uma outra jogada
        }

        //testar se e a mensagem de termino
        if (strncmp(buffer_in, "W", 1) == 0) {
            fprintf(stdout, "%s\n", buffer_in);

        }


        buffer_out[0] = get_move_from_buffer(tictactoe, buffer_in);
        while (buffer_out[0] == 'X') {
            send(sockfd, buffer_out, strlen(buffer_out), 0);
            recv(sockfd, buffer_in, LEN, 0);
            buffer_out[0] = get_move_from_buffer(tictactoe, buffer_in);
        }

        //testar se o servidor ganhou
        if (check_win(tictactoe) == 1){
            buffer_out[0] = 'W';
            send(sockfd, buffer_out, strlen(buffer_out), 0);
            fprintf(stdout, "Game Over!\n");
        }

    }

    //strncmp();
    //fechar a conexao
    close(sockfd);
    fprintf(stdout, "\nConnection closed\n\n");
}
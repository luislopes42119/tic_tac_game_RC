#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


#define PORT 1300
#define QLEN 1
#define BUFFER_LEN 255


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

        if (!try_play_server(ttt, move - '1', 1)) {
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

    if (!try_play_client(ttt, move - '1', 1)) {
        move = 'X';
    }

    return move;
}

// board initialization
char tictactoe[9] = {2,2,2,2,2,2,2,2,2};

int main(){
    int sockfd, adrr;
    struct sockaddr_in address;

    int client_sockfd;
    struct sockaddr_in client_address;

    //char rbuffer[BUFFER_LEN];      //buffer de leitura
    char buffer_in[BUFFER_LEN];     //buffer de escrita
    char buffer_out[BUFFER_LEN];
    int sockopt = 1;
    char a[] = "w";


    printf("Starting server\n");

    //criando o socket TCP
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("ERROR opening socket.");
        exit(1);
    }
    printf("Socket criado com sucesso com sockfd: %d\n", sockfd);


    //atrelando o socket na porta 1300 e impedindo outros programas de o utilizar
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt)) == -1){
        perror("ERROR socket option");
        exit(1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    memset(address.sin_zero, 0x0, 8);
    printf("Socket atrelado a port %d com sucesso\n", PORT);

    //atrelando bind para o address e a porta
    if (bind(sockfd, (struct sockaddr* ) &address, sizeof(address)) == -1){
        perror("ERROR calling bind.");
        exit(1);
    }
    printf("Bind atrelado com sucesso\n");
//    printf("\naddress: %d\n\n", address);

    //atrelando uma quantidade de conexoes a espera de ser aceite
    if (listen(sockfd, QLEN) == -1){
        perror("ERROR calling listen");
        exit(1);
    }
    printf("Listening on port %d\n", PORT);


    //aceitando conexoes
    socklen_t client_len = sizeof(client_address);
    client_sockfd = accept(sockfd, (struct sockaddr*) &client_address, &client_len);
    if (client_sockfd < 0){
        perror("ERROR accepting new conections");
        exit(1);
    }
    printf("Conexao aceite com sucesso com new_socket: %d\n", client_sockfd);

    strcpy(buffer_out, "Hello, client!\n");

    //send
    if (send(client_sockfd, buffer_out, strlen(buffer_out), 0) == 0){
        printf("Client connected.\nWaiting for client message ...\n");
    }

    while(buffer_out[0] != 'W'){
        print_ttt(tictactoe);
        //zerar o buffer
        memset(buffer_in, 0x0, BUFFER_LEN);
        memset(buffer_out, 0x0, BUFFER_LEN);

        //receber jogada do cliente
        recv(client_sockfd, buffer_in, BUFFER_LEN, 0);

        while (buffer_in[0] == 'X'){
            //guardando a jogada
            buffer_out[0] = get_move(tictactoe);

            //enviar a mensagem
            send(client_sockfd, buffer_out, strlen(buffer_out), 0);

            //receber resposta do servidor
            recv(client_sockfd, buffer_in, BUFFER_LEN, 0);
            //se resposta for X enviamos uma outra jogada
        }

        buffer_out[0] = get_move_from_buffer(tictactoe, buffer_in);
        while (buffer_out[0] == 'X') {
            send(client_sockfd, buffer_out, strlen(buffer_out), 0);
            recv(client_sockfd, buffer_in, BUFFER_LEN, 0);
            buffer_out[0] = get_move_from_buffer(tictactoe, buffer_in);
        }

        //testar se e a mensagem de termino
        if (strncmp(buffer_in, "W", 1) == 0) {
            fprintf(stdout, "%s\n", buffer_in);
        }

        //testar se o servidor ganhou
        if (check_win(tictactoe) == 0){
            buffer_out[0] = 'W';
            send(client_sockfd, buffer_out, strlen(buffer_out), 0);
            fprintf(stdout, "Game Over!\n");

        }

        //receber jogada do servidor
        buffer_out[0] = get_move(tictactoe);

        //enviar a mensagem
        send(client_sockfd, buffer_out, strlen(buffer_out), 0);

    }

    //close client
    if (close(client_sockfd) == -1){
        perror("ERROR closing client");
    }
    printf("Cliente fechado com sucesso\n");

    //close local socket
    if (close(sockfd) == -1){
        perror("ERROR closing socket");
    }
    printf("Socket fechado com sucesso\n");

    exit(1);
}
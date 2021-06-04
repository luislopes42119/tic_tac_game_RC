Trabalho Pratico de Redes de Computadores
Luis Miguel Lopes nº 42119

1 - Diretorias do trabalho

    |--src
        |--cliente
            |--client.c
        |--servidor
            |--server.c
        |--readme.txt

2 - Como compilar

    Servidor:
        myterminal:$ cc server.c 
        myterminal:$ ./a.out 

    Cliente:
        myterminal:$ cc client.c 
        myterminal:$ ./a.out 

    Primeiro compilar e executar o servidor, e depois fazer o mesmo com o cliente.

3 - Como o trabalho funciona

    Ligamos o servidor (o servidor respeita o protocolo TCP) e depois ligamos o cliente conectando ele com o servidor, a primeira jogada é do cliente, que logo de seguida fica a espera da resposta do servidor que pode ser a jogada do servidor se a jogada do cliente for valida, 'W' se o cliente venceu, e 'X' se a jogada ê invalida, e o mesmo acontece com o servidor.

    O programa respeita a seguinte ordem de acontecimantos:
        Servidor
        1 - Cria um socket;
        2 - Atrelamos o socket a uma porta (no nosso caso 1300);
        3 - Fazemos o bind;
        4 - Fazemos o listen;
        5 - Aceitamos a conexão do cliente atrelando a um socket;
        6 - Entramos no ciclo:
            - Desenhamos a tabela;
            - Colocamos o buffer_in e o buffer_out a 0;
            - Recebemos a jogada 
            - Analisamos se a jogada é valida, se alguem ganhou, ou se a jogada anterior do servidor era valida;
            - Enviamos a jogada do servidor ou o 'W' se o cliente ganhou;
        7 - Saimos do cilco e fechamos os sockets.

        Cliente
        1 - Cria um socket;
        2 - Atrelamos o socket a uma porta (no nosso caso 1300), e no endereço do servidor;
        3 - Conectamos com o servidor;
        4 - Entramos no ciclo:
            - Desenhamos a tabela;
            - Colocamos o buffer_in e o buffer_out a 0;
            - Enviamos a jogada do cliente;
            - Recebemos a jogada 
            - Analisamos se a jogada é valida, se alguem ganhou, ou se a jogada anterior do cliente era valida;
            - Enviamos a jogada do cliente ou o 'W' se o cliente ganhou;
        7 - Saimos do cilco e fechamos o socket.
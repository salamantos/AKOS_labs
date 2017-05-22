#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "MessagesFormat.h"
#include "Common.h"
#include <signal.h>

int portno = 1337;

void error( const char* msg ) {
    perror( msg );
    exit( 0 );
}

int sockfd;

// Штатное завершение работы
void terminate( int param ) {
    //status = 1;
    close( sockfd );
    printf( "Terminated\n" );
    exit( 0 );
}

// Функция в отдельном потоке, только принимающая сообщения и выводящая их на экран
void* receiver( void* type ) {
    char buffer[MESSAGE_LEN];
    while (*(char*) type != 'o') {
        bzero( buffer, MESSAGE_LEN );
        ssize_t n = read( sockfd, buffer, 5 );
        size_t bytesCount = bytesToInt( buffer + 1 ) - 5;
        n = read( sockfd, buffer + 5, bytesCount );
        if (n < 0) error( "ERROR reading from socket" );

        size_t buff_size = 0;
        char typeGet = 0;
        char messageBody[MESSAGE_LEN];
        recognizeMessage( buffer, &buff_size, &typeGet, messageBody );

        char* lines[3];
        lines[0] = (char*) malloc( 20 );
        lines[1] = (char*) malloc( 32 );
        lines[2] = (char*) malloc( MESSAGE_LEN );
        size_t linesCount = 0;
        switch (typeGet) {
            case 'k':
                printf( "Вы были удалены из чата, причина: %s\n", messageBody );
                exit( 0 );
            case 'r':
                for (int i = 0; i < buff_size; ++i) {
                    printf( "%d ", messageBody[i] );
                }
                printf( "\n" );
                getLinesList( messageBody, buff_size, lines, &linesCount );

                printf( "   %s: %s\n", lines[1], lines[2] );
                break;
            case 'l':
                printf( "%s\n", messageBody );
                break;
            case 'm':
                printf( "%s\n", messageBody );
                break;
            case 's':
                printf( "Статусное сообщение №%s\n", messageBody );
                break;
            case 'h':
                printf( "   History | %s\n", messageBody );
                break;
            case 0:
                terminate( 146 );
                break;
            default:
                printf( "Неизвестная команда: %c\n", typeGet );
        }
    }
    printf( "Goodbye!\n" );
}

int main( int argc, char* argv[] ) {
    if (argc != 3) {
        error( "Не хватает параметров" );
    }

    portno = atoi( argv[2] );

    // Обработка ctrl+c
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = terminate;
    sigemptyset( &sigIntHandler.sa_mask );
    sigIntHandler.sa_flags = 0;
    sigaction( SIGINT, &sigIntHandler, NULL);

    struct sockaddr_in serv_addr;
    struct hostent* server;

    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    if (sockfd < 0) error( "ERROR opening socket" );

    server = gethostbyname( argv[1] );
    if (server == NULL) {
        fprintf( stderr, "ERROR, no such host\n" );
        exit( 0 );
    }
    bzero((char*) &serv_addr, sizeof( serv_addr ));
    serv_addr.sin_family = AF_INET;
    bcopy((char*) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length );
    serv_addr.sin_port = htons( portno );
    if (connect( sockfd, (struct sockaddr*) &serv_addr, sizeof( serv_addr )) < 0) error( "ERROR connecting" );

    ssize_t n;

    // Логинимся
    char type = 'i'; // login
    char messBody[100];
    char login[32];
    char password[32];
    printf( "login: " );
    bzero( login, 32 );
    fgets( login, 32, stdin );
    login[strlen( login ) - 1] = 0; // Удаляем символ \n из конца строки

    printf( "password: " );
    bzero( password, 32 );
    fgets( password, 32, stdin );
    password[strlen( password ) - 1] = 0; // Удаляем символ \n из конца строки

    sprintf( messBody, "%s\n%s", login, password );

    char* message = (char*) malloc( MESSAGE_LEN );
    size_t messSize = formMessage( message, type, messBody, strlen( messBody ));
    n = write( sockfd, message, messSize );
    if (n < 0)
        error( "ERROR writing to socket" );

    bzero( message, MESSAGE_LEN );
    n = read( sockfd, message, MESSAGE_LEN );

    if (n < 0)
        error( "ERROR reading from socket" );
    recognizeMessage( message, &messSize, &type, messBody );
    //printf( "2t: %c, m: %s\n", type, messBody );
    if (type == 's') {
        switch (messBody[0]) {
            case '0':
                printf( "Успешная аутентификация!\n" );
                break;
            case '3':
                printf( "Ошибка аутентификации ->%c<-\n", type );
                exit( 0 );
            case '4':
                printf( "Ошибка регистрации ->%c<-\n", type );
                exit( 0 );
            case '5':
                printf( "Ошибка доступа ->%c<-\n", type );
                exit( 0 );
            default:
                printf( "Server is crazy! ->%c<-\n", type );
                exit( 0 );
        }
    } else {
        printf( "Server is crazy! ->%c<-\n", type );
        exit( 0 );
    }

    printf( "Welcome to чат! Список команд:\n" );
    printf( "r message - Просто сообщение\n" );
    printf( "o - logout\n" );
    printf( "h count - history\n" );
    printf( "l - list\n" );
    printf( "Например: 'r Привет!' - отправить сообщение 'Привет!'\n\n" );

    pthread_t thread;
    pthread_create( &thread, NULL, receiver, (void*) &type );

    while (type != 'o') {
        char buffer[MESSAGE_LEN];
        bzero( buffer, MESSAGE_LEN );
        fgets( buffer, 3, stdin ); // Для типа сообщения
        type = buffer[0];
        size_t bufferLen = 0;
        if (type == 'o' || type == 'l') {
            bzero( buffer, MESSAGE_LEN );
        } else {
            fgets( buffer, MESSAGE_LEN, stdin );
            bufferLen = strlen( buffer );
            buffer[--bufferLen] = 0; // Удаляем символ \n из конца строки
        }

        char res[MESSAGE_LEN];
        size_t resSize = formMessage( res, type, buffer, bufferLen );
        n = write( sockfd, res, resSize );
        printf( "Send\n" );
        if (n < 0) error( "ERROR writing to socket" );
    }
    sleep( 1 );
    close( sockfd );
    return 0;
}

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


void error( const char* msg ) {
    perror( msg );
    exit( 0 );
}

int sockfd;

void terminate( int param ) {
    //status = 1;
    close( sockfd );
    printf( "term %d\n", param );
    exit( 0 );
}

void* receiver( void* type ) {
    char buffer[MESSAGE_LEN];
    while (*(char*) type != 'o') {
        bzero( buffer, MESSAGE_LEN );
        printf( "qwertyuiop!\n" );
        ssize_t n = read( sockfd, buffer, MESSAGE_LEN );
        if (n < 0)
            error( "ERROR reading from socket" );

        size_t buff_size = 0;
        char typeGet = 0;
        char messageBody[MESSAGE_LEN];
        recognizeMessage( buffer, &buff_size, &typeGet, messageBody );

        switch (typeGet) {
            case 'k':
                printf( "Вы были удалены из чата, причина: %s\n", messageBody );
                exit( 0 );
            case 'r':
                printf( "%s\n", messageBody );
                break;
            default:
                    printf( "Server is crazy! ->%c<-\n", typeGet );
        }
    }
    printf( "goodbye!\n" );
}

int main( int argc, char* argv[] ) {
    if (argc < 2) {
        printf( "Run with server IP!\n" );
        exit( 0 );
    }

    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = terminate;
    sigemptyset( &sigIntHandler.sa_mask );
    sigIntHandler.sa_flags = 0;
    sigaction( SIGINT, &sigIntHandler, NULL);

    struct sockaddr_in serv_addr;
    struct hostent* server;

    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    if (sockfd < 0)
        error( "ERROR opening socket" );

    server = gethostbyname( argv[1] );
    if (server == NULL) {
        fprintf( stderr, "ERROR, no such host\n" );
        exit( 0 );
    }
    bzero((char*) &serv_addr, sizeof( serv_addr ));
    serv_addr.sin_family = AF_INET;
    bcopy((char*) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length );
    serv_addr.sin_port = htons( portno );
    if (connect( sockfd, (struct sockaddr*) &serv_addr, sizeof( serv_addr )) < 0)
        error( "ERROR connecting" );

    ssize_t n;

    printf( "hello\n" );

    // Логинимся
    char type = 'i'; // login
    char messBody[100];
    char login[32];
    char password[32];
    printf( "\nlogin: " );
    bzero( login, 32 );
    fgets( login, 32, stdin );
    login[strlen( login ) - 1] = 0; // Удаляем символ \n из конца строки

    printf( "password: " );
    bzero( password, 32 );
    fgets( password, 32, stdin );
    password[strlen( password ) - 1] = 0; // Удаляем символ \n из конца строки

    sprintf( messBody, "%s\n%s", login, password );
    //printf( "1t: %c, m: %s\n", type, messBody );

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
            default:
                printf( "Server is crazy! ->%c<-\n", type );
                exit( 0 );
        }
    } else {
        printf( "Server is crazy! ->%c<-\n", type );
        exit( 0 );
    }

    printf( "Welcome to чат! Список команд:\n" );
    printf( "%%r message - Просто сообщение\n" );
    printf( "%%o - logout\n" );
    printf( "%%h count - history\n" );
    printf( "%%l - list\n" );
    printf( "Например: 'r Привет' - отправить сообщение\n" );

    pthread_t thread;
    pthread_create( &thread, NULL, receiver, (void*) &type );

    while (type != 'o') {
        //printf( "Please enter the message ([t] mess): " );
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
        if (n < 0)
            error( "ERROR writing to socket" );
    }
    sleep( 2 );
    close( sockfd );
    return 0;
}

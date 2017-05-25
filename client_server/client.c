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
        lines[0] = (char*) malloc( MESSAGE_LEN );
        lines[1] = (char*) malloc( MESSAGE_LEN );
        lines[2] = (char*) malloc( MESSAGE_LEN );
        size_t linesCount = 0;
        int status = 0;
        switch (typeGet) {
            case 'k':
                getLinesList( messageBody, buff_size, lines, &linesCount );
                printf( "Вы были удалены из чата, причина: %s\n", lines[0] );
                exit( 0 );
            case 'r':
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
                status = bytesToInt( messageBody + 4 );
                switch (status) {
                    case 0:
                        printf( "Успешно!\n" );
                        break;
                    case 1:
                        printf( "Сообщение неизвестного типа\n" );
                        break;
                    case 3:
                        printf( "Ошибка аутентификации\n" );
                        break;
                    case 4:
                        printf( "Ошибка регистрации\n" );
                        break;;
                    case 5:
                        printf( "Ошибка доступа\n" );
                        break;
                    case 6:
                        printf( "Битое сообщение\n" );
                        break;
                    default:
                        printf( "Server is crazy! ->%d<-\n", status );
                        break;
                }
                break;
            case 'h':
                getLinesList( messageBody, buff_size, lines, &linesCount );
                printf( "   History | %s: %s\n", lines[1], lines[2] );
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

    char messageBody[MESSAGE_LEN];
    char* lines[2];
    lines[0] = login;
    lines[1] = password;
    size_t messBLen = 0;
    formMessageBody( messageBody, &messBLen, lines, 2 );

    char* message = (char*) malloc( MESSAGE_LEN );
    size_t messSize = formMessage( message, 'i', messageBody, messBLen );
    n = write( sockfd, message, messSize );
    if (n < 0) error( "ERROR writing to socket" );

    bzero( message, MESSAGE_LEN );
    n = read( sockfd, message, MESSAGE_LEN );
    if (n < 0) error( "ERROR reading from socket" );

    recognizeMessage( message, &messSize, &type, messBody );
    if (type == 's') {
        int status = bytesToInt( messBody + 4 );
        switch (status) {
            case 0:
                printf( "Успешная аутентификация!\n" );
                break;
            case 3:
                printf( "Ошибка аутентификации\n" );
                exit( 0 );
            case 4:
                printf( "Ошибка регистрации\n" );
                exit( 0 );
            case 5:
                printf( "Ошибка доступа\n" );
                exit( 0 );
            default:
                printf( "Server is crazy! ->%d<-\n", status );
                exit( 0 );
        }
    } else {
        printf( "Server is crazy! ->%c<-\n", type );
        exit( 0 );
    }

    printf( "Вы подключились, список команд:\n" );
    printf( "/r message - Отправить сообщение в чат\n" );
    printf( "/o - выйти из чата\n" );
    printf( "/h count - показать историю\n" );
    printf( "/l - показать список кто онлайн\n" );
    printf( "Например: '/r Привет!' - отправить сообщение 'Привет!'\n\n" );

    pthread_t thread;
    pthread_create( &thread, NULL, receiver, (void*) &type );

    while (type != 'o') {
        char buffer[MESSAGE_LEN];
        bzero( buffer, MESSAGE_LEN );
        fgets( buffer, 4, stdin ); // Для типа сообщения
        type = buffer[1];
        size_t bufferLen = 0;
        if (type == 'o' || type == 'l') {
            bzero( buffer, 5 );
        } else {
            fgets( buffer, MESSAGE_LEN, stdin );
            bufferLen = strlen( buffer );
            buffer[--bufferLen] = 0; // Удаляем символ \n из конца строки
        }

        char message[MESSAGE_LEN];
        size_t messSize = 0;
        if (type == 'r' || type == 'h') {
            char messageBody[MESSAGE_LEN];
            char* lines[1];
            lines[0] = buffer;
            size_t messBLen = 0;
            formMessageBody( messageBody, &messBLen, lines, 1 );
            messSize = formMessage( message, type, messageBody, messBLen );
        } else if (type == 'k') {
            char id[32];
            char reason[MESSAGE_LEN];

            char messageBody[MESSAGE_LEN];
            char* lines[2];
            lines[0] = id;
            lines[1] = reason;

            int i = 0;
            while (buffer[i] != ' ' && i < strlen( buffer )) {
                id[i] = buffer[i];
                ++i;
            }
            int j = i++;
            while (i < strlen( buffer )) {
                reason[i - j - 1] = buffer[i];
                ++i;
            }

            size_t messBLen = 0;
            formMessageBody( messageBody, &messBLen, lines, 2 );
            messSize = formMessage( message, type, messageBody, messBLen );
        } else if (type == 'o' || type == 'l') {
            messSize = formMessage( message, type, buffer, bufferLen );
        } else {
            printf( "Неизвестная команда!\n" );
        }
        n = write( sockfd, message, messSize );
        if (n < 0) error( "ERROR writing to socket" );
    }
    sleep( 1 );
    close( sockfd );
    return 0;
}

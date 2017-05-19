#include <stdio.h>
#include <stdlib.h>
#include "MessagesFormat.h"
#include "UsersModule.h"
#include "SharedMemory.h"
#include <dirent.h>
#include "MessagesReceiving.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include "TCP_connection.h"
#include "Common.h"

int status = 0;

void error( const char* msg ) {
    perror( msg );
    exit( 1 );
}

int sockfd, newsockfd;
sem_t semaphore;

void terminate( int param ) {
    //status = 1;
    close( sockfd );
    sem_destroy( &semaphore );
    printf( "term %d\n", param );
    exit( 0 );
}

int main() {
    // Проверяем, что все нужные директории и файлы существуют
    DIR* dir = opendir( "users" );
    if (!dir) {
        fprintf( stderr, "Error opening directory\n" );
        return 1;
    }

    // Обработка сигнала ctrl+C
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = terminate;
    sigemptyset( &sigIntHandler.sa_mask );
    sigIntHandler.sa_flags = 0;
    sigaction( SIGINT, &sigIntHandler, NULL);

    // Выделяем общую память под буффер сообщений
    struct CMessage* messBuffer = (struct CMessage*) malloc( 50 * sizeof( struct CMessage ));
    int* lastMessId = 0; // Номер последнего пришедшего сообщения, нужен, чтобы потоки могли знать, есть ли сообщения для отправки

    // Инициализация семафора
    sem_init( &semaphore, 0, 1 );

    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    printf( "!!!%d!!!\n", sockfd );
    if (sockfd < 0)
        error( "ERROR opening socket" );
    bzero((char*) &serv_addr, sizeof( serv_addr ));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons( portno );
    if (bind( sockfd, (struct sockaddr*) &serv_addr, sizeof( serv_addr )) < 0)
        error( "ERROR on binding" );
    listen( sockfd, 100 );
    clilen = sizeof( cli_addr );
    printf( "Started listening\n" );

    int socketList[100];
    int online = 0;

    while (1) {
        printf("lalala\n");
        newsockfd = accept( sockfd, (struct sockaddr*) &cli_addr, &clilen );
        if (newsockfd < 0)
            continue;

        struct CThreadParam param;
        param.newsockfd = &newsockfd;
        param.messBuffer = messBuffer;
        param.lastMessId = lastMessId;
        param.semaphore = &semaphore;
        pthread_t thread;
        pthread_create( &thread, NULL, newThread, (void*) &param );
    }
}

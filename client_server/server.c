#include <stdio.h>
#include <stdlib.h>
#include "MessagesFormat.h"
#include "UsersModule.h"
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include "TCP_connection.h"
#include "Common.h"

int status = 0;
int portno = 1337;

// Завершение работы с ошибкой
void error( const char* msg ) {
    perror( msg );
    exit( 1 );
}

int sockfd;
sem_t semaphore;

// Штатное завершение работы
void terminate( int param ) {
    //status = 1;
    close( sockfd );
    sem_destroy( &semaphore );
    printf( "term %d\n", param );
    exit( 0 );
}

// Массовая рассылка новых сообщений
void* openBroadcast( void* getParam ) {
    // Получаем данные
    struct CThreadParam* param = (struct CThreadParam*) getParam;
    int newsockfd = param->newsockfd;
    struct CMessage* messBuffer = param->messBuffer;
    int* lastMessId = param->lastMessId;
    int* onlineCount = param->onlineCount;
    struct CUser* usersList = param->usersList;
    sem_t* semaphore = param->semaphore;
    //free( param );
    int currentId = *lastMessId;
    while (1) {
        // Есть ли новые сообщения
        if (currentId == *lastMessId) {
            continue;
        }
//        char message[MESSAGE_LEN];
//        char messageBody[MESSAGE_LEN];
//        size_t messBLen=0;
//        formMessageBody(messageBody, &messBLen, &messBuffer[currentId].mess, 1);
//        size_t messSize = formMessage( message, 'r', messageBody, messBLen );
//        for (int i = 0; i < messBuffer[currentId].len; ++i) {
//            printf("%d ", messBuffer[currentId].mess[i]);
//        }
//        printf("\n");
        sendToAll( messBuffer[currentId].mess, messBuffer[currentId].len, *onlineCount, usersList );
        ++currentId;
    }
}

int main( int argc, char* argv[] ) {
    if (argc != 3) {
        error( "Не хватает параметров" );
    }

    if (strcmp( "", argv[2] ) == 0) {
        error( "Пустой пароль запуска" );
    }
    strcpy( ROOT_PASSWORD, argv[2] );

    portno = atoi( argv[1] );

    // Инициализируем переменные
    userIdRandom = 1000;
    nullUser = (struct CUser*) malloc( sizeof( struct CUser ));
    nullUser->id = 0;
    nullUser->isKicked = 0;
    nullUser->isOnline = 0;
    nullUser->login = 0;
    nullUser->password = 0;

    // Обработка сигнала ctrl+C
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = terminate;
    sigemptyset( &sigIntHandler.sa_mask );
    sigIntHandler.sa_flags = 0;
    sigaction( SIGINT, &sigIntHandler, NULL);

    // Выделяем общую память под буффер сообщений
    struct CMessage* messBuffer = (struct CMessage*) malloc( 50 * sizeof( struct CMessage ));
    int* lastMessId = (int*) malloc(
            sizeof( int )); // Номер последнего пришедшего сообщения, нужен, чтобы потоки могли знать, есть ли сообщения для отправки
    *lastMessId = 0;

    // Инициализация семафора
    sem_init( &semaphore, 0, 1 );

    // Открываем соединение
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    sockfd = socket( AF_INET, SOCK_STREAM, 0 ); // Номер сокета, с которого будем читать
    if (sockfd < 0) error( "ERROR opening socket" );
    bzero((char*) &serv_addr, sizeof( serv_addr ));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; //inet_addr("150.50.50.50");
    serv_addr.sin_port = htons( portno );
    // Assigning a name to a socket - для сокета получаем файл-дескриптор, теперь можем работать с ним как с файлом
    if (bind( sockfd, (struct sockaddr*) &serv_addr, sizeof( serv_addr )) < 0)
        error( "ERROR on binding" );
    // Начинаем прослушку
    listen( sockfd, MAX_CONNECTED_USERS );
    clilen = sizeof( cli_addr );
    printf( "Started listening\n" );

    // Хранилище известных нам пользователей
    struct CUser* usersList = (struct CUser*) malloc( MAX_CONNECTED_USERS * sizeof( struct CUser ));
    for (int i = 0; i < MAX_CONNECTED_USERS; ++i) {
        usersList[i] = *nullUser;
    }
    int* onlineCount = (int*) malloc( sizeof( int ));
    *onlineCount = 0;

    // Новый поток для broadcast
    pthread_t thread1;
    struct CThreadParam* param = (struct CThreadParam*) malloc( sizeof( struct CThreadParam ));
    param->newsockfd = sockfd;
    param->messBuffer = messBuffer;
    param->lastMessId = lastMessId;
    param->onlineCount = onlineCount;
    param->usersList = usersList;
    param->semaphore = &semaphore;
    pthread_create( &thread1, NULL, openBroadcast, (void*) param );

    // Ждем нового пользователя, начинаем работать с ним в новом потоке
    while (1) {
        int newsockfd = accept( sockfd, (struct sockaddr*) &cli_addr, &clilen );
        if (newsockfd < 0) continue;

        param->newsockfd = newsockfd;
        param->messBuffer = messBuffer;
        param->lastMessId = lastMessId;
        param->onlineCount = onlineCount;
        param->usersList = usersList;
        param->semaphore = &semaphore;
        pthread_t thread;
        pthread_create( &thread, NULL, newThread, (void*) param );
    }
}

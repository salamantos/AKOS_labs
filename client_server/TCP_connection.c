#include "TCP_connection.h"
#include "Common.h"
#include "MessagesFormat.h"
#include "MessagesReceiving.h"
#include "UsersModule.h"

int switchMessType( char type, char* messBody, size_t messBSize, int newsockfd ) {
    //sleep(10);
    printf( "t: %c, m: %s\n", type, messBody );
    char message[MESSAGE_LEN];
    switch (type) {
        case 'i':
            type = 's';
            // Получаем логин и пароль
            char login[32];
            char password[32];
            bzero( login, 32 );
            bzero( password, 32 );
            int j = 0;
            int i = 0;
            for (i = 0; i < messBSize; ++i) {
                if (messBody[i] == '\n')
                    break;
                login[j++] = messBody[i];
            }
            j = 0;
            for (int k = i + 1/*Пропускаем \n*/; k < messBSize; ++k) {
                password[j++] = messBody[k];
            }
            int userStatus = isUserExist( login );
            if (userStatus == 0) {
                int crStatus = createUser( login, password, 0 );
                if (crStatus == 0) {
                    bzero( messBody, MESSAGE_LEN );
                    messBody[0] = '0';
                } else {
                    bzero( messBody, MESSAGE_LEN );
                    messBody[0] = '4';
                }
            } else if (userStatus == 1) {
                int auStatus = authentication( login, password );
                if (auStatus == 0) {
                    bzero( messBody, MESSAGE_LEN );
                    messBody[0] = '0';
                } else {
                    bzero( messBody, MESSAGE_LEN );
                    messBody[0] = '3';
                }
            }
            break;
        case 'r':
            break;
        default:
            printf( "Client is crazy! ->%d<-\n", type );
            break;
    }

    size_t messSize = formMessage( message, type, messBody, messBSize );

//    for (int l = 0; l < 10; ++l) {
//        ssize_t n = write( newsockfd, message, messSize );
//        sleep(1);
//    }
    ssize_t n = write( newsockfd, message, messSize );

    if (n < 0) {
        error( "ERROR writing to socket" );
    }
}

int connectNewUser( struct CThreadParam* param ) {
    char buffer[MESSAGE_LEN];
    ssize_t n;

    int newsockfd = *param->newsockfd;
    struct CMessage* messBuffer = param->messBuffer;
    int* lastMessId = param->lastMessId;
    sem_t* semaphore = param->semaphore;

    printf( "Started talking\n" );
    char type = '0';
    while (type != 'o') {
        bzero( buffer, MESSAGE_LEN );
        n = read( newsockfd, buffer, MESSAGE_LEN );
        printf("read\n");
        if (buffer[0] == 0) break;
        if (n < 0)
            error( "ERROR reading from socket" );

        size_t messBSize = 0;
        char messBody[MESSAGE_LEN];
        recognizeMessage( buffer, &messBSize, &type, messBody );
        switchMessType( type, messBody, messBSize, newsockfd );

    }
    close( newsockfd );
    printf( "Stopped talking\n" );
    return 0;
}
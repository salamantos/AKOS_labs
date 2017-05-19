#include <stdio.h>
#include <stdlib.h>
#include "MessagesFormat.h"
#include "UsersModule.h"
#include <dirent.h>


int main() {
    // Проверяем, что все нужные директории и файлы существуют
    DIR* dir = opendir( "users" );
    if (!dir) {
        fprintf( stderr, "Error opening directory\n" );
        return 1;
    }

//    unsigned char* resultMessage = (unsigned char*) malloc(5000 * sizeof(char));
//    size_t len = formMessage(resultMessage, 'm', getTimeStamp(), 8);
//    for (int i = 0; i < len; ++i) {
//        printf("%c", resultMessage[i]);
//    }
//    printf("\n");
//    char type = 0;
//    unsigned char* messageBody = (unsigned char*) malloc(5000 * sizeof(char));
//    recognizeMessage(resultMessage, &len, &type, messageBody);
//    int n1 = 0, n2 = 0;
//    bytesToDoubleInt(messageBody, &n1, &n2);
//    printf("Recognized: %c %d %d %d\n", type, (int) len, n1,n2);
//
//    free(resultMessage);
//    free(messageBody);

//    char login[32] = "root";
//    if (isUserExist( login ) == 0) {
//        printf( "User %s created\n", login );
//        createUser( login, "pass", 0 );
//    } else {
//        printf( "User %s exist\n", login );
//    }
//    printf( "%d\n", authentication( login, "pass" ));
//    printf( "%d\n", kick("salamantos"));

    unsigned char* resultMessage = (unsigned char*) malloc( 5000 * sizeof( char ));
    size_t len = formMessage( resultMessage, 'm', "Qwerty yru", 10 );
    for (int i = 0; i < len; ++i) {
        //printf( "%c", resultMessage[i] );
    }
    //printf( "\n" );

    //writeToHistory( "1234567890qw", 12 );
    char mess[100];
    size_t length = 0;
    long int pos = 0;
    readFromHistory( mess, &length, &pos );
    for (int i = 0; i < length; ++i) {
        printf( "%c", mess[i] );
    }
    printf( ", %d; %ld\n", (int) length, pos );

    readFromHistory( mess, &length, &pos );
    for (int i = 0; i < length; ++i) {
        printf( "%c", mess[i] );
    }
    printf( ", %d; %ld\n", (int) length, pos );

    readFromHistory( mess, &length, &pos );
    for (int i = 0; i < length; ++i) {
        printf( "%c", mess[i] );
    }
    printf( ", %d; %ld\n", (int) length, pos );

    free( resultMessage );





    struct CSharedData data;
    data.online = 146;
    data.onlineUsers[0] = "salo";
    writeToSharedMemory(&data);

    struct CSharedData data2;
    readFromSharedMemory(&data2);
    printf("%u, %s", data2.online, data2.onlineUsers[0]);

    return 0;
}




int status = 0;

void error( const char* msg ) {
    perror( msg );
    exit( 1 );
}


int sockfd, newsockfd, portno;

void terminate( int param ) {
    //status = 1;
    printf( "term\n" );
    close( newsockfd );
    close( sockfd );
    exit( 0 );
}


int main() {
    // Проверяем, что все нужные директории и файлы существуют
    DIR* dir = opendir( "users" );
    if (!dir) {
        fprintf( stderr, "Error opening directory\n" );
        return 1;
    }

    signal( SIGINT, terminate ); // обработка сигнала


    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    if (sockfd < 0)
        error( "ERROR opening socket" );
    bzero((char*) &serv_addr, sizeof( serv_addr ));
    portno = 8083;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons( portno );
    if (bind( sockfd, (struct sockaddr*) &serv_addr, sizeof( serv_addr )) < 0)
        error( "ERROR on binding" );
    listen( sockfd, 100 );
    clilen = sizeof( cli_addr );

    while (1) {
        printf("Started listening\n");
        newsockfd = accept( sockfd,
                            (struct sockaddr*) &cli_addr,
                            &clilen );
        if (newsockfd < 0)
            continue;
        //while (buffer[0] != '1') {
        printf("Started talking\n");
        bzero( buffer, 256 );
        n = read( newsockfd, buffer, 255 );
        if (n < 0)
            error( "ERROR reading from socket" );
        printf( "Here is the message: %s\n", buffer );
        n = write( newsockfd, "I got your message\n", 19 );
        if (n < 0)
            error( "ERROR writing to socket" );
//            if (status != 0) {
//                //printf( "yes\n" );
//                break;
//            }
        //}

        printf("Stopped talking\n");

        if (status != 0) {
            //printf( "yes\n" );
            //break;
        }
    }

    return 0;
}

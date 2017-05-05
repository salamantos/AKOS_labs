#include <stdio.h>
#include <stdlib.h>
#include "MessagesFormat.h"
#include "UsersModule.h"
#include <dirent.h>
#include "MessagesReceiving.h"


int main() {
    // Проверяем, что все нужные директории и файлы существуют
    DIR* dir = opendir( "users" );
    if (!dir) {
        fprintf( stderr, "Error opening directory\n" );
        return 1;
    }

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

    return 0;
}
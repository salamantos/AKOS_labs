//
// Created by salamant on 01.05.17.
//

#include <stdio.h>
#include "MessagesFormat.h"

int writeToHistory( char* message, size_t len ) {
    FILE* f = fopen( "history.ultimatefiletype", "a" );
    if (f == NULL) {
        // файл не удалось открыть
        return 2;
    }
    fwrite( intToBytes(5 + len), 1, 4, f );
    fwrite( message, 1, len, f );
    fprintf( f, "\n" );
    fclose( f );
    return 0;
}

int readFromHistory( char* message, size_t* len, long int* pos ) {
    FILE* f = fopen( "history.ultimatefiletype", "r" );
    if (f == NULL) {
        // файл не удалось открыть
        return 2;
    }
    fseek(f, *pos, SEEK_SET);

    char buff[4];
    fread( buff, 1, 4, f );
    *len = bytesToInt(buff);
    fread( message, 1, *len, f );
    *pos += *len;
    *len -= 5;
//    printf("-->len: %d<--\n", (int)*len);
//    printf("-->pos: %d<--\n\n\n", (int)*pos);

    fclose( f );
    return 0;
}
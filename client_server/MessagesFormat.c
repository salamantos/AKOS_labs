#include "MessagesFormat.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "Common.h"

unsigned char* intToBytes( unsigned long number ) {
    unsigned char* bytes = (unsigned char*) malloc( 5 );
    bytes[3] = (unsigned char) (number & 0x000000FF);
    bytes[2] = (unsigned char) ((number & 0x0000FF00) >> 8);
    bytes[1] = (unsigned char) ((number & 0x00FF0000) >> 16);
    bytes[0] = (unsigned char) ((number & 0xFF000000) >> 24);
    bytes[4] = 0; // Symbol of string end

    return bytes;
}

size_t bytesToInt( unsigned char* bytes ) {
    size_t number = 0;
    number = bytes[0] << 24;
    number += (bytes[1] << 16);
    number += (bytes[2] << 8);
    number += (bytes[3]);

    return number;
}

char* timeStampToStr( char* bytes ) {
    int number1 = 0;
    number1 = bytes[0] << 24;
    number1 += (bytes[1] << 16);
    number1 += (bytes[2] << 8);
    number1 += (bytes[3]);

    int number2 = 0;
    number2 = bytes[4] << 24;
    number2 += (bytes[5] << 16);
    number2 += (bytes[6] << 8);
    number2 += (bytes[7]);

    struct timeval tv;
    tv.tv_sec = number1;
    tv.tv_usec = number2;
    time_t nowtime;
    struct tm* nowtm;
    char tmbuf[64];
    char* buff = (char*) malloc( 64 );

    nowtime = tv.tv_sec;
    nowtm = localtime( &nowtime );
    strftime( tmbuf, sizeof tmbuf, "%H:%M:%S", nowtm );
    snprintf( buff, sizeof buff, "%s.%06ld", tmbuf, tv.tv_usec );
    return buff;
}

unsigned char* getTimeStamp() {
    struct timeval tv;
    gettimeofday( &tv, NULL);

    unsigned char* result = (unsigned char*) malloc( 9 );
    int i = 0;
    int k;
    unsigned char* timeInSec = intToBytes((unsigned long) tv.tv_sec );
    unsigned char* timeInMicroSec = intToBytes((unsigned long) tv.tv_usec );
    for (k = 0; k < 4; ++k) {
        result[i++] = timeInSec[k];
    }
    for (k = 0; k < 4; ++k) {
        result[i++] = timeInMicroSec[k];
    }
    result[8] = 0;

    return result;
}

void formMessageBody( unsigned char* messageBody, size_t* messageBodyLen, char* lines[], size_t linesCount ) {
    int j = 0;
    int i = 0;
    // Для timestamp
    if (linesCount > 0 && lines[0][0] == 0) {
        unsigned char* lenInBytes = intToBytes( 8 );
        for (int k = 0; k < 4; ++k) {
            messageBody[j++] = lenInBytes[k];
        }
        for (int k = 0; k < 8; ++k) {
            messageBody[j++] = lines[0][k];
        }
        i = 1;
    }
    for (; i < linesCount; ++i) {
        unsigned char* lenInBytes = intToBytes( strlen( lines[i] ));
        for (int k = 0; k < 4; ++k) {
            messageBody[j++] = lenInBytes[k];
        }
        int strLen = strlen( lines[i] );
        for (int k = 0; k < strLen; ++k) {
            messageBody[j++] = lines[i][k];
        }
        free( lenInBytes );
    }
    *messageBodyLen = j;
}

void getLinesList( unsigned char* messageBody, size_t messageBodyLen, char* lines[], size_t* linesCount ) {
    int j = 0;
    int i = 4;
    while (i < messageBodyLen) {
        size_t len = bytesToInt( &messageBody[i - 4] );
        for (int k = 0; k < len; ++k) {
            lines[j][k] = messageBody[i++];
        }
        i += 4;
        ++j;
    }
    *linesCount = j;
}

// Формирование сообщения
size_t
formMessage( unsigned char* resultMessage, unsigned char type, unsigned char* messageBody, size_t messageBodyLen ) {
    unsigned char* lenInBytes = intToBytes( messageBodyLen + 5 );
    int i = 0;
    resultMessage[i++] = type;
    int k;
    for (k = 0; k < 4; ++k) {
        resultMessage[i++] = lenInBytes[k];
    }

    for (k = 0; k < messageBodyLen; ++k) {
        resultMessage[i++] = messageBody[k];
    }
    free( lenInBytes );
    return (size_t) i;
}

void recognizeMessage( char* message, size_t* len, char* type, char* messageBody ) {
    unsigned int i = 0;
    *type = message[i++];
    unsigned char* lenInBytes = (unsigned char*) malloc( 5 );
    size_t j;
    for (j = 0; j < 4; ++j) {
        lenInBytes[j] = (unsigned char) message[i++];
    }
    size_t messageBodyLen = bytesToInt( lenInBytes ) - 5;
    for (j = 0; j < messageBodyLen; ++j) {
        messageBody[j] = (unsigned char) message[i++];
    }
    messageBody[messageBodyLen] = 0;
    *len = messageBodyLen;

    free( lenInBytes );
}

void prepareUsersList( char* messageBody, size_t* messBSize, int* onlineCount, struct CUser* usersList ) {
    bzero( messageBody, MESSAGE_LEN );
    strcpy( messageBody, "Пользователи онлайн:\n" );
    size_t shift = strlen( "Пользователи онлайн:\n" ); // Сдвиг относительно начала строки
    int i = 0; // Количество учтенных пользователей
    int j = 0; // Индекс массива
    while (i < *onlineCount) {
        if (usersList[j].id != 0 && usersList[j].isOnline == 1) {
            int charactersWritten = sprintf( messageBody + shift, "(%d) %s\n", usersList[j].id, usersList[j].login );
            shift += charactersWritten;
            ++i;
        }
        ++j;
    }
    *messBSize = shift;
}
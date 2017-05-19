#include "MessagesFormat.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

unsigned char* intToBytes(unsigned long number) {
    unsigned char* bytes = (unsigned char*) malloc(5);
    bytes[3] = (unsigned char) (number & 0x000000FF);
    bytes[2] = (unsigned char) ((number & 0x0000FF00) >> 8);
    bytes[1] = (unsigned char) ((number & 0x00FF0000) >> 16);
    bytes[0] = (unsigned char) ((number & 0xFF000000) >> 24);
    bytes[4] = 0; // Symbol of string end

    return bytes;
}

size_t bytesToInt(unsigned char* bytes) {
    size_t number = 0;
    number = bytes[0] << 24;
    number += (bytes[1] << 16);
    number += (bytes[2] << 8);
    number += (bytes[3]);

    return number;
}

void bytesToDoubleInt(unsigned char* bytes, int* number1, int* number2) {
    *number1 = 0;
    *number1 = bytes[0] << 24;
    *number1 += (bytes[1] << 16);
    *number1 += (bytes[2] << 8);
    *number1 += (bytes[3]);

    *number2 = 0;
    *number2 = bytes[4] << 24;
    *number2 += (bytes[5] << 16);
    *number2 += (bytes[6] << 8);
    *number2 += (bytes[7]);
}

unsigned char* getTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    printf("%d, %d\n", (int) tv.tv_sec, (int) (unsigned long) tv.tv_usec);

    unsigned char* result = (unsigned char*) malloc(9);
    int i = 0;
    int k;
    unsigned char* timeInSec = intToBytes((unsigned long) tv.tv_sec);
    unsigned char* timeInMicroSec = intToBytes((unsigned long) tv.tv_usec);
    for (k = 0; k < 4; ++k) {
        result[i++] = timeInSec[k];
    }
    for (k = 0; k < 4; ++k) {
        result[i++] = timeInMicroSec[k];
    }

    return result;
}

size_t
formMessage(unsigned char* resultMessage, unsigned char type, unsigned char* messageBody, size_t messageBodyLen) {
    unsigned char* lenInBytes = intToBytes(messageBodyLen);
    int i = 0;
    resultMessage[i++] = type;
    int k;
    for (k = 0; k < 4; ++k) {
        resultMessage[i++] = lenInBytes[k];
    }

    for (k = 0; k < messageBodyLen; ++k) {
        resultMessage[i++] = messageBody[k];
    }
    free(lenInBytes);
    return (size_t) i;
}

void recognizeMessage(char* message, size_t* len, char* type, char* messageBody) {
    unsigned int i = 0;
    *type = message[i++];
    unsigned char* lenInBytes = (unsigned char*) malloc(5);
    size_t j;
    for (j = 0; j < 4; ++j) {
        lenInBytes[j] = (unsigned char) message[i++];
    }
    size_t messageBodyLen = bytesToInt(lenInBytes);
    for (j = 0; j < messageBodyLen; ++j) {
        messageBody[j] = (unsigned char) message[i++];
    }
    messageBody[messageBodyLen] = 0;
    *len = messageBodyLen;

    free(lenInBytes);
}
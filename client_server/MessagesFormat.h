//
// Created by salamantos on 25.04.17.
// Describes the format of messages
//

#pragma once

#include <stdio.h>
#include "Common.h"

unsigned char* intToBytes( unsigned long number );

size_t bytesToInt( unsigned char* bytes );

char* timeStampToStr( char* bytes );

void formMessageBody( unsigned char* messageBody, size_t* messageBodyLen, char* lines[], size_t linesCount );

void getLinesList( unsigned char* messageBody, size_t messageBodyLen, char* lines[], size_t* linesCount );

size_t
formMessage( unsigned char* resultMessage, unsigned char type, unsigned char* messageBody, size_t messageBodyLen );

void recognizeMessage( char* message, size_t* len, char* type, char* messageBody );

unsigned char* getTimeStamp();

void prepareUsersList( char* messageBody, size_t* messBSize, int* onlineCount, struct CUser* usersList );
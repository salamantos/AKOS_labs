//
// Created by salamantos on 25.04.17.
// Describes the format of messages
//

#pragma once

#include <stdio.h>
#include "Common.h"

unsigned char* intToBytes( unsigned long number );

size_t bytesToInt( unsigned char* bytes );

void bytesToDoubleInt( unsigned char* bytes, int* number1, int* number2 );

size_t
formMessage( unsigned char* resultMessage, unsigned char type, unsigned char* messageBody, size_t messageBodyLen );

void recognizeMessage( char* message, size_t* len, char* type, char* messageBody );

unsigned char* getTimeStamp();

void prepareUsersList( char* messageBody, size_t* messBSize, int* onlineCount, struct CUser* usersList);
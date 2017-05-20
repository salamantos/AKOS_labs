//
// Created by salamantos on 01.05.17.
//

#include <stdio.h>
#pragma once

int writeToHistory( char* message, size_t len );

int readFromHistory( char* message, size_t* len, long int* pos );
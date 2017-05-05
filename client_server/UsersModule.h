//
// Created by salamantos on 01.05.17.
// Realisation of users data logic
//

#pragma once

#include <stdio.h>

char* hash( char* str );

int isCorrect( char* login, char* password );

int isUserExist( char* login );

int createUser( char* login, char* password, int isKicked );

int authentication( char* login, char* password );

int kick( char* login );

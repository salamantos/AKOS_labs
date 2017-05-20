//
// Created by salamantos on 01.05.17.
// Realisation of users data logic
//

#pragma once

#include <stdio.h>
#include "Common.h"

char* hash( char* str );

int isCorrect( char* login, char* password );

int isUserExist( struct CUser* usersList, char* login );

int createUser( struct CUser* usersList, char* login, char* password, int isKicked, int sockfd );

int authentication( struct CUser* usersList, char* login, char* password, int sockfd );

int kick( struct CUser* usersList, int id, int* kickedSock, int* isOnline );

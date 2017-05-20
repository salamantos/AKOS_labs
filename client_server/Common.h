//
// Created by salamantos on 07.05.17.
//

#pragma once

#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>

#define portno 1338
#define MESSAGE_LEN 1000
#define ROOT_PASSWORD "pass"
#define MAX_CONNECTED_USERS 100

int userIdRandom;

struct CMessage {
    char* mess;
    size_t len;
};

struct CUser {
    char* login;
    char* password;
    int id;
    int isOnline;
    int isKicked;
    int sockfd;
};

struct CThreadParam {
    int newsockfd;
    struct CMessage* messBuffer;
    int* lastMessId;
    int* onlineCount;
    struct CUser* usersList;
    sem_t* semaphore;
};

struct CUser* nullUser;

void error( const char* msg );

void* newThread( void* );

int addUserToList( struct CUser* usersList, struct CUser* userInfo );

int rmUserFromList( struct CUser* usersList, char* login );

void sendToAll( char* message, size_t messSize, int onlineCount, struct CUser* usersList );
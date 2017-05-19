//
// Created by salamantos on 07.05.17.
//

#pragma once

#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>

#define portno 8080
#define MESSAGE_LEN 1000
#define ROOT_PASSWORD "pass"

void error( const char* msg );

void* newThread( void* );

struct CMessage {
    char* mess;
    size_t len;
};

struct CThreadParam {
    int* newsockfd;
    struct CMessage* messBuffer;
    int* lastMessId;
    sem_t* semaphore;
};
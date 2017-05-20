//
// Created by salamantos on 05.05.17.
//

#pragma once

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct CSharedData {
    size_t online;
    char* onlineUsers[100];
};

int writeToSharedMemory( struct CSharedData* data );

int readFromSharedMemory( struct CSharedData* data );
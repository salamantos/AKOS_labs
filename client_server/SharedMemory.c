//
// Created by salamant on 05.05.17.
//

#include "SharedMemory.h"

// We'll name our shared memory segment "5678".
key_t key = 5555;

int writeToSharedMemory( struct CSharedData* data ) {
    int shmid;
    struct CSharedData* shm;

    // Create the segment.
    if ((shmid = shmget( key, sizeof( struct CSharedData ), IPC_CREAT | 0666 )) < 0) {
        perror( "shmget" );
        return 1;
    }

    // Now we attach the segment to our data space.
    if ((shm = shmat( shmid, NULL, 0 )) == (struct CSharedData*) -1) {
        perror( "shmat" );
        return 1;
    }

    // Now put some things into the memory for the other process to read.
    *shm = *data;

    return 0;
}

int readFromSharedMemory( struct CSharedData* data ) {
    int shmid;
    struct CSharedData* shm;

    // Create the segment.
    if ((shmid = shmget( key, sizeof( struct CSharedData ), IPC_CREAT | 0666 )) < 0) {
        perror( "shmget" );
        return 1;
    }

    // Now we attach the segment to our data space.
    if ((shm = shmat( shmid, NULL, 0 )) == (struct CSharedData*) -1) {
        perror( "shmat" );
        return 1;
    }

    // Now get some things into the memory for the other process to read.
    *data = *shm;

    return 0;
}

//main() {
//    char c;
//    int shmid;
//    key_t
//    key;
//    char* shm, * s;
//
//    // We'll name our shared memory segment "5678".
//    key = 5678;
//
//    // Create the segment.
//    if ((shmid = shmget( key, sizeof( struct CSharedData ), IPC_CREAT | 0666 )) < 0) {
//        perror( "shmget" );
//        exit( 1 );
//    }
//
//    // Now we attach the segment to our data space.
//    if ((shm = shmat( shmid, NULL, 0 )) == (char*) -1) {
//        perror( "shmat" );
//        exit( 1 );
//    }
//
//    // Now put some things into the memory for the other process to read.
//    s = shm;
//
//    for (c = 'a'; c <= 'z'; c++)
//        *s++ = c;
//    *s = NULL;
//
//    // Finally, we wait until the other process changes the first character of our memory
//    // to '*', indicating that it has read what we put there.
//    while (*shm != '*')
//        sleep( 1 );
//
//    exit( 0 );
//}
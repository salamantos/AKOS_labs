//
// Created by salamantos on 07.05.17.
//

#include <stdio.h>
#include <stdlib.h>
#include "MessagesFormat.h"
#include "UsersModule.h"
#include "test/SharedMemory.h"
#include <dirent.h>
#include "test/MessagesReceiving.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include "Common.h"

#pragma once

int connectNewUser( struct CThreadParam* param );
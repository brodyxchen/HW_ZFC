#pragma once
#include <stdio.h>     // sprintf
#include <stdarg.h>
#include <tchar.h>
#include <time.h>
#include <process.h>

#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include "udp_api.h"
#include "Game.h"

using namespace std;

//³£Á¿ºêÉùÃ÷
#define BUFLEN 256
#define BUFLEN1024 1024

#define log printf

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

#ifndef ERROR
#define ERROR (-1)
#endif
#define OK 0

extern string teamName;
extern string myRole; //¡°POL¡± OR ¡°THI¡±
extern ushort localPort;
extern Game game;



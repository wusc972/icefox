/*
 * Original author: Xiaoxia
 */

#include "httpsocket.h"
#include "youconfig.h"
#include "httpserver.h"
#include <signal.h>
#include <iostream>
#include <ctime>

using namespace std;

int main(int argc, char *argv[])
{
#ifndef __WIN32__
    signal(SIGPIPE, SIG_IGN);
#else
    static WSADATA wsa_data;
    if(WSAStartup((WORD)(1<<8|1), &wsa_data) != 0)
        exit(1);
#endif
    YouConfig::instance()->loadFromNetwork();
    HttpServer* server = new HttpServer();
    cout << "Start proxy server at localhost:1998\n";
    server->startThread();

    while(1){
        sleep(1);
    }
}

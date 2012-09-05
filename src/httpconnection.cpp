#include "httpconnection.h"
#include "dnscache.h"
#include "youconfig.h"
#include <iostream>
#include <cstdio>
using namespace std;

HttpConnection::~HttpConnection()
{
    delete this->request;
    delete this->response;
    if(this->clientSocket)
        delete this->clientSocket;
    if(this->remoteSocket)
        delete this->remoteSocket;
}

HttpConnection::HttpConnection(HttpSocket* __sock)
{
    this->clientSocket = __sock;
    this->remoteSocket = 0;
    this->request = new HttpRequest();
    this->response = new HttpResponse();
    this->closed = false;
    this->addNewLine = false;
}

void HttpConnection::process()
{
    string host = "";
    int port = 0;
    for(int c=1; !this->closed; c++){
        if(!this->request->parseRequest(this->clientSocket))
            break;

        cout << this->request->getCommand() << " " << this->request->getPath() <<
                " " << this->request->getHost() <<  endl;

        /* Check if need to re-establish connection */
        if(host != this->request->getHost() || port != this->request->getPort()){
            port = this->request->getPort();
            host = parseConnectHost(this->request->getHost());
            if(this->remoteSocket)
                delete this->remoteSocket;
            this->remoteSocket = 0;
        }

        /* Connect to remote server */
        if(this->remoteSocket == 0){
            this->remoteSocket = new HttpSocket();

            cout << "Connecting to " << host << ":" << port << "," << this->addNewLine << endl;
            if(!this->remoteSocket->connect(host, port)){
                DnsCache::instance()->releaseItem(this->request->getHost());
                break;
            }
            if(this->request->getCommand() != "CONNECT" && this->addNewLine)
                this->remoteSocket->send(string("\r\n"));
        }

        /* Do CONNECT */
        if(this->request->getCommand() == "CONNECT"){
            this->clientSocket->send("HTTP/1.1 200 Established\r\n");
            this->doCONNECT();
            break;
        }

        /* Retry */
        if(!this->request->sendRequest(this->remoteSocket)){
            delete this->remoteSocket;
            this->remoteSocket = new HttpSocket();
            if(!this->remoteSocket->connect(host, port)){
                DnsCache::instance()->releaseItem(this->request->getHost());
                break;
            }

            if(this->request->getCommand() != "CONNECT" && this->addNewLine)
                this->remoteSocket->send(string("\r\n"));
            if(!this->request->sendRequest(this->remoteSocket))
                break;
        }

        /* Get Response */
        if(!this->response->parseResponse(this->remoteSocket))
            break;

        if(this->addNewLine && this->response->getStatus() != "200"){
            cout << "Get response code: " << this->response->getStatus() << endl;
        }
        /* Send Response */
        if(!this->response->sendResponse(this->clientSocket))
            break;

        /* Response Data */
        if(this->response->getContentLength() > 0){
            if(this->response->isChunked()){
                this->transferTrunkedData();
            }else{
                this->transferData();
            }
        }

        /* 
         * Fix me!!!
         * Cannot handle more than one requests each connection.
         * At the present, force to close the connection immediately 
         */
        if(this->response->wouldClose() || this->request->wouldClose() || true)
            break;
    }

    if(this->clientSocket){
        delete this->clientSocket;
        this->clientSocket = 0;
    }
    if(this->remoteSocket){
        delete this->remoteSocket;
        this->remoteSocket = 0;
    }
    this->closed = true;
}

void HttpConnection::doCONNECT()
{
    const int bufferSize = 8192;
    char* buffer = new char[bufferSize];
    if(buffer == NULL)
        return;
    fd_set fdreads;
    int sock1 = this->clientSocket->getSocketHandle();
    int sock2 = this->remoteSocket->getSocketHandle();
    int ret, size;
    for(;!this->closed;){
        FD_ZERO(&fdreads);
        FD_SET(sock1, &fdreads);
        FD_SET(sock2, &fdreads);
        ret = select((sock1>sock2 ? sock1:sock2)+1, &fdreads, 0, 0, 0);
        if(ret <= 0)
            break;
        else{
            if(FD_ISSET(sock1, &fdreads)){
                size = recv(sock1, buffer, bufferSize, 0);
                if(size <= 0)
                    break;
                send(sock2, buffer, size, 0);
            }
            if(FD_ISSET(sock2, &fdreads)){
                size = recv(sock2, buffer, bufferSize, 0);
                if(size <= 0)
                    break;
                send(sock1, buffer, size, 0);
            }
        }
    }
    delete[] buffer;
}

void HttpConnection::close()
{
    if(this->clientSocket){
        this->clientSocket->shutdown();
    }
    if(this->remoteSocket){
        this->remoteSocket->shutdown();
    }
}

bool HttpConnection::isClosed() const
{
    return this->closed;
}


bool HttpConnection::transferData()
{
    unsigned int length = this->response->getContentLength();
    const unsigned int bufferSize = 8192;
    char* buffer = new char[bufferSize];
    if(buffer == NULL)
        return false;
    while(length > 0){
        int sendSize = length < bufferSize ? length : bufferSize;
        if((sendSize = this->remoteSocket->receive(buffer, sendSize)) <= 0)
            break;
        if(this->clientSocket->send(buffer, sendSize) != sendSize)
            break;
        length -= sendSize;
    }
    delete[] buffer;
    return true;
}

bool HttpConnection::transferTrunkedData()
{
    const unsigned int bufferSize = 8192;
    char* buffer = new char[bufferSize];
    if(buffer == NULL)
        return false;
    string line;
    bool end = false;
    for(;!end;){
        /* read trunk size */
        if(!this->remoteSocket->receive(line))
            break;

        unsigned int length = 0;
        sscanf(line.c_str(), "%x", &length);
        if(length == 0)
            end = true;

        while(length > 0){
            int sendSize = length < bufferSize ? length : bufferSize;
            if((sendSize = this->remoteSocket->receive(buffer, sendSize)) <= 0)
                break;
            if(this->clientSocket->send(buffer, sendSize) != sendSize)
                break;
            length -= sendSize;
        }

        /* read endline */
        if(!this->remoteSocket->receive(line))
            break;
        if(!line.empty())
            break;
    }

    delete[] buffer;
    return true;
}


HttpRequest* HttpConnection::getRequest()
{
    return this->request;
}

HttpResponse* HttpConnection::getResponse()
{
    return this->response;
}

string HttpConnection::parseConnectHost(const string &host)
{
    int fuck = 0;
    string ip = YouConfig::instance()->getAddressByHost(host, &fuck);
    if(fuck)
        this->addNewLine = true;
    return ip;
}


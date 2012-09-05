#include "youconfig.h"
#include "dnscache.h"
#include "httpsocket.h"
#include "cppthread.h"
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>

using namespace std;

static YouConfig* config = 0;

class DownloadConfig: public CppThread{
public:
    void process(){
        cout << "Downloading config file from web...\n";
        HttpSocket * http = new HttpSocket();
        if(!http->connect("icefox.googlecode.com", 80)){
            cerr << "Failed to connect to icefox.googlecode.com" << endl;
            return;
        }

        http->send("GET /svn/trunk/yhosts HTTP/1.0\r\n"
                   "Host: icefox.googlecode.com\r\n"
                   "Connection: close\r\n");
        string line;
        while(http->receive(line)){
            if(line.empty())
                break;
        }
        char* ptr = new char[1024 * 1024 + 1];
        int pos = 0;
        while(true){
            int ret = http->receive(ptr + pos, 1024*1024 - pos);
            if(ret <= 0)
                break;
            pos += ret;
        }
        ptr[pos] = '\0';
        cout << "Read http config content:\n" << ptr << endl;
        YouConfig::instance()->parseConfig(ptr);
        delete[] ptr;
        delete http;
    }
};

YouConfig::YouConfig()
{
    pthread_mutex_init(&this->lock, NULL);
    loadDefaults();
}

void YouConfig::loadFromNetwork()
{
    DownloadConfig * config = new DownloadConfig();
    config->startThread();
}

void YouConfig::loadDefaults()
{
    const static char* defaults =
            "plus.google.com             ip,203.208.45.206\n"
            "google.com                  ip,203.208.46.161\n"
            "googleusercontent.com       ip,203.208.46.170\n"
            "gstatic.com                 ip,203.208.46.175\n"
            "ggpht.com                   ip,203.208.46.170\n"
            "appspot.com                 ip,203.208.46.161\n"
            "blogspot.com                ip,203.208.46.161\n"
            "blogger.com                 ip,203.208.46.161\n"
            "googleapis.com              ip,203.208.46.161\n"
            "googlecode.com              ip,203.208.46.161\n"
            "google.com.hk               ip,203.208.46.161\n"
            "www.facebook.com            ip,66.220.158.70,fuck\n"
            "facebook.com                dns,fuck\n"
            "fbcdn.net                   dns,fuck\n"
            "dropbox.com                 dns,fuck\n"
            "wikipedia.com               dns,fuck\n"
            "www.youtube.com             ip,203.208.45.206\n"
            "ytimg.com                   ip,203.208.45.206\n"
            "youtube.com                 dns,fuck\n"
            "vimeo.com                   dns,fuck\n"
            "vimeocdn.com                dns,fuck\n";
    parseConfig(defaults);
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

void YouConfig::parseConfig(string content)
{
    cout << "Loading config ...\n";

    this->lockConfig();
    ruleList.clear();

    vector<string> lines;
    split(content, '\n', lines);

    for(size_t i=0; i<lines.size(); i++){
        string line = lines[i];
        if(line.length()<1 || line[0]=='#')
            continue;
        YouRule r;
        memset(&r, 0, sizeof(r));

        char* tmp = new char[line.length() + 1];
        memcpy(tmp, line.data(), line.length());
        tmp[line.length()] = '\0';
        char* p = strtok(tmp, " ");

        strcpy(r.suffix, p);
        while((p = strtok(NULL, " ,\r"))){
            if(strcmp(p, "dns") == 0){
                r.dns = 1;
            }else if(strcmp("fuck", p) == 0){
                r.fuck = 1;
            }else if(strcmp("ip", p) == 0){
                p = strtok(NULL, " ,");
                strcpy(r.ip, p);
            }
        }
        ruleList.push_back(r);
    }
    this->unlockConfig();
    cout << "Loaded" << ruleList.size() << "rules.\n";
}

YouConfig* YouConfig::instance()
{
    if(config != 0){
        return config;
    }
    config = new YouConfig();
    return config;
}

string YouConfig::getAddressByHost(const string &host, int * fuck)
{
    this->lockConfig();

    string targetIP;
    int dns = 0;
    vector<YouRule>::iterator i;
    for (i = this->ruleList.begin(); i != this->ruleList.end(); ++i){
        YouRule& r = *i;
        if(host.find(r.suffix) != string::npos){
            *fuck = r.fuck;
            if(r.ip[0]){
                targetIP = r.ip;
            }
            dns = r.dns;
            break;
        }
    }

    this->unlockConfig();

    if(targetIP.empty()){
        if(dns){
            targetIP = DnsCache::instance()->queryA(host);
        }else{
            return host;
        }
    }
    return targetIP;
}

void YouConfig::lockConfig()
{
    pthread_mutex_lock(&this->lock);
}

void  YouConfig::unlockConfig()
{
    pthread_mutex_unlock(&this->lock);
}

#include "dnscache.h"
#include <iostream>
#include <cstring>

static DnsCache* dns = 0;

DnsCache::DnsCache()
{
    pthread_mutex_init(&this->lock, NULL);
    this->records.clear();

    this->udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(53);
    addr.sin_addr.s_addr = inet_addr("8.8.4.4");

    if(0 != ::connect(this->udpSocket, (sockaddr*)&addr, sizeof addr)){
        cerr << "Failed to connect to dns server." << endl;
    }
}

DnsCache* DnsCache::instance()
{
    if(dns)
        return dns;
    dns = new DnsCache();
    return dns;
}

string DnsCache::doQueryA(const string &host)
{
    char buffer[4000];
    static const char head[] = {0,1,1,0,0,2,0,0,0,0,0,0};
    static const char tail[] = {0,0,1,0,1};
    memcpy(buffer, head, sizeof(head));
    char* kd = buffer + sizeof(head);

    for(int i=0; i<2; i++){
        char tmp[host.length() + 2];
        strcpy(tmp, host.c_str());
        char *sb = strtok(tmp, ".");
        while(sb){
            int ls = strlen(sb);
            *kd = ls;
            kd += 1;
            memcpy(kd, sb, ls);
            kd += ls;
            sb = strtok(NULL, ".");
        }
        memcpy(kd, tail, sizeof tail);
        kd += sizeof tail;
    }

    size_t packetSize = size_t(kd - buffer);
    if(send(this->udpSocket, buffer, packetSize, 0) < 0){
        cerr << "Failed to send udp packet. ret < 0" << endl;
        return "";
    }

    for(;;){
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(this->udpSocket, &reads);
        timeval tv;
        tv.tv_sec = 3;
        tv.tv_usec = 0;
        int ret = select(this->udpSocket + 1, &reads, 0, 0, &tv);
        if(ret <= 0){
            cerr << "Failed to parse host " << host.c_str() << endl;
            return "";
        }

        ret = recv(this->udpSocket, buffer, 4000, 0);
        if(ret <= 10){
            cerr << "Failed to recv udp packet, ret=" << ret << endl;
            return "";
        }

        if(buffer[7] < 1){
            // no answers found.
            cerr << "No answer returned from dns server." << endl;
            continue;
        }

        in_addr ip;
        memcpy(&ip.s_addr, &buffer[ret - 4], 4);
        return inet_ntoa(ip);
    }
    return "";
}

string DnsCache::queryA(const string &host)
{
    string targetIP = "";
    pthread_mutex_lock(&this->lock);

    if(this->records.find(host) != this->records.end()){
        targetIP = this->records[host];
    }else{
        targetIP = this->doQueryA(host);
        this->records[host] = targetIP;
        cout << "dns resolve " << host << " => " << targetIP << endl;
    }

    pthread_mutex_unlock(&this->lock);
    return targetIP;
}

void DnsCache::releaseItem(const string &ip)
{
    pthread_mutex_lock(&this->lock);
    this->records.erase(ip);
    pthread_mutex_unlock(&this->lock);
}

#include <iostream>
#include <cstring>
#include "dnscache.h"
#include "dnsquery.h"

static const char* DNS_SERVER = "8.8.4.4";
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
    addr.sin_addr.s_addr = inet_addr(DNS_SERVER);

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
    DnsQuery query;
    query.setNameServer(DNS_SERVER);
    if(!query.connect()){
        cerr << "Failed to connect to dns server." << endl;
        return host;
    }
    if(!query.queryA(host)){
        cerr << "Failed to query domain: " << query.getErrorString() << endl;
        return host;
    }
    return query.getARecord();
    
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

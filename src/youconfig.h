#ifndef YOUCONFIG_H
#define YOUCONFIG_H

#include <string>
#include <vector>
#include <pthread.h>
using namespace std;

struct YouRule{
    char suffix[100];
    char ip[50];
    char fuck;
    char dns;
};

class YouConfig
{
private:
    YouConfig();
    vector<YouRule> ruleList;
    pthread_mutex_t lock;

    void lockConfig();
    void unlockConfig();
public:
    static YouConfig* instance();

    string getAddressByHost(const string& host, int* fuck);
    void loadFromNetwork();
    void loadDefaults();
    void parseConfig(string content);

};

#endif // YOUCONFIG_H

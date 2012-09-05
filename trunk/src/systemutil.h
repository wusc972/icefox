#ifndef SYSTEMUTIL_H
#define SYSTEMUTIL_H

#include <QString>

class SystemUtil
{
public:
    SystemUtil();
    static void setSystemProxy(QString proxyServer, int proxyPort);
    static void enableSystemProxy();
    static void disableSystemProxy();
};

#endif // SYSTEMUTIL_H

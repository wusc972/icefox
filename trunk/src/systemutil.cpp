#include "systemutil.h"
#include <QString>
#include <QtDebug>
#include <QUdpSocket>
#include <wininet.h>
#include <windows.h>

typedef struct {
  DWORD dwOption;
  union {
    DWORD    dwValue;
    LPTSTR   pszValue;
    FILETIME ftValue;
  } Value;
} INTERNET_PER_CONN_OPTION, *LPINTERNET_PER_CONN_OPTION;


typedef struct {
  DWORD                      dwSize;
  LPTSTR                     pszConnection;
  DWORD                      dwOptionCount;
  DWORD                      dwOptionError;
  LPINTERNET_PER_CONN_OPTION pOptions;
} INTERNET_PER_CONN_OPTION_LIST, *LPINTERNET_PER_CONN_OPTION_LIST;

//// Options used in INTERNET_PER_CONN_OPTON struct
#define INTERNET_PER_CONN_FLAGS                         1
#define INTERNET_PER_CONN_PROXY_SERVER                  2
#define INTERNET_PER_CONN_PROXY_BYPASS                  3
#define INTERNET_PER_CONN_AUTOCONFIG_URL                4
#define INTERNET_PER_CONN_AUTODISCOVERY_FLAGS           5
//etc.
//// PER_CONN_FLAGS
#define PROXY_TYPE_DIRECT                               0x00000001   // direct to net
#define PROXY_TYPE_PROXY                                0x00000002   // via named proxy
#define PROXY_TYPE_AUTO_PROXY_URL                       0x00000004   // autoproxy URL
#define PROXY_TYPE_AUTO_DETECT                          0x00000008   // use autoproxy detection
#define INTERNET_OPTION_PER_CONNECTION_OPTION   75

BOOL SetConnectionProxy(const char * proxyAdressStr , char * connNameStr = NULL)
{
    INTERNET_PER_CONN_OPTION_LIST conn_options;
    BOOL    bReturn;
    DWORD   dwBufferSize = sizeof(conn_options);
    conn_options.dwSize = dwBufferSize;
    conn_options.pszConnection = (TCHAR*)connNameStr;//NULL == LAN

    conn_options.dwOptionCount = 3;
    conn_options.pOptions = new INTERNET_PER_CONN_OPTION[3];

    if(!conn_options.pOptions)
        return FALSE;

    conn_options.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    conn_options.pOptions[0].Value.dwValue = PROXY_TYPE_DIRECT|PROXY_TYPE_PROXY;

    conn_options.pOptions[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    conn_options.pOptions[1].Value.pszValue = (TCHAR*)proxyAdressStr;
    conn_options.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    conn_options.pOptions[2].Value.pszValue = (TCHAR*)"<local>";

    bReturn = InternetSetOptionA(NULL,INTERNET_OPTION_PER_CONNECTION_OPTION, &conn_options, dwBufferSize);

    delete [] conn_options.pOptions;

    InternetSetOptionA(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    // InternetSetOption(NULL, INTERNET_OPTION_REFRESH , NULL, 0);
    return bReturn;
}

BOOL RemoveConnectionProxy(char* connectionNameStr = NULL)
{
    INTERNET_PER_CONN_OPTION_LIST conn_options;
    BOOL    bReturn;
    DWORD   dwBufferSize = sizeof(conn_options);

    conn_options.dwSize = dwBufferSize;

    conn_options.pszConnection = (TCHAR*)connectionNameStr; //NULL - LAN
    conn_options.dwOptionCount = 1;
    conn_options.pOptions = new INTERNET_PER_CONN_OPTION[conn_options.dwOptionCount];

    if(!conn_options.pOptions)
        return FALSE;

    conn_options.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    conn_options.pOptions[0].Value.dwValue = PROXY_TYPE_DIRECT  ;

    bReturn = InternetSetOptionA(NULL,INTERNET_OPTION_PER_CONNECTION_OPTION, &conn_options, dwBufferSize);

    delete [] conn_options.pOptions;
    InternetSetOptionA(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    InternetSetOptionA(NULL, INTERNET_OPTION_REFRESH , NULL, 0);

    return bReturn;
}


SystemUtil::SystemUtil()
{
}

void SystemUtil::disableSystemProxy()
{
    int ret = RemoveConnectionProxy();
    qDebug() << "RemoveConnectionProxy result" << ret << GetLastError();
}


void SystemUtil::enableSystemProxy()
{
}

void SystemUtil::setSystemProxy(QString proxyServer, int proxyPort)
{
    QString proxy = proxyServer + ":" + QString::number(proxyPort);
    int ret = SetConnectionProxy(proxy.toStdString().c_str());
    qDebug() << "SetConnectionProxy result" << ret << GetLastError();
}

CC= g++
CFLAGS= -O2 -s -Wall -D__WIN32__  -mwindows -D_UNICODE -D_UNICODE_
LDFLAGS= -I. -I./posixthread/include -L./posixthread/lib/x86 -lpthreadGC2 -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lwinspool -lwinmm -lshell32 -lcomctl32 -lole32 -loleaut32 -luuid -lrpcrt4 -ladvapi32 -lwsock32 -lodbc32 -lWininet
OBJS= cppthread.o dnscache.o httpconnection.o httpmessage.o httprequest.o httpresponse.o main.o httpserver.o httpsocket.o youconfig.o systemutil.o 


all: YouProxy

YouProxy: $(OBJS) res.o
	$(CC) $(CFLAGS) -o $@ $(OBJS) res.o  $(LDFLAGS)



$(OBJS):%.o:%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<  $(LDFLAGS)

res.o:main.rc Logo.ico
	windres -imain.rc -o $@

.PHONY: clean
clean:
#-rm -rf youagent
#-rm -rf *.o
	-del -rf YouProxy.exe
	-del -rf *.o

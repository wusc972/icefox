CC= g++
CFLAGS= 
LDFLAGS=
OBJS= cppthread.o dnscache.o httpconnection.o httpmessage.o httprequest.o httpresponse.o main.o httpserver.o httpsocket.o youconfig.o


all: youagent

youagent: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

.o: .cpp
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -rf youagent
	rm -rf *.o

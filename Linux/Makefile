CC= g++
CFLAGS= -O2 -s -Wall  -D_UNICODE -D_UNICODE_
SRC_DIR=./../src
LDFLAGS= -I. -I$(SRC_DIR) -lpthread
OBJS= main.o $(filter-out  $(SRC_DIR)/systemutil.%, $(patsubst %.cpp,%.o, $(wildcard $(SRC_DIR)/*.cpp)))

all: YouProxy

YouProxy: $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $(OBJS)  $(LDFLAGS)



$(OBJS):%.o:%.cpp
#echo "OBJS: $(OBJS)"
	$(CC) $(CFLAGS) -c -o $@ $<  $(LDFLAGS)


.PHONY: clean
clean:
	-rm -rf ./YouProxy
	-rm -rf ./../src/*.o
	-rm *.o
	-rm *~

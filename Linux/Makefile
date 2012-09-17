CC= g++
CFLAGS= -O2 -s -Wall  -D_UNICODE -D_UNICODE_
LFLAGS = -I. -I$(SRC_DIR) -lpthread

SRC_DIR=./../src

OBJS= main.o $(filter-out  $(SRC_DIR)/systemutil.%, $(patsubst %.cpp,%.o, $(wildcard $(SRC_DIR)/*.cpp)))

##Options 

#BITS
ifeq ($(BITS),32)
    CFLAGS += -m32
    LFLAGS += -m32
else
    ifeq ($(BITS),64)
        CFLAGS += -m64
        LFLAGS += -m64
    endif
endif


all: YouProxy

YouProxy: $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $(OBJS)  $(LFLAGS)

$(OBJS):%.o:%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<  $(LFLAGS)


.PHONY: clean
clean:
	-rm -rf ./YouProxy*
	-rm -rf ./../src/*.o
	-rm *.o
	-rm *~

SOURCE  := $(wildcard base/*.cc net/*.cc net/poller/*.cc)
OBJS     := $(patsubst %.cc,%.o,$(SOURCE))
CC       := g++
LIBS     := -lpthread
INCLUDE  := -I./usr/local/lib 
CFLAGS   := -fPIC -std=c++11 -g -Wall -O3 -D_PTHREADS
CXXFLAGS := $(CFLAGS)

SUBTARGET1 := Server
SUBTARGET2 := Client

.PHONY : all debug clean veryclean rebuild
all : $(SUBTARGET1) $(SUBTARGET2) clean
clean:
	find . -name '*.o' | xargs rm -f
veryclean:
	find . -name '*.o' | xargs rm -f
	find . -name 'Server' | xargs rm -f
	find . -name 'Client' | xargs rm -f

rebuild: veryclean all

debug:
	echo $(SOURCE)

$(SUBTARGET1) : $(OBJS) ChatServer/ServerMain.o
	$(CC) $(CXXFLAGS) -o $@ $^ $(LIBS)  
$(SUBTARGET2) : $(OBJS) ChatClient/ClientMain.o
	$(CC) $(CXXFLAGS) -o $@ $^ $(LIBS) 



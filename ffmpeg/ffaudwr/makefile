#Author :Rupesh Sreeraman
#This file is a part of rlab

CC =g++
SOURCES =main.cpp
CFLAGS= -I ../include  
LDFLAGS=-L ../lib  -lmingw32 -lSDLmain -lSDL -lavformat -lavcodec -lavutil -lswresample 
BINDIR=../bin/
EXECUTABLE =$(BINDIR)ffaudwr.exe

OBJECTS	:= $(SOURCES:.cpp=.o)

all:$(OBJECTS)
	$(CC) -o $(EXECUTABLE) $(OBJECTS) $(LDFLAGS) 

 %.o: %.cpp
	$(CC) $(CFLAGS) -o $(OBJECTS) -c $(SOURCES) 

clean:
	rm -f $(EXECUTABLE) *.o

rebuild: clean all


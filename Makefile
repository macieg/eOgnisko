MFLAGS=-std=c++11 -Wall -lboost_system -lboost_program_options -lpthread -lboost_regex -g
DFLAGS=-std=c++11 -Wall -c
K=klient
S=serwer
C=connection
SC=server
SA=server_attributes
KC=client
M=mixer
P=parser

O=.o
CPP=.cpp
H=.h

all: $(S) $(K)

$(K): $(K)$(CPP) $(KC)$(O) $(P)$(O)
	g++ $(K)$(CPP) $(KC)$(O) $(P)$(O) -o $(K) $(MFLAGS)

$(S): $(S)$(CPP) $(SA)$(O) $(M)$(O) $(C)$(O) $(SC)$(O) $(P)$(O)
	g++ $(S)$(CPP) $(SA)$(O) $(M)$(O) $(C)$(O) $(SC)$(O) $(P)$(O) -o $(S) $(MFLAGS)

%$(O): %$(CPP)
	g++ -c $< $(DFLAGS)

clean:
	rm -f $(K) $(S)
	rm -f *.o
	rm -f *~

MFLAGS=-std=c++11 -Wall -lboost_system -lboost_program_options -lpthread -lboost_regex
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

$(KC)$(O): $(KC)$(CPP) $(KC)$(H)
	g++ $(DFLAGS) $(KC)$(CPP) -o $(KC)$(O)
	
$(SC)$(O): $(SC)$(CPP) $(SC)$(H)
	g++ $(DFLAGS) $(SC)$(CPP) -o $(SC)$(O)
	
$(M)$(O): $(M)$(CPP) $(M)$(H)
	g++ $(DFLAGS) $(M)$(CPP) -o $(M)$(O)

$(C)$(O): $(C)$(CPP) $(C)$(H) 
	g++ $(DFLAGS) $(C)$(CPP) -o $(C)$(O)

$(P)$(O): $(P)$(CPP) $(P)$(H)
	g++ $(DFLAGS) $(P)$(CPP) -o $(P)$(O)
	
$(SA)$(O): $(SA)$(CPP) $(SA)$(H)
	g++ $(DFLAGS) $(SA)$(CPP) -o $(SA)$(O)

clean:
	rm -f $(K) $(S)
	rm -f *.o
	rm -f *~

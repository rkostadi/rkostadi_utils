CC = g++ -std=c++11
#CC = g++ -std=c++0x
CCFLAGS = -O2 -g -Wall
LDFLAGS = -lgsl -lgslcblas -lm

# Programs
sample_at_random.o: sample_at_random.cpp
	${CC} ${CCFLAGS} ${LDFLAGS} -c sample_at_random.cpp
sample_at_random: check-env sample_at_random.o
	${CC} ${CCFLAGS} ${LDFLAGS} -o ./bin/$@ sample_at_random.o

# All programs
PROGRAMS = sample_at_random

all: check-env $(PROGRAMS)

check-env:
	mkdir -p ./bin

clean:
	-rm -f *.o
	-cd bin && rm -f $(PROGRAMS)

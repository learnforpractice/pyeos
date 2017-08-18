#
# makefile
# Make captest Executable
#

CLEANFILES = core *.o *.out *.exe

CXX = g++ -w -g -std=gnu++14 -O3 -Wall -lpthread

INC_DIRS = 

SRC    =  captest.cpp
OBJ    = ${SRC:.cpp=.o}

.SUFFIXES:
.SUFFIXES: 	.cpp .o

.cpp.o:
		@echo
		@echo   Building $@
		${CXX} ${INC_DIRS} -c -o $@ $<

all:		captest
allWithCopy:	captest UpdateShares

captest:	${OBJ}
		@echo Building captest Executable
		#${CXX} -o captest.exe ${OBJ} -lpthread
		${CXX} -o captest ${OBJ} -lpthread

install:	UpdateShares

UpdateShares:	captest

clean:
		rm -rf ${CLEANFILES}

depend:
		${CXX} -M ${INC_DIRS} ${SRC}



# DO NOT DELETE


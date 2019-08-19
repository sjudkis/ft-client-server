XX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic-errors -g

PRGM = ftserver

OBJS = ftserver.o Server.o
SRCS = ftserver.cpp Server.cpp
HDRS = Server.hpp



${PRGM}: ${OBJS}
	${CXX} ${CXXFLAGS} ${OBJS} -o ${PRGM}

${OBJS}: ${SRCS}
	${CXX} ${CXXFLAGS} -c $(@:.o=.cpp)


clean:
	rm *.o ${PRGM}

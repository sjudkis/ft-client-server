XX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic-errors -fopenmp -g

PRGM = ftserver

OBJS = ftserver.o Server.o Socketft.o
SRCS = ftserver.cpp Server.cpp Socketft.cpp
HDRS = Server.hpp Socketft.hpp



${PRGM}: ${OBJS}
	${CXX} ${CXXFLAGS} ${OBJS} -o ${PRGM}

${OBJS}: ${SRCS}
	${CXX} ${CXXFLAGS} -c $(@:.o=.cpp)


clean:
	rm *.o ${PRGM}

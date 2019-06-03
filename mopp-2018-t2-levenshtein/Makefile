CC = g++

.PHONY = all clean

all : lev

clean :
	rm -rf lev

lev : lev_dist.cc
	${CC} -g -O0 lev_dist.cc -pthread -o lev


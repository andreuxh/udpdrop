all: ../lib/libudpdrop.so

../lib/libudpdrop.so: udpdrop.cc
	g++ -shared -fPIC -std=c++0x -o $@ $^ -ldl -lm

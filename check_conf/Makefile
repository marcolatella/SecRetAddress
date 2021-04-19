all: test

test: test.cpp
	g++ -std=gnu++11 -I`gcc -print-file-name=plugin`/include -fPIC -shared -fno-rtti -O2 test.cpp -o test.so

	
check: test.c
	gcc -fplugin=./test.so test.c -o test.o
	
 clean: 
	rm -rf test.so test.o

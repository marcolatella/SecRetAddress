all: test

test: plugin.cpp
	g++ -std=gnu++11 -I`gcc -print-file-name=plugin`/include -fPIC -shared -fno-rtti -O2 plugin.cpp -o plugin.so

	
check: test.c
	gcc -fplugin=./test.so test.c -o test.o
	
 clean: 
	rm -rf plugin.so test.o

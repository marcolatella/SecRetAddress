YOURCFILE= toupdate
OUTPUTFILE= toupdate

plugin.so: plugin.cpp
	g++ -std=gnu++11 -I`gcc -print-file-name=plugin`/include -fPIC -shared -fno-rtti -O2 plugin.cpp -o plugin.so


.PHONY: check
check: plugin.so plugin.cpp
	g++ -fplugin=`pwd`/./plugin.so $(YOURCFILE) -x c++ /dev/null -o /dev/null

.PHONY: clean
clean:
	rm plugin.so

protect:
	gcc -fplugin=./plugin.so $(YOURCFILE) -o $(OUTPUTFILE) -w

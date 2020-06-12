all:
	g++ -std=c++11 -I/usr/include -I/usr/include/libindi -c perfectstarb.cpp
	#g++ -std=c++11 -rdynamic perfectstarb.o -o indi_perfectstarb -lindidriver -lrt
	g++ -std=c++11 -rdynamic perfectstarb.o -o indi_perfectstarb_focus -lindidriver

clean:
	rm -rf perfectstarb.o indi_perfectstarb_focus

install:
	\cp -f indi_perfectstarb_focus /usr/bin/
	\cp -f indi_perfectstarb.xml /usr/share/indi/


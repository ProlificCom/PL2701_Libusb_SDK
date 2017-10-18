CC=g++

All: transmit benchmark listdev 

transmit: transmit.cpp 
	$(CC) transmit.cpp -lusb-1.0 -lpthread -o $@

benchmark: benchmark.cpp 
	$(CC) benchmark.cpp -lusb-1.0 -lpthread -o $@

listdev: listdev.cpp
	$(CC) listdev.cpp -lusb-1.0 -o $@

clean:
	rm -f transmit
	rm -f benchmark
	rm -f listdev
	rm -f *.o

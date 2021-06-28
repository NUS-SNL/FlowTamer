include /usr/local/etc/PcapPlusPlus.mk

# All Target
all: pcap-parser

pcap-parser: pcap-parser.o
	g++ $(PCAPPP_LIBS_DIR) -static-libstdc++ -o pcap-parser pcap-parser.o $(PCAPPP_LIBS)
	
pcap-parser.o: pcap-parser.cpp
	g++ $(PCAPPP_BUILD_FLAGS) $(PCAPPP_INCLUDES) -c -o pcap-parser.o pcap-parser.cpp

# Clean Target
clean:
	rm pcap-parser.o
	rm pcap-parser

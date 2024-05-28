CC = gcc
CFLAGS = -g -c -Wall -Wextra -pedantic
LIBS = -lpcap

SRCS = ipk.c socket.c tcp_listener.c handle_tcp.c tcp_receiver.c udp_receiver.c udp_setup.c handle_udp.c converter.c
OBJS = ipk.o socket.o tcp_listener.o handle_tcp.o tcp_receiver.o udp_receiver.o udp_setup.o handle_udp.o converter.o
OUT = ipk24chat-server

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)

ipk.o: ipk.c
	$(CC) $(CFLAGS) ipk.c

tcp_listener.o: tcp_listener.c
	$(CC) $(CFLAGS) tcp_listener.c

handle_tcp.o: handle_tcp.c
	$(CC) $(CFLAGS) handle_tcp.c


tcp_receiver.o: tcp_receiver.c
	$(CC) $(CFLAGS) tcp_receiver.c

socket.o: socket.c
	$(CC) $(CFLAGS) socket.c

handle_udp.o: handle_udp.c
	$(CC) $(CFLAGS) handle_udp.c

udp_receiver.o: udp_receiver.c
	$(CC) $(CFLAGS) udp_receiver.c

udp_setup.o: udp_setup.c
	$(CC) $(CFLAGS) udp_setup.c

converter.o: converter.c
	$(CC) $(CFLAGS) converter.c

clean:
	rm -f $(OBJS) $(OUT)
CC = gcc
CFLAGS = -g -c -Wall -Wextra -pedantic
LIBS = -lpcap

SRCS = ipk.c udp.c tcp.c tcp_message.c tcp_command.c udp_command.c udp_message.c udp_receive.c
OBJS = ipk.o udp.o tcp.o tcp_message.o tcp_command.o udp_command.o udp_message.o udp_receive.o
OUT = ipk24chat-client

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)

ipk.o: ipk.c
	$(CC) $(CFLAGS) ipk.c

udp.o: udp.c
	$(CC) $(CFLAGS) udp.c

udp_command.o: udp_command.c
	$(CC) $(CFLAGS) udp_command.c

udp_message.o: udp_message.c
	$(CC) $(CFLAGS) udp_message.c

udp_receive.o: udp_receive.c
	$(CC) $(CFLAGS) udp_receive.c

tcp.o: tcp.c
	$(CC) $(CFLAGS) tcp.c

tcp_message.o: tcp_message.c
	$(CC) $(CFLAGS) tcp_message.c

tcp_command.o: tcp_command.c
	$(CC) $(CFLAGS) tcp_command.c

clean:
	rm -f $(OBJS) $(OUT)
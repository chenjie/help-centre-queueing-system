PORT = 64738
CFLAGS = -DPORT=$(PORT) -Wall -g -std=c99

all: hcq_server helpcentre

hcq_server: hcq_server.o hcq.o
	gcc $(CFLAGS) -o $@ $^

helpcentre: helpcentre.o hcq.o
	gcc $(CFLAGS) -o $@ $^

%.o: %.c hcq.h hcq_server.h
	gcc $(CFLAGS) -c $<

clean:
	-rm -f helpcentre *.o hcq_server

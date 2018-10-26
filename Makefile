CFLAGS = -Wall -g -std=gnu99

helpcentre: helpcentre.o hcq.o
	gcc ${CFLAGS} -o $@ $^

%.o : %.c hcq.h
	gcc ${CFLAGS} -c $<

clean:
	-rm helpcentre *.o

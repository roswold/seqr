CFLAGS=-Wfatal-errors
LDFLAGS=-lportaudio -lncurses -lpthread -lm -pthread
PROG=seqr

all:$(PROG)
clean:
	$(RM) -f $(PROG) *.o *~

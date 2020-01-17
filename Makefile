CFLAGS=-Wfatal-errors
LDFLAGS=-lm -lportaudio -lncurses -pthread -lpthread
PROG=seqr

all:$(PROG)
clean:
	$(RM) -f $(PROG) *.o *~

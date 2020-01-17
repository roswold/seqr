CFLAGS=-Wfatal-errors
LDFLAGS=-lm -lportaudio
PROG=seqr

all:$(PROG)
clean:
	$(RM) -f $(PROG) *.o *~

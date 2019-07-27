CC=cc
CFLAGS=-Wfatal-errors
LDFLAGS=-lm
TGT=seqr
RM=rm
all:
	$(CC) $(TGT).c -o $(TGT) $(CFLAGS) $(LDFLAGS)
clean:
	@$(RM) -f $(TGT) *.o *~

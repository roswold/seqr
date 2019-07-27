CC=cc
CFLAGS=-Wfatal-errors
LDFLAGS=-lm
TGT=seqr
all:
	$(CC) $(TGT).c -o $(TGT) $(CFLAGS) $(LDFLAGS)
clean:
	@rm -f $(TGT) *.o *~
 

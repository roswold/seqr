CC=cc
CFL=-Wfatal-errors
LFL=-lm
TGT=seqr
all:
 $(CC) $(TGT).c -o $(TGT) $(CFL) $(LFL)
clean:
 @rm -f $(TGT) *.o *~

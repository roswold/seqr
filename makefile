CC=cc
 2 CFL=-Wfatal-errors
 3 LFL=-lm
 4 TGT=seqr
 5 all:
 6     $(CC) $(TGT).c -o $(TGT) $(CFL) $(LFL)
 7 clean:
 8     @rm -f $(TGT) *.o *~

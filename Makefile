all: seqr
seqr: src/seqr
	cp src/seqr .
src/seqr:
	make -C src
clean:
	make clean -C src
	$(RM) seqr

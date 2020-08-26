all:
	make -C src
	cp src/seqr .
clean:
	make clean -C src
	$(RM) seqr
.PHONY: all

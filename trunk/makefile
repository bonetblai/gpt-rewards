all:
	@echo "*** Making all in `pwd`"
	@(cd src; make all)
	@(cd bin; ln -s ../src/rtdp-bel/gpt gpt)

clean:
	@echo "*** Cleaning `pwd`"
	@rm -f bin/gpt
	@(cd src; make clean)


all:
	@echo "*** Making all in `pwd`"
	@(cd src; make all)
	@(cd bin; rm gpt; ln -s ../src/rtdp-bel/gpt gpt)

clean:
	@echo "*** Cleaning `pwd`"
	@rm -f bin/gpt
	@(cd src; make clean)


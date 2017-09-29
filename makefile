all:
	@echo "*** Making all in `pwd`"
	@mkdir -p bin src/include/mdp src/lib
	@(cd src; make all)
	@(cd bin; rm -f gpt; ln -s ../src/rtdp-bel/gpt gpt)

clean:
	@echo "*** Cleaning `pwd`"
	@rm -f bin/gpt
	@(cd src; make clean)


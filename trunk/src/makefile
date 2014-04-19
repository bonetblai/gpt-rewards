all:
	@echo "*** Making all in `pwd`"
	@(cd lib-src/nmdp; make)
	@(cd rtdp-bel; make)

clean:
	@echo "*** Cleaning `pwd`"
	@rm -f lib/* include/mdp/*
	@(cd lib-src/nmdp; make clean)
	@(cd rtdp-bel; make clean)


# Faust-related make targets:

CLASS_NAME = MonoSource
ARCH = faustMinimal.h

FAUST = faust -a $(ARCH) -scn ::dsp -cn $(CLASS_NAME) --inline-architecture-files --in-place
#
# --inline-architecture-files (-i) means that all Faust headers
#           and include files get included in the output C++ file
#           so that it can be compiled without a Faust installation
#           on the compiling machine.
#
# --in-place (-inpl) means the input signal array can be used
#           as the output signal array in the compute() function.

all: $(CLASS_NAME).h

$(CLASS_NAME).h: $(CLASS_NAME).dsp Makefile
	$(FAUST) $< > $@

$(CLASS_NAME).app app: $(CLASS_NAME).dsp
	faust2caqt $<
	open $@

clean:
	-/bin/rm $(CLASS_NAME).h
	-/bin/rm -rf $(CLASS_NAME).app
	-/bin/rm -rf $(CLASS_NAME)-svg

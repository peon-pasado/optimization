#
# $Id: Makefile,v 1.26 2022/12/05 05:45:39 tanaka Rel $
# $Revision: 1.26 $
# $Date: 2022/12/05 05:45:39 $
# $Author: tanaka $
#
#
REVISION=r108p4

.PHONY: all strip clean depend sips sipsi sips-clean sipsi-clean

subdirs     = SiPS SiPSi
DEFS        = # -DUSE_CLOCK -DCOST_REAL -DCOST_LONGLONG -DREAL_LONG
              # -DLB_DEBUG -DAGGRESSIVE -DWITHOUT_MTABLE
              # -DNLOGN_EDGE_FINDING
CFLAGS      = -Wall -O3 -fomit-frame-pointer -ffast-math
CFLAGS     += -mcpu=apple-m1
CO          = co -r$(REVISION)

export DEFS CFLAGS CO

all strip clean depend:
	@for dir in $(subdirs); do \
	$(MAKE) -C $$dir $@ || exit; \
	done
sips:
	$(MAKE) -C SiPS
sipsi:
	$(MAKE) -C SiPSi
sips-clean:
	$(MAKE) -C SiPS clean
sipsi-clean:
	$(MAKE) -C SiPSi clean

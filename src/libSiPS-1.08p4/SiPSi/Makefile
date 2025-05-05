#
# $Id: Makefile,v 1.10 2011/12/23 01:32:52 tanaka Rel $
# $Revision: 1.10 $
# $Date: 2011/12/23 01:32:52 $
# $Author: tanaka $
#
#
.PHONY: all strip clean depend

topdir     = ..
common     = $(topdir)/common
VPATH      = $(common)

OBJS1      = bmemory.o constraint.o dominance.o fixed.o job_sort.o lag.o \
 	     lag2.o memory.o param.o print.o prob_common.o ptable.o sol.o \
             solve.o ssdp.o subgradient.o timer.o
OBJS2      = dynasearch.o func.o heuristics.o objective.o problem.o
OBJS       = $(OBJS1) $(OBJS2)
OBJM       = main.o
SRCS      := $(OBJS:.o=.c)
SRCM      := $(OBJM:.o=.c)

TARGET     = $(topdir)/sipsi
SIPSLIB    = libSiPSi.a
LIBS       = -L. -lSiPSi

CC         = gcc
MAKEDEP    = gcc -MM
AR         = ar cr
RANLIB     = ranlib

override DEFS += -I. -I$(common) -DSIPSI

all: $(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(DEFS) -c $<

$(TARGET): $(OBJM) $(SIPSLIB)
	$(CC) $(CFLAGS) $(DEFS) -o $@ $(OBJM) $(LIBS)

$(SIPSLIB): $(OBJS)
	$(AR) $(SIPSLIB) $(OBJS)
	$(RANLIB) $(SIPSLIB)

strip: $(TARGET)
	@strip $(TARGET)

clean:
	rm -f $(TARGET) $(SIPSLIB) $(OBJS) $(OBJM) *~ *.bak #*

depend:
	@sed -i -e "/^# START/,/# END/d" Makefile
	@echo "# START" >> Makefile
	@$(MAKEDEP) $(DEFS) $(OBJS1:%.o=$(common)/%.c) >> Makefile
	@$(MAKEDEP) $(DEFS) $(OBJS2:.o=.c) >> Makefile
	@$(MAKEDEP) $(DEFS) $(OBJM:%.o=$(common)/%.c) >> Makefile
	@echo "# END" >> Makefile

# START
bmemory.o: ../common/bmemory.c ../common/define.h ../common/sips_common.h \
 sipsi.h ../common/bmemory.h ../common/memory.h
constraint.o: ../common/constraint.c ../common/define.h \
 ../common/sips_common.h sipsi.h ../common/bmemory.h \
 ../common/constraint.h ../common/fixed.h ../common/lag2.h \
 ../common/lag_common.h ../common/memory.h ../common/ptable.h \
 ../common/print.h
dominance.o: ../common/dominance.c ../common/define.h \
 ../common/sips_common.h sipsi.h ../common/dominance.h
fixed.o: ../common/fixed.c ../common/define.h ../common/sips_common.h \
 sipsi.h ../common/fixed.h ../common/memory.h
job_sort.o: ../common/job_sort.c ../common/define.h \
 ../common/sips_common.h sipsi.h ../common/job_sort.h
lag.o: ../common/lag.c ../common/define.h ../common/sips_common.h sipsi.h \
 ../common/ssdp.h ../common/bmemory.h ../common/constraint.h \
 ../common/lag.h ../common/lag_common.h ../common/memory.h \
 ../common/objective.h ../common/ptable.h ../common/sol.h
lag2.o: ../common/lag2.c ../common/define.h ../common/sips_common.h \
 sipsi.h ../common/bmemory.h ../common/constraint.h ../common/dominance.h \
 ../common/fixed.h ../common/lag.h ../common/lag2.h \
 ../common/lag_common.h ../common/memory.h ../common/objective.h \
 ../common/print.h ../common/ptable.h ../common/sol.h
memory.o: ../common/memory.c ../common/define.h ../common/sips_common.h \
 sipsi.h ../common/memory.h
param.o: ../common/param.c ../common/define.h ../common/sips_common.h \
 sipsi.h default.h ../common/sips_common.h ../common/param.h
print.o: ../common/print.c ../common/define.h ../common/sips_common.h \
 sipsi.h ../common/timer.h ../common/print.h
prob_common.o: ../common/prob_common.c ../common/define.h \
 ../common/sips_common.h sipsi.h ../common/bmemory.h ../common/fixed.h \
 ../common/memory.h ../common/param.h ../common/print.h \
 ../common/prob_common.h ../common/problem.h ../common/ptable.h \
 ../common/sol.h
ptable.o: ../common/ptable.c ../common/define.h ../common/sips_common.h \
 sipsi.h ../common/ssdp.h ../common/constraint.h ../common/job_sort.h \
 ../common/lag.h ../common/lag_common.h ../common/lag2.h \
 ../common/memory.h ../common/ptable.h ../common/sol.h
sol.o: ../common/sol.c ../common/define.h ../common/sips_common.h sipsi.h \
 ../common/bmemory.h func.h ../common/define.h ../common/sips_common.h \
 ../common/memory.h ../common/objective.h ../common/sol.h
solve.o: ../common/solve.c ../common/define.h ../common/sips_common.h \
 sipsi.h ../common/memory.h ../common/prob_common.h ../common/problem.h \
 ../common/sol.h ../common/ssdp.h ../common/timer.h
ssdp.o: ../common/ssdp.c ../common/define.h ../common/sips_common.h \
 sipsi.h ../common/dynasearch.h ../common/heuristics.h \
 ../common/job_sort.h ../common/lag_common.h ../common/lag.h \
 ../common/lag2.h ../common/memory.h ../common/sol.h ../common/ssdp.h \
 ../common/subgradient.h ../common/timer.h
subgradient.o: ../common/subgradient.c ../common/define.h \
 ../common/sips_common.h sipsi.h ../common/dynasearch.h \
 ../common/heuristics.h ../common/lag.h ../common/lag2.h \
 ../common/memory.h ../common/print.h ../common/ptable.h ../common/sol.h \
 ../common/subgradient.h ../common/timer.h
timer.o: ../common/timer.c ../common/define.h ../common/sips_common.h \
 sipsi.h ../common/timer.h
dynasearch.o: dynasearch.c ../common/define.h ../common/sips_common.h \
 sipsi.h ../common/sips_common.h ../common/bmemory.h ../common/define.h \
 ../common/dynasearch.h func.h ../common/memory.h ../common/objective.h \
 ../common/print.h ../common/sol.h
func.o: func.c ../common/define.h ../common/sips_common.h sipsi.h \
 ../common/sips_common.h ../common/bmemory.h ../common/define.h \
 ../common/memory.h func.h
heuristics.o: heuristics.c ../common/define.h ../common/sips_common.h \
 sipsi.h ../common/sips_common.h ../common/bmemory.h ../common/define.h \
 func.h ../common/heuristics.h ../common/job_sort.h ../common/memory.h \
 ../common/print.h ../common/objective.h ../common/sol.h
objective.o: objective.c ../common/define.h ../common/sips_common.h \
 sipsi.h ../common/sips_common.h ../common/memory.h ../common/objective.h
problem.o: problem.c ../common/define.h ../common/sips_common.h sipsi.h \
 ../common/sips_common.h ../common/job_sort.h ../common/memory.h \
 ../common/objective.h ../common/prob_common.h ../common/problem.h
main.o: ../common/main.c ../common/sips_common.h sipsi.h
# END

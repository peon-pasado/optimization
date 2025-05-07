cmake_minimum_required(VERSION 3.12)
project(SiPS C)

set(CMAKE_BUILD_TYPE "Release")
if(WIN32)
  set(CMAKE_C_FLAGS "-Wall")
  add_definitions(-DUSE_CLOCK)
else()
  set(CMAKE_C_FLAGS "-Wall -Wextra -pedantic -ffast-math")
endif()

# add_definitions(-DUSE_CLOCK) # use clock() for measuring time
# add_definitions(-DCOST_LONGLONG) # objvective value stored in long long int
# add_definitions(-DCOST_REAL) # double
# add_definitions(-DCOST_REAL -DREAL_LONG) # long double
# add_definitions(-DAGGRESSIVE) # aggressive constraint propagation
# add_definitions(-DNLOGN_EDGE_FINDING) # O(n log n) algorithm for edge-finding
# add_definitions(-DWITHOUT_MTABLE) # for debugging
# add_definitions(-DLB_DEBUG) # for debugging

include_directories(common)

set(common_srcs
  ../common/bmemory.c
  ../common/constraint.c
  ../common/dominance.c
  ../common/fixed.c
  ../common/job_sort.c
  ../common/lag.c
  ../common/lag2.c
  ../common/memory.c
  ../common/param.c
  ../common/print.c
  ../common/prob_common.c
  ../common/ptable.c
  ../common/sol.c
  ../common/solve.c
  ../common/ssdp.c
  ../common/subgradient.c
  ../common/timer.c
)

set(common_headers
  ../common/bmemory.h
  ../common/constraint.h
  ../common/define.h
  ../common/dominance.h
  ../common/dynasearch.h
  ../common/fixed.h
  ../common/heuristics.h
  ../common/job_sort.h
  ../common/lag.h
  ../common/lag2.h
  ../common/lag_common.h
  ../common/memory.h
  ../common/objective.h
  ../common/param.h
  ../common/print.h
  ../common/prob_common.h
  ../common/problem.h
  ../common/ptable.h
  ../common/sips_common.h
  ../common/sol.h
  ../common/ssdp.h
  ../common/subgradient.h
  ../common/timer.h
)

add_subdirectory(SiPS)
add_subdirectory(SiPSi)

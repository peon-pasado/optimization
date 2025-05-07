add_compile_definitions(SIPSI)
include_directories(.)

set(sipsi_srcs dynasearch.c func.c heuristics.c objective.c problem.c)
set(sipsi_headers default.h func.h sipsi.h)
add_library(SiPSi STATIC ${common_srcs} ${sipsi_srcs} ${common_headers} ${sipsi_headers})

add_executable(sipsi ../common/main.c sipsi.h)
target_link_libraries(sipsi SiPSi)

# add_executable(sample-sipsi ../docs/sample.c sipsi.h)
# target_link_libraries(sample-sipsi SiPSi)

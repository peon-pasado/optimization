include_directories(.)

set(sips_srcs dynasearch.c heuristics.c objective.c problem.c)
set(sips_headers default.h sips.h)
add_library(SiPS STATIC ${common_srcs} ${sips_srcs} ${common_headers} ${sips_headers})

add_executable(sips ../common/main.c sips.h)
target_link_libraries(sips SiPS)

# add_executable(sample-sips ../docs/sample.c sips.h)
# target_link_libraries(sample-sips SiPS)

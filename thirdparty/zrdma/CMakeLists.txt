
set(ZRDMA_SOURCE zQP.cc QP.hpp zQP.h)
add_library(zrdma STATIC ${ZRDMA_SOURCE})
target_link_libraries(zrdma ibverbs rdmacm pthread rt mlx5 tbb profiler)
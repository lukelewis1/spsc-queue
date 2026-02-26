add_library(spsc_queue INTERFACE)
target_include_directories(spsc_queue INTERFACE include)

add_subdirectory(bench)
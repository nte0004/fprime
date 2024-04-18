####
# platform.cmake.template:
#
#   Platform file for FreeRTOS - may need to change hard coded values in the code if using something other than the Cortex A53
#   likely, though, only need to change assumptions related to only using the Cortex A53
####


## STEP 2: Specify the OS type include directive i.e. LINUX or DARWIN
add_definitions(-DTGT_OS_TYPE_FREERTOS)

# STEP 3: Specify CMAKE C and CXX compile flags. DO NOT clear existing flags
set(CMAKE_C_FLAGS
  "${CMAKE_C_FLAGS} -static -march=armv8-a -mtune=cortex-a53 -mcpu=cortex-a53 -mfloat-abi=hard"
)
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -static -march=armv8-a -mtune=cortex-a53 -mcpu=cortex-a53 -mfloat-abi=hard"
)


# STEP 4: Specify that a thread package should be searched in the toolchain
#         directory. NOTE: when running without threads, remove this line.
#         Here there is a check for the using baremetal scheduler
#if (NOT DEFINED FPRIME_USE_BAREMETAL_SCHEDULER)
#   set(FPRIME_USE_BAREMETAL_SCHEDULER OFF)
#   message(STATUS "Requiring thread library")
#   FIND_PACKAGE ( Threads REQUIRED )
#endif()

choose_fprime_implementation(Os/File Os/File/FreeRTOS)

# STEP 5: Specify a directory containing the "PlatformTypes.hpp" headers, as well
#         as other system headers. Other global headers can be placed here.
#         Note: Typically, the Linux directory is a good default, as it grabs
#         standard types from <cstdint>.
include_directories(SYSTEM "${CMAKE_CURRENT_LIST_DIR}/types")
include_directories(SYSTEM "$ENV{FREERTOS_INCLUDE}")
include_directories(SYSTEM "ENV${BSP_INCLUDE_DIRECTORY}")


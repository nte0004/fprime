####
# zcu102 - freeRTOS toolchain:
#
#  This is the toolchain for the zcu102. It should work (in conjunction with the FreeRTOS) for any Cortex A53 processor, though.
#  This, of course, is assuming you are running this as a baremetal application - not under petaLinux or similar.
#  Similar instructions to the arm-linux toolchains. need to specify ARM_TOOLS_PATH and (for the platform file) FREERTOS_INCLUDE as environment variables.
####

set(CMAKE_SYSTEM_PROCESSOR "aarch64")
include("${CMAKE_CURRENT_LIST_DIR}/helpers/arm-FreeRTOS-base.cmake")

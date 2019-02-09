# Find Vulkan
#
# VULKAN_FOUND - True if Vulkan is found
# VULKAN_INCLUDE_DIR - The include directory of Vulkan
# VULKAN_LIBRARY - The Library for Vulkan


IF (WIN32)
    FIND_PATH(VULKAN_INCLUDE_DIRS NAMES vulkan/vulkan.h HINTS
        "$ENV{VULKAN_SDK}/Include"
        "$ENV{VK_SDK_PATH}/Include")
    IF (CMAKE_CL_64)
        FIND_LIBRARY(VULKAN_LIBRARY NAMES vulkan-1 HINTS
            "$ENV{VULKAN_SDK}/Bin"
            "$ENV{VK_SDK_PATH}/Bin")
    ELSE()
        FIND_LIBRARY(VULKAN_LIBRARY NAMES vulkan-1 HINTS
            "$ENV{VULKAN_SDK}/Bin32"
            "$ENV{VK_SDK_PATH}/Bin32")
    ENDIF()
ELSE()
    FIND_PATH(VULKAN_INCLUDE_DIR NAMES vulkan/vulkan.h HINTS
    "/usr/include")
    FIND_LIBRARY(VULKAN_LIBRARY NAMES vulkan HINTS
    "/usr/lib/x86_64-linux-gnu")
ENDIF(WIN32)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Vulkan DEFAULT_MSG VULKAN_LIBRARY VULKAN_INCLUDE_DIRS)

MARK_AS_ADVANCED(VULKAN_INCLUDE_DIR VULKAN_LIBRARY)

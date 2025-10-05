#VULKAN
if (DEFINED VULKAN_SDK_PATH)
    set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include") # 1.1 Make sure this include path is correct
    set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib") # 1.2 Make sure lib path is correct
    set(Vulkan_FOUND "True")
else()
    find_package(Vulkan REQUIRED) # throws error if could not find Vulkan
    message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
endif()
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
else()
    message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
endif()

#GLFW
if (DEFINED GLFW_PATH)
    message(STATUS "Using GLFW path specified in .env")
    set(GLFW_INCLUDE_DIRS "${GLFW_PATH}/include")
    if (MSVC)
        set(GLFW_LIB "${GLFW_PATH}/lib-vc2019") # 2.1 Update lib-vc2019 to use same version as your visual studio
    elseif (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
        message(STATUS "USING MINGW")
        set(GLFW_LIB "${GLFW_PATH}/lib-mingw-w64") # 2.1 make sure matches glfw mingw subdirectory
    endif()
else()
    find_package(glfw3 3.3 REQUIRED)
    set(GLFW_LIB glfw)
    message(STATUS "Found GLFW")
endif()
if (NOT GLFW_LIB)
    message(FATAL_ERROR "Could not find glfw library!")
else()
    message(STATUS "Using glfw lib at: ${GLFW_LIB}")
endif()

#OPEN_AL
if (DEFINED OpenAL_PATH)
    set(OpenAL_INCLUDE_DIRS "${OpenAL_PATH}/include") # 1.1 Make sure this include path is correct
    set(OpenAL_LIB "${OpenAL_PATH}/lib") # 1.2 Make sure lib path is correct
    set(OpenAL_FOUND "True")
else()
    find_package(OpenAL REQUIRED)
    message(STATUS "Found OpenAL: $ENV{OpenAL_PATH}")
endif()
if (NOT OpenAL_FOUND)
    message(FATAL_ERROR "Could not find OpenAL library!")
else()
    message(STATUS "Using OpenAL lib at: ${OPENAL_LIB}")
endif()

# IMGUI
# FetchContent to download and include ImGui
include(FetchContent)
# Add ImGui via FetchContent
FetchContent_Declare(
  ImGui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.5  # You can specify the version you want to use here.
)
# Make sure the content is downloaded and available
FetchContent_MakeAvailable(ImGui)

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp

  # Add Vulkan backend
  ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.h
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp  # Assuming GLFW is used
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h
)

# Include the backend headers
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)

# Link libraries (add glfw, and vulkan dependencies)
target_link_libraries(imgui Vulkan::Vulkan glfw)

# Fetch ImPlot
FetchContent_Declare(
    ImPlot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG        v0.16
)
FetchContent_MakeAvailable(ImPlot)
add_library(implot STATIC
  ${implot_SOURCE_DIR}/implot.h
  ${implot_SOURCE_DIR}/implot_internal.h
  ${implot_SOURCE_DIR}/implot.cpp
  ${implot_SOURCE_DIR}/implot_items.cpp
  ${implot_SOURCE_DIR}/implot_demo.cpp
)
target_link_libraries(implot imgui)


#linking
if (WIN32)
  message(STATUS "CREATING BUILD FOR WINDOWS")

  if (USE_MINGW)
    target_include_directories(${PROJECT_NAME} PUBLIC
      ${MINGW_PATH}/include
    )
    target_link_directories(${PROJECT_NAME} PUBLIC
      ${MINGW_PATH}/lib
    )
  endif()

  target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${Vulkan_INCLUDE_DIRS}
    ${TINYOBJ_PATH}
    ${STB_PATH}
    ${GLFW_INCLUDE_DIRS}
    ${GLM_PATH}
    )

  target_link_directories(${PROJECT_NAME} PUBLIC
    ${Vulkan_LIBRARIES}
    ${GLFW_LIB}
  )

  target_link_libraries(
      ${PROJECT_NAME}
      glfw3 vulkan-1 imgui implot
  )
elseif (UNIX)
    message(STATUS "CREATING BUILD FOR UNIX")
    target_include_directories(${PROJECT_NAME} PUBLIC
      ${PROJECT_SOURCE_DIR}/src
      ${TINYOBJ_PATH}
      ${STB_PATH} ${Vulkan_INCLUDE_DIRS}
    )
    target_link_libraries(
        ${PROJECT_NAME} glfw ${Vulkan_LIBRARIES} imgui implot OpenAL::OpenAL
    )
endif()

############## Build SHADERS #######################

# Find all vertex and fragment sources within shaders directory
# taken from VBlancos vulkan tutorial
# https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters/CMakeLists.txt
find_program(GLSL_VALIDATOR glslangValidator REQUIRED HINTS
  ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
)
message(STATUS "GLSL validator found: ${GLSL_VALIDATOR}")

# get all .vert and .frag files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES
  "${PROJECT_SOURCE_DIR}/assets/shaders/*.comp"
  "${PROJECT_SOURCE_DIR}/assets/shaders/*.frag"
  "${PROJECT_SOURCE_DIR}/assets/shaders/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/assets/shaders/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    # message(STATUS "compiled: ${SPIRV}")
    # message("${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}")
endforeach(GLSL)

add_custom_target(
    Shaders
    ALL DEPENDS ${SPIRV_BINARY_FILES}
)

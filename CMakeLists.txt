# CMakeList.txt: IsaacSaveReverter 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("IsaacSaveReverter")

################## imgui #########
add_executable (ImguiApp "imgui/gui.cpp"  "IsaacSave.h" "imgui/main.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ImguiApp PROPERTY CXX_STANDARD 17)
endif()

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(ImguiApp PRIVATE glfw)

find_package(glad CONFIG REQUIRED)
target_link_libraries(ImguiApp PRIVATE glad::glad)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(ImguiApp PRIVATE imgui::imgui)


################# cli ############

add_executable (Cli "IsaacSave.h" "cli/cli.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Cli PROPERTY CXX_STANDARD 17)
endif()



################# test ############

add_executable (Test "IsaacSave.h" "test/test.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Test PROPERTY CXX_STANDARD 17)
endif()




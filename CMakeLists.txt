cmake_minimum_required (VERSION 3.28.0)

project ("JSONParser")

# TODO: REMOVE
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

find_package(nlohmann_json CONFIG REQUIRED)

add_executable(x "main.cpp" "src/JSONParser/JSONTokenizer.cpp" "src/JSONParser/JSONParser.cpp" "src/JSONParser/Concepts.h")
target_link_libraries(x PRIVATE nlohmann_json::nlohmann_json)

set_target_properties(x PROPERTIES CXX_STANDARD 23)
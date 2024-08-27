cmake_minimum_required (VERSION 3.28.0)

project ("JSONParser")

add_executable(x "main.cpp" "src/JSONParser/JSONTokenizer.cpp")

set_target_properties(x PROPERTIES CXX_STANDARD 23)
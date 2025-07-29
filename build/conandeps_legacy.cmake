message(STATUS "Conan: Using CMakeDeps conandeps_legacy.cmake aggregator via include()")
message(STATUS "Conan: It is recommended to use explicit find_package() per dependency instead")

find_package(glfw3)
find_package(glad)
find_package(implot)
find_package(imgui)
find_package(nlohmann_json)

set(CONANDEPS_LEGACY  glfw  glad::glad  implot::implot  imgui::imgui  nlohmann_json::nlohmann_json )
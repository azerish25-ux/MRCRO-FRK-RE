cmake_minimum_required(VERSION 3.22)
project(mfrk_knob_geometry LANGUAGES CXX)
set(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../../..")
add_library(mfrk_knob_geometry SHARED knob_geometry_jni.cpp slider_controller_jni.cpp
    "${REPO_ROOT}/core/src/ui_primitives.cpp"
    "${REPO_ROOT}/core/src/ui_slider_controller.cpp"
    "${REPO_ROOT}/core/src/ui_slider_drag.cpp"
    "${REPO_ROOT}/core/src/ui_slider_value.cpp")
target_include_directories(mfrk_knob_geometry PRIVATE "${REPO_ROOT}/core/include")
target_compile_features(mfrk_knob_geometry PRIVATE cxx_std_20)
target_compile_options(mfrk_knob_geometry PRIVATE -fno-fast-math -ffp-contract=off)
target_link_libraries(mfrk_knob_geometry PRIVATE m)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_sources(mfrk_knob_geometry PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../../debug/cpp/gesture_probe_jni.cpp")
endif()

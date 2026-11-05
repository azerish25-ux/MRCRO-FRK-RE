cmake_minimum_required(VERSION 3.22)
project(mfrk_midi_jni LANGUAGES CXX)
set(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../../..")
add_library(mfrk_midi_jni SHARED midi_jni.cpp
  "${REPO_ROOT}/core/src/midi_stream.cpp" "${REPO_ROOT}/core/src/midi_ingress.cpp")
target_include_directories(mfrk_midi_jni PRIVATE "${REPO_ROOT}/core/include")
target_compile_features(mfrk_midi_jni PRIVATE cxx_std_20)

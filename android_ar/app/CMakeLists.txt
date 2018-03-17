# Copyright (C) 2017 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##

# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Import the ARCore library.
add_library(arcore SHARED IMPORTED)
set_target_properties(arcore PROPERTIES IMPORTED_LOCATION
                      ${ARCORE_LIBPATH}/${ANDROID_ABI}/libarcore_sdk.so)

# This is the main app library.
add_library(hello_ar_native SHARED
           src/main/cpp/background_renderer.cc
           src/main/cpp/hello_ar_application.cc
           src/main/cpp/jni_interface.cc
           src/main/cpp/game_renderer.cc
           src/main/cpp/game_box_renderer.cc
           src/main/cpp/debug_renderer.cc
           src/main/cpp/util.cc
           src/main/cpp/game_controller.cc
           src/main/cpp/game/src/game.cpp
           src/main/cpp/game/src/game-box.cpp
           src/main/cpp/game/src/cemented-block-array.cpp
           src/main/cpp/game/src/piece.cpp
           src/main/cpp/game/src/piece-generator.cpp)

target_include_directories(hello_ar_native PRIVATE
           src/main/cpp
            ${ARCORE_INCLUDE}
            ${ANDROID_NDK}/sources/third_party/vulkan/src/libs/glm
            src/main/cpp/game/include)

target_link_libraries(hello_ar_native
                      android
                      log
                      GLESv2
                      arcore)

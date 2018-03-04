/*
 * Copyright 2017 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Modifications copyright (C) Otto Seiskari 2018
 */

#ifndef C_GAME_RENDERER_
#define C_GAME_RENDERER_
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "arcore_c_api.h"
#include "glm.h"
#include "api.hpp"

namespace hello_ar {

class GameRenderer {
public:
  GameRenderer() = default;
  ~GameRenderer() = default;

  // Loads the OBJ file and texture and sets up OpenGL resources used to draw
  // the model.  Must be called on the OpenGL thread prior to any other calls.
  void InitializeGlContent(AAssetManager* asset_manager);

  // Draws the model.
  void Draw(const glm::mat4& projection_mat, const glm::mat4& view_mat,
            const glm::mat4& model_mat, float light_intensity) const;

  void update(const Game& game, float game_scale);

private:
  // Shader material lighting pateremrs
  float ambient_ = 0.0f;
  float diffuse_ = 3.5f;
  float specular_ = 1.0f;
  float specular_power_ = 6.0f;

  // Model attribute arrays
  std::vector<GLfloat> cube_vertices_, vertices_;
  std::vector<GLfloat> cube_uvs_, uvs_;
  std::vector<GLfloat> cube_normals_, normals_;

  // Model triangle indices
  std::vector<GLushort> cube_indices_, indices_;

  // Shader program details
  GLuint shader_program_;
  GLuint attri_vertices_;
  GLuint attri_uvs_;
  GLuint attri_normals_;
  GLuint uniform_mvp_mat_;
  GLuint uniform_mv_mat_;
  GLuint uniform_lighting_param_;
  GLuint uniform_material_param_;
};

extern const glm::mat4 GAME_MODEL_TRANSFORM;

}  // namespace hello_ar

#endif

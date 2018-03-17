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

  struct Model {
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> uvs;
    std::vector<GLfloat> normals;
    std::vector<GLushort> indices;
  };

  // Model attribute arrays
  Model cube_;

  std::vector<Model> scene_by_material_;
  std::vector<glm::vec3> material_colors_;

  // Shader program details
  GLuint shader_program_;
  GLuint attri_vertices_;
  GLuint attri_uvs_;
  GLuint attri_normals_;
  GLuint uniform_mvp_mat_;
  GLuint uniform_mv_mat_;
  GLuint uniform_lighting_param_;
  GLuint uniform_material_param_;
  GLuint uniform_diffuse_color_;

  Model blocksToModel(const std::vector<Block> &blocks, Pos3d dims, float game_scale) const;
};

extern const glm::mat4 GAME_MODEL_TRANSFORM;

#endif

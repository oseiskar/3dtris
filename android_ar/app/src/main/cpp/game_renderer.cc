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

#include "game_renderer.h"
#include "util.h"

namespace hello_ar {
  namespace {
    const glm::vec4 kLightDirection(0.0f, 1.0f, 0.0f, 0.0f);

    constexpr char kVertexShader[] = R"(
uniform mat4 u_ModelView;
uniform mat4 u_ModelViewProjection;

attribute vec4 a_Position;
attribute vec3 a_Normal;
//attribute vec2 a_TexCoord;

varying vec3 v_ViewPosition;
varying vec3 v_ViewNormal;
//varying vec2 v_TexCoord;

void main() {
    v_ViewPosition = (u_ModelView * a_Position).xyz;
    v_ViewNormal = normalize((u_ModelView * vec4(a_Normal, 0.0)).xyz);
    //v_TexCoord = a_TexCoord;
    gl_Position = u_ModelViewProjection * a_Position;
})";

    constexpr char kFragmentShader[] = R"(
precision mediump float;

uniform vec4 u_LightingParameters;
uniform vec4 u_MaterialParameters;

varying vec3 v_ViewPosition;
varying vec3 v_ViewNormal;
//varying vec2 v_TexCoord;

void main() {
    // We support approximate sRGB gamma.
    const float kGamma = 0.4545454;
    const float kInverseGamma = 2.2;

    // Unpack lighting and material parameters for better naming.
    vec3 viewLightDirection = u_LightingParameters.xyz;
    float lightIntensity = u_LightingParameters.w;

    float materialAmbient = u_MaterialParameters.x;
    float materialDiffuse = u_MaterialParameters.y;
    float materialSpecular = u_MaterialParameters.z;
    float materialSpecularPower = u_MaterialParameters.w;

    // Normalize varying parameters, because they are linearly interpolated in
    // the vertex shader.
    vec3 viewFragmentDirection = normalize(v_ViewPosition);
    vec3 viewNormal = normalize(v_ViewNormal);

    vec4 objectColor = vec4(0.5, 0.5, 0.5, 0.5);
    objectColor.rgb = pow(objectColor.rgb, vec3(kInverseGamma));

    // Ambient light is unaffected by the light intensity.
    float ambient = materialAmbient;

    // Approximate a hemisphere light (not a harsh directional light).
    float diffuse = lightIntensity * materialDiffuse *
            0.5 * (dot(viewNormal, viewLightDirection) + 1.0);

    // Compute specular light.
    vec3 reflectedLightDirection = reflect(viewLightDirection, viewNormal);
    float specularStrength = max(0.0, dot(viewFragmentDirection,
            reflectedLightDirection));
    float specular = lightIntensity * materialSpecular *
            pow(specularStrength, materialSpecularPower);

    // Apply SRGB gamma before writing the fragment color.
    gl_FragColor.a = objectColor.a;
    gl_FragColor.rgb = pow(objectColor.rgb * (ambient + diffuse) + specular,
        vec3(kGamma));
}
)";

constexpr float yIsUpInsteadOfZEl[16] = {
    1, 0, 0, 0,
    0, 0,-1, 0,
    0, 1, 0, 0,
    0, 0, 0, 1
};
}  // namespace

const glm::mat4 GAME_MODEL_TRANSFORM = glm::make_mat4(yIsUpInsteadOfZEl);

void GameRenderer::InitializeGlContent(AAssetManager* asset_manager) {
  shader_program_ = util::CreateProgram(kVertexShader, kFragmentShader);

  if (!shader_program_) {
    LOGE("Could not create program.");
  }

  uniform_mvp_mat_ =
      glGetUniformLocation(shader_program_, "u_ModelViewProjection");
  uniform_mv_mat_ = glGetUniformLocation(shader_program_, "u_ModelView");

  uniform_lighting_param_ =
      glGetUniformLocation(shader_program_, "u_LightingParameters");
  uniform_material_param_ =
      glGetUniformLocation(shader_program_, "u_MaterialParameters");

  attri_vertices_ = glGetAttribLocation(shader_program_, "a_Position");
  //attri_uvs_ = glGetAttribLocation(shader_program_, "a_TexCoord");
  attri_normals_ = glGetAttribLocation(shader_program_, "a_Normal");

  util::LoadObjFile(asset_manager, "cube.obj", &cube_vertices_, &cube_normals_, &cube_uvs_, &cube_indices_);

  util::CheckGlError("obj_renderer::InitializeGlContent()");
}

void GameRenderer::update(const Game& game, float game_scale) {
  // TODO: vertex buffer

  vertices_.clear();
  //uvs_.clear();
  normals_.clear();
  indices_.clear();

  int index_offset = 0;
  for (Block block : game.getAllBlocks()) {
    float pos[3] = {
        block.pos.x + 0.5f - game.getDimensions().x*0.5f,
        block.pos.y + 0.5f - game.getDimensions().y*0.5f,
        block.pos.z + 0.5f
    };
    for (int vertex_index=0; vertex_index < cube_vertices_.size() / 3; ++vertex_index) {
      for (int coord = 0; coord < 3; ++coord) {
        vertices_.push_back((cube_vertices_[vertex_index*3+coord]+pos[coord])*game_scale);
      }
    }

    for (int index : cube_indices_) {
      indices_.push_back(index + index_offset);
    }
    index_offset += cube_indices_.size();

    // copy others unmodified
    //uvs_.insert(uvs_.end(), cube_uvs_.begin(), cube_uvs_.end());
    normals_.insert(normals_.end(), cube_normals_.begin(), cube_normals_.end());

  }

}

void GameRenderer::Draw(const glm::mat4& projection_mat,
                       const glm::mat4& view_mat, const glm::mat4& model_mat,
                       float light_intensity) const {
  if (!shader_program_) {
    LOGE("shader_program is null.");
    return;
  }

  if (vertices_.size() == 0) {
    return;
  }

  glUseProgram(shader_program_);

  glm::mat4 mvp_mat = projection_mat * view_mat * model_mat * GAME_MODEL_TRANSFORM;
  glm::mat4 mv_mat = view_mat * model_mat;
  glm::vec4 view_light_direction = glm::normalize(mv_mat * kLightDirection);

  glUniform4f(uniform_lighting_param_, view_light_direction[0],
      view_light_direction[1], view_light_direction[2],
      light_intensity);
  glUniform4f(uniform_material_param_, ambient_, diffuse_, specular_,
      specular_power_);

  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));
  glUniformMatrix4fv(uniform_mv_mat_, 1, GL_FALSE, glm::value_ptr(mv_mat));

  // Note: for simplicity, we are uploading the model each time we draw it.  A
  // real application should use vertex buffers to upload the geometry once.

  glEnableVertexAttribArray(attri_vertices_);
  glVertexAttribPointer(attri_vertices_, 3, GL_FLOAT, GL_FALSE, 0,
      vertices_.data());

  glEnableVertexAttribArray(attri_normals_);
  glVertexAttribPointer(attri_normals_, 3, GL_FLOAT, GL_FALSE, 0,
      normals_.data());

  //glEnableVertexAttribArray(attri_uvs_);
  //glVertexAttribPointer(attri_uvs_, 2, GL_FLOAT, GL_FALSE, 0, uvs_.data());

  glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_SHORT,
      indices_.data());

  glDisableVertexAttribArray(attri_vertices_);
  //glDisableVertexAttribArray(attri_uvs_);
  glDisableVertexAttribArray(attri_normals_);

  glUseProgram(0);
  util::CheckGlError("obj_renderer::Draw()");
}

}  // namespace hello_ar

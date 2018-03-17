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
#include <array>

namespace {
const glm::vec4 kLightDirection(0.5f, 1.0f, 2.0f, 0.0f);

// Shader material lighting "pateremrs" [sic]
// ... just wondering how sleep-deprived the Google interns who wrote the original
// shader were after the ARKit release :P

constexpr float AMBIENT = 0.0f;
constexpr float DIFFUSE = 3.5f;
constexpr float SPECULAR = 1.0f;
constexpr float SPECULAR_POWER = 6.0f;

constexpr char kVertexShader[] = R"(
uniform mat4 u_ModelView;
uniform mat4 u_ModelViewProjection;

attribute vec4 a_Position;
attribute vec3 a_Normal;
attribute vec2 a_TexCoord;

varying vec3 v_ViewPosition;
varying vec3 v_ViewNormal;
varying vec2 v_TexCoord;

void main() {
    v_ViewPosition = (u_ModelView * a_Position).xyz;
    v_ViewNormal = normalize((u_ModelView * vec4(a_Normal, 0.0)).xyz);
    v_TexCoord = a_TexCoord;
    gl_Position = u_ModelViewProjection * a_Position;
})";

constexpr char kFragmentShader[] = R"(
precision mediump float;

uniform vec4 u_LightingParameters;
uniform vec4 u_MaterialParameters;
uniform vec3 u_DiffuseColor;

varying vec3 v_ViewPosition;
varying vec3 v_ViewNormal;
varying vec2 v_TexCoord;

void main() {
    // We support approximate sRGB gamma.
    const float kGamma = 0.4545454;
    const float kInverseGamma = 2.2;

    const float EDGE_WIDTH = 0.02;
    const float EDGE_BRIGHTNESS = 0.3;

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

    vec4 objectColor = vec4(u_DiffuseColor, 1.0);
    objectColor.rgb = pow(objectColor.rgb, vec3(kInverseGamma));

    // Ambient light is unaffected by the light intensity.
    float ambient = materialAmbient;

    // Approximate a hemisphere light (not a harsh directional light).
    float diffuse = lightIntensity * materialDiffuse *
            0.5 * (dot(viewNormal, viewLightDirection) + 1.0);

    vec2 edgeVec = abs((v_TexCoord - 0.5)*2.0);
    float edgeness = max(edgeVec.x, edgeVec.y);

    if (edgeness > 1.0 - EDGE_WIDTH) objectColor.rgb = objectColor.rgb * EDGE_BRIGHTNESS;

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

constexpr int32_t kBlockColorRgbaSize = 16;

constexpr std::array<uint32_t, kBlockColorRgbaSize> kBlockColorRgba = {{
   0xFFFFFFFF, 0xF44336FF, 0xE91E63FF, 0x9C27B0FF, 0x673AB7FF, 0x3F51B5FF,
   0x2196F3FF, 0x03A9F4FF, 0x00BCD4FF, 0x009688FF, 0x4CAF50FF, 0x8BC34AFF,
   0xCDDC39FF, 0xFFEB3BFF, 0xFFC107FF, 0xFF9800FF }};

constexpr float COLOR_LIGHTNESS = 0.7;

inline glm::vec3 GetBlockColor(int i) {
  const int32_t colorRgba = kBlockColorRgba[i % kBlockColorRgbaSize];
  return glm::vec3(((colorRgba >> 24) & 0xff) / 255.0f,
      ((colorRgba >> 16) & 0xff) / 255.0f,
      ((colorRgba >> 8) & 0xff) / 255.0f) * COLOR_LIGHTNESS;
}
}  // namespace


GameRenderer::Model GameRenderer::blocksToModel(const std::vector<Block>& blocks,
                                                Pos3d dims,
                                                float game_scale) const {
  Model m = Model();

  int index_offset = 0;
  for (Block block : blocks) {
    float pos[3] = {
        block.pos.x + 0.5f - dims.x*0.5f,
        block.pos.y + 0.5f - dims.y*0.5f,
        block.pos.z + 0.5f
    };
    for (int vertex_index=0; vertex_index < cube_.vertices.size() / 3; ++vertex_index) {
      for (int coord = 0; coord < 3; ++coord) {
        m.vertices.push_back((cube_.vertices[vertex_index*3+coord]+pos[coord])*game_scale);
      }
    }

    for (int index : cube_.indices) {
      m.indices.push_back(index + index_offset);
    }
    index_offset += cube_.indices.size();

    // copy others unmodified
    m.uvs.insert(m.uvs.end(), cube_.uvs.begin(), cube_.uvs.end());
    m.normals.insert(m.normals.end(), cube_.normals.begin(), cube_.normals.end());

  }

  return m;
}

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

  uniform_diffuse_color_ =
      glGetUniformLocation(shader_program_, "u_DiffuseColor");

  attri_vertices_ = glGetAttribLocation(shader_program_, "a_Position");
  attri_uvs_ = glGetAttribLocation(shader_program_, "a_TexCoord");
  attri_normals_ = glGetAttribLocation(shader_program_, "a_Normal");

  util::LoadObjFile(asset_manager, "cube.obj", &cube_.vertices, &cube_.normals, &cube_.uvs, &cube_.indices);

  util::CheckGlError("obj_renderer::InitializeGlContent()");

  // initialize colors
  material_colors_.clear();
  scene_by_material_.clear();
  const int nMaterials = kBlockColorRgbaSize;
  const int randomOffset = std::rand();
  for (int i = 0; i < nMaterials; ++i) {
    material_colors_.push_back(GetBlockColor(i + randomOffset));
    scene_by_material_.push_back(Model());
  }
}

void GameRenderer::update(const Game& game, float game_scale) {

  const int nMaterials = material_colors_.size();

  std::vector< std::vector<Block> > blocks_by_material(nMaterials);

  for (Block block : game.getAllBlocks()) {
    const int materialId = block.pieceId % nMaterials;
    blocks_by_material[materialId].push_back(block);
  }

  for (int i = 0; i < nMaterials; ++i) {
    scene_by_material_[i] = blocksToModel(blocks_by_material[i], game.getDimensions(), game_scale);
  }
}

void GameRenderer::Draw(const glm::mat4& projection_mat,
                       const glm::mat4& view_mat, const glm::mat4& model_mat,
                       float light_intensity) const {
  if (!shader_program_) {
    LOGE("shader_program is null.");
    return;
  }

  glUseProgram(shader_program_);

  glm::mat4 mvp_mat = projection_mat * view_mat * model_mat * GAME_MODEL_TRANSFORM;
  glm::mat4 mv_mat = view_mat * model_mat;
  glm::vec4 view_light_direction = glm::normalize(mv_mat * kLightDirection);

  glUniform4f(uniform_lighting_param_, view_light_direction[0],
      view_light_direction[1], view_light_direction[2],
      light_intensity);

  glUniform4f(uniform_material_param_, AMBIENT, DIFFUSE, SPECULAR, SPECULAR_POWER);
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));
  glUniformMatrix4fv(uniform_mv_mat_, 1, GL_FALSE, glm::value_ptr(mv_mat));

  for (int i = 0; i < scene_by_material_.size(); ++i) {
    const Model& model = scene_by_material_[i];
    if (model.vertices.size() == 0) {
      continue;
    }

    const glm::vec3 color = material_colors_[i];

    glUniform3f(uniform_diffuse_color_, color.x, color.y, color.z);

    glEnableVertexAttribArray(attri_vertices_);
    glVertexAttribPointer(attri_vertices_, 3, GL_FLOAT, GL_FALSE, 0,
        model.vertices.data());

    glEnableVertexAttribArray(attri_normals_);
    glVertexAttribPointer(attri_normals_, 3, GL_FLOAT, GL_FALSE, 0,
        model.normals.data());

    glEnableVertexAttribArray(attri_uvs_);
    glVertexAttribPointer(attri_uvs_, 2, GL_FLOAT, GL_FALSE, 0, model.uvs.data());

    glDrawElements(GL_TRIANGLES, model.indices.size(), GL_UNSIGNED_SHORT,
        model.indices.data());

    glDisableVertexAttribArray(attri_vertices_);
    glDisableVertexAttribArray(attri_uvs_);
    glDisableVertexAttribArray(attri_normals_);
  }


  glUseProgram(0);
  util::CheckGlError("obj_renderer::Draw()");
}

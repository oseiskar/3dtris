
#include "game_box_renderer.h"
#include "game_renderer.h" // GAME_MODEL_TRANSFORM
#include "util.h"

namespace {
  constexpr char kVertexShader[] = R"(
  precision highp float;
  precision highp int;

  uniform mat4 u_ModelViewProjection;
  attribute vec4 a_Position;

  void main() {
     gl_Position = u_ModelViewProjection * vec4(a_Position.xyz, 1.0);
  })";

  constexpr char kFragmentShader[] = R"(
  precision highp float;
  precision highp int;
  uniform vec4 u_Color;

  void main() {
      gl_FragColor = u_Color;
  })";

  const glm::vec4 kColor = {1.0f, 0.0f, 1.0f, 0.5f};
}  // namespace

GameBoxRenderer::GameBoxRenderer(Pos3d dims) : dimensions(dims) {}

void GameBoxRenderer::InitializeGlContent(AAssetManager* asset_manager) {
  shader_program_ = util::CreateProgram(kVertexShader, kFragmentShader);
  util::CheckGlError("GameBoxRenderer::InitializeGlContent() - create program");

  if (!shader_program_) {
    LOGE("Could not create program.");
  }

  uniform_mvp_mat_ =
      glGetUniformLocation(shader_program_, "u_ModelViewProjection");
  //uniform_mv_mat_ = glGetUniformLocation(shader_program_, "u_ModelView");

  uniform_color_ = glGetUniformLocation(shader_program_, "u_Color");

  attri_vertices_ = glGetAttribLocation(shader_program_, "a_Position");
  //attri_uvs_ = glGetAttribLocation(shader_program_, "a_TexCoord");
  //attri_normals_ = glGetAttribLocation(shader_program_, "a_Normal");

  generateGameBoxVertices();

  util::CheckGlError("GameBoxRenderer::InitializeGlContent()");
}

void GameBoxRenderer::generateGameBoxVertices() {
  vertices_ = std::vector<GLfloat>({
    -0.5f*dimensions.x, -0.5f*dimensions.y, 0.0f,
     0.5f*dimensions.x, -0.5f*dimensions.y, 0.0f,
    -0.5f*dimensions.x,  0.5f*dimensions.y, 0.0f,
     0.5f*dimensions.x,  0.5f*dimensions.y, 0.0f,
    -0.5f*dimensions.x, -0.5f*dimensions.y, 1.0f*dimensions.z,
     0.5f*dimensions.x, -0.5f*dimensions.y, 1.0f*dimensions.z,
    -0.5f*dimensions.x,  0.5f*dimensions.y, 1.0f*dimensions.z,
     0.5f*dimensions.x,  0.5f*dimensions.y, 1.0f*dimensions.z
  });

  indices_ = std::vector<GLushort>({
      // bottom
      0, 1,
      1, 3,
      3, 2,
      2, 0,
      // vertical boundaries
      0, 4,
      1, 5,
      2, 6,
      3, 7,
  });
}

void GameBoxRenderer::Draw(const glm::mat4& projection_mat,
                           const glm::mat4& view_mat,
                           const glm::mat4& model_mat,
                           const float scale) {
  if (!shader_program_) {
    LOGE("shader_program is null.");
    return;
  }

  glUseProgram(shader_program_);
  //glDepthMask(GL_FALSE);
  glLineWidth(5.0);

  glm::mat4 scale_mat = glm::mat4(scale);
  scale_mat[3][3] = 1.0;

  glm::mat4 mvp_mat = projection_mat * view_mat * model_mat * GAME_MODEL_TRANSFORM * scale_mat;
  //glm::mat4 mv_mat = view_mat * model_mat;

  glUniform4f(uniform_color_, kColor.r, kColor.g, kColor.b, kColor.a);
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));
  //glUniformMatrix4fv(uniform_mv_mat_, 1, GL_FALSE, glm::value_ptr(mv_mat));

  glEnableVertexAttribArray(attri_vertices_);
  glVertexAttribPointer(attri_vertices_, 3, GL_FLOAT, GL_FALSE, 0, vertices_.data());

  /*glEnableVertexAttribArray(attri_normals_);
  glVertexAttribPointer(attri_normals_, 3, GL_FLOAT, GL_FALSE, 0,
                        normals_.data());

  glEnableVertexAttribArray(attri_uvs_);
  glVertexAttribPointer(attri_uvs_, 2, GL_FLOAT, GL_FALSE, 0, uvs_.data());*/

  glDrawElements(GL_LINES, indices_.size(), GL_UNSIGNED_SHORT, indices_.data());

  glDisableVertexAttribArray(attri_vertices_);
  //glDisableVertexAttribArray(attri_uvs_);
  //glDisableVertexAttribArray(attri_normals_);

  glUseProgram(0);
  //glDepthMask(GL_TRUE);
  util::CheckGlError("GameBoxRenderer::Draw()");
}

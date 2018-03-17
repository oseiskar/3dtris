#include "debug_renderer.h"
#include "game_box_renderer.h"
#include "util.h"

namespace {
  const glm::vec4 kColor = {1.0f, 0.0f, 0.0f, 0.5f};
}  // namespace

DebugRenderer::DebugRenderer() {}

void DebugRenderer::InitializeGlContent() {
  shader_program_ = util::CreateProgram(LINE_VERTEX_SHADER, LINE_FRAGMENT_SHADER);
  util::CheckGlError("GameBoxRenderer::InitializeGlContent() - create program");

  if (!shader_program_) {
    LOGE("Could not create program.");
  }

  uniform_mvp_mat_ =
      glGetUniformLocation(shader_program_, "u_ModelViewProjection");
  uniform_color_ = glGetUniformLocation(shader_program_, "u_Color");
  attri_vertices_ = glGetAttribLocation(shader_program_, "a_Position");

  util::CheckGlError("DebugRenderer::InitializeGlContent()");
}

void DebugRenderer::setLines(const std::vector< std::pair<glm::vec3, glm::vec3> > &lines) {
  vertices_.clear();
  indices_.clear();
  GLushort i = 0;
  for (auto p : lines) {
    vertices_.push_back(p.first.x);
    vertices_.push_back(p.first.y);
    vertices_.push_back(p.first.z);
    vertices_.push_back(p.second.x);
    vertices_.push_back(p.second.y);
    vertices_.push_back(p.second.z);

    indices_.push_back(i++);
    indices_.push_back(i++);
  }
}

void DebugRenderer::Draw(const glm::mat4& projection_mat,
                           const glm::mat4& view_mat) {
  if (!shader_program_) {
    LOGE("shader_program is null.");
    return;
  }

  if (vertices_.size() == 0) {
    return;
  }

  glUseProgram(shader_program_);
  //glDepthMask(GL_FALSE);
  glLineWidth(5.0);

  glm::mat4 mvp_mat = projection_mat * view_mat;

  glUniform4f(uniform_color_, kColor.r, kColor.g, kColor.b, kColor.a);
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  glEnableVertexAttribArray(attri_vertices_);
  glVertexAttribPointer(attri_vertices_, 3, GL_FLOAT, GL_FALSE, 0, vertices_.data());

  glDrawElements(GL_LINES, indices_.size(), GL_UNSIGNED_SHORT, indices_.data());

  glDisableVertexAttribArray(attri_vertices_);

  glUseProgram(0);
  //glDepthMask(GL_TRUE);
  util::CheckGlError("DebugRenderer::Draw()");
}

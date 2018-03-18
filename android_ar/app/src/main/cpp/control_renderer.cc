#include "control_renderer.h"
#include "game_box_renderer.h"
#include "util.h"

namespace {
  const glm::vec4 kColor = {1.0f, 1.0f, 1.0f, 0.5f};

  std::vector< std::pair<glm::vec3, glm::vec3> > drawRotationAnchors(
      const GameController &controller) {

    std::vector< std::pair<glm::vec3, glm::vec3> > lines;

    auto makeArc = [&lines](
        glm::vec3 pos,
        glm::vec3 r,
        glm::vec3 dir) {

      constexpr unsigned n = 20;
      constexpr float ang0 = (float)M_PI*0.02f;
      constexpr float ang1 = (float)M_PI*0.15f;

      glm::vec3 pPrev;

      for (int i = 0; i < n; ++i) {
        float ang = ang0 + (ang1-ang0)*i/(n-1);
        glm::vec3 p = r*cos(ang) + dir*sin(ang);
        if (i > 0) {
          lines.push_back(std::make_pair(
              pos + pPrev,
              pos + p));
        }
        pPrev = p;
      }
    };

    auto makeGimbal = [makeArc](const GameController::RotationAnchor& gimbal) {
      for (auto arc : gimbal.arcs) {
        makeArc(gimbal.origin, gimbal.r, arc.dir);
        makeArc(gimbal.origin, gimbal.r, -arc.dir);
      }
    };

    if (controller.hasActiveRotationAnchor()) {
      makeGimbal(controller.getActiveRotationAnchor());
    } else {
      for (auto gimbal : controller.getRotationAnchors()) {
        makeGimbal(gimbal);
      }
    }

    return lines;
  };
}

ControlRenderer::ControlRenderer(const GameController& controller): controller_(controller) {}

void ControlRenderer::InitializeGlContent() {
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

void ControlRenderer::setLines(const std::vector< std::pair<glm::vec3, glm::vec3> > &lines) {
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

void ControlRenderer::Draw(const glm::mat4& projection_mat,
                           const glm::mat4& view_mat,
                           const glm::mat4& model_mat) {
  if (!shader_program_) {
    LOGE("shader_program is null.");
    return;
  }

  setLines(drawRotationAnchors(controller_));

  glUseProgram(shader_program_);
  glDepthMask(GL_FALSE);
  glLineWidth(5.0);

  glm::mat4 mvp_mat = projection_mat * view_mat;

  glUniform4f(uniform_color_, kColor.r, kColor.g, kColor.b, kColor.a);
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  glEnableVertexAttribArray(attri_vertices_);
  glVertexAttribPointer(attri_vertices_, 3, GL_FLOAT, GL_FALSE, 0, vertices_.data());

  glDrawElements(GL_LINES, indices_.size(), GL_UNSIGNED_SHORT, indices_.data());

  glDisableVertexAttribArray(attri_vertices_);

  glUseProgram(0);
  glDepthMask(GL_TRUE);
  util::CheckGlError("ControlRenderer::Draw()");
}

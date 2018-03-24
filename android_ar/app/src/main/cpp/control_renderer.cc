#include <algorithm>

#include "control_renderer.h"
#include "game_box_renderer.h"
#include "util.h"

extern const glm::mat4 GAME_MODEL_TRANSFORM;

namespace {
  const glm::vec4 kColor = {1.0f, 1.0f, 1.0f, 0.5f};

  typedef ControlRenderer::LineList LineList;

  LineList drawRotationAnchors(const GameController &controller) {

    LineList lines;

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
  }

  LineList drawMoveArrows(const GameController &controller) {

    LineList lines;

    auto makeArrow = [&lines](
        glm::vec3 pos0,
        glm::vec3 dir,
        glm::vec3 side) {

      const float length = 0.5f;
      const float width = 2.0f;

      const glm::vec3 pos = pos0;
      const glm::vec3 tip = pos + dir * length;
      const glm::vec3 w = side * width * 0.5f;

      lines.push_back(std::make_pair(tip, pos + w));
      lines.push_back(std::make_pair(tip, pos - w));
    };

    auto dim = controller.getGame().getDimensions();
    const float d = (std::max(dim.x, dim.y)*0.5f + 3);

    const glm::vec3 x = util::RotateOnly(GAME_MODEL_TRANSFORM, glm::vec3(1,0,0));
    const glm::vec3 y = util::RotateOnly(GAME_MODEL_TRANSFORM, glm::vec3(0,1,0));

    makeArrow(x*d, x, y);
    makeArrow(-x*d, -x, y);
    makeArrow(y*d, y, x);
    makeArrow(-y*d, -y, x);

    return lines;
  }

  // one does not simply a + b arrays in C++
  LineList concat(const LineList &a, const LineList &b) {
    LineList r = a;
    r.insert(r.begin(), b.begin(), b.end());
    return r;
  }
}

ControlRenderer::ControlRenderer(const GameController& controller) :
    controller_(controller), arrows_(drawMoveArrows(controller_))
{}

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

void ControlRenderer::setLines(const LineList &lines) {
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
                           const glm::mat4& model_mat,
                           float game_scale) {
  if (!shader_program_) {
    LOGE("shader_program is null.");
    return;
  }



  glUseProgram(shader_program_);
  glDepthMask(GL_FALSE);
  glLineWidth(5.0);

  glUniform4f(uniform_color_, kColor.r, kColor.g, kColor.b, kColor.a);
  glEnableVertexAttribArray(attri_vertices_);

  setLines(drawRotationAnchors(controller_));
  glm::mat4 mvp_mat = projection_mat * view_mat;
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));
  glVertexAttribPointer(attri_vertices_, 3, GL_FLOAT, GL_FALSE, 0, vertices_.data());
  glDrawElements(GL_LINES, indices_.size(), GL_UNSIGNED_SHORT, indices_.data());

  setLines(arrows_);
  glm::mat4x4 scale = glm::mat4()*game_scale;
  scale[3][3] = 1.0f;
  mvp_mat = mvp_mat * model_mat * scale;
  glUniformMatrix4fv(uniform_mvp_mat_, 1, GL_FALSE, glm::value_ptr(mvp_mat));
  glVertexAttribPointer(attri_vertices_, 3, GL_FLOAT, GL_FALSE, 0, vertices_.data());
  glDrawElements(GL_LINES, indices_.size(), GL_UNSIGNED_SHORT, indices_.data());

  glDisableVertexAttribArray(attri_vertices_);

  glUseProgram(0);
  glDepthMask(GL_TRUE);
  util::CheckGlError("ControlRenderer::Draw()");
}

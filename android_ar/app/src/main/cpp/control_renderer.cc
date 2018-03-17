#include "control_renderer.h"
#include "game_box_renderer.h"
#include "util.h"

namespace {
  const glm::vec4 kColor = {1.0f, 1.0f, 1.0f, 0.5f};
}  // namespace

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

void ControlRenderer::drawGimbal(const glm::mat4& projection_mat,
                                 const glm::mat4& view_mat,
                                 const glm::mat4& model_mat) {
  const float x_ndc = 0.0;
  const float y_ndc = -0.7;

  const glm::mat4 inv_view = glm::inverse(view_mat);
  const glm::vec4 v = inv_view
                      * glm::inverse(projection_mat)
                      * glm::vec4(x_ndc, y_ndc, 1.0, 1.0);
  const glm::vec3 dir = glm::normalize(glm::vec3(v.x, v.y, v.z));
  const glm::vec3 origin = util::GetTranslation(inv_view);

  const float distance = 0.4;

  const glm::vec3 gimbalPos = origin + dir*distance;

  /*auto makeArrow = [gimbalPos, model_mat](int dx, int dy) {
    constexpr float d1 = 0.01;
    constexpr float d2 = 0.03;

    return std::make_pair(
        gimbalPos + util::RotateOnly(model_mat, glm::vec3(dx*d1, 0, dy*d1)),
        gimbalPos + util::RotateOnly(model_mat, glm::vec3(dx*d2, 0, dy*d2)));
  };

  setLines({
      makeArrow(1,0),
      makeArrow(-1,0),
      makeArrow(0,1),
      makeArrow(0,-1)
  });*/

  std::vector< std::pair<glm::vec3, glm::vec3> > lines;

  auto makeArc = [gimbalPos, model_mat, &lines](
      int x0, int y0, int z0,
      int dx, int dy, int dz) {

    constexpr unsigned n = 20;
    constexpr float ang0 = (float)M_PI*0.02f;
    constexpr float ang1 = (float)M_PI*0.15f;
    constexpr float r = 0.05;

    const glm::vec3 uAxis = r*glm::vec3(x0, y0, z0);
    const glm::vec3 vAxis = r*glm::vec3(dx, dy, dz);

    glm::vec3 pPrev;

    for (int i = 0; i < n; ++i) {
      float ang = ang0 + (ang1-ang0)*i/(n-1);
      glm::vec3 p = uAxis*cos(ang) + vAxis*sin(ang);
      if (i > 0) {
        lines.push_back(std::make_pair(
            gimbalPos + util::RotateOnly(model_mat, pPrev),
            gimbalPos + util::RotateOnly(model_mat, p)));
      }
      pPrev = p;
    }
  };

  /*makeArc(0, 1, 0, 1, 0, 0);
  makeArc(0, 1, 0, 0, 0, 1);
  makeArc(0, 0, 1, 1, 0, 0);*/

  auto makeArcs = [makeArc](int x0, int y0, int z0) {
    int dx1 = 0, dy1 = 0, dz1 = 0, dx2 = 0, dy2 = 0, dz2 = 0;
    if (x0 != 0) {
      dy1 = 1;
      dz2 = 1;
    } else if (y0 != 0) {
      dx1 = 1;
      dz2 = 1;
    } else {
      dx1 = 1;
      dy2 = 1;
    }
    makeArc(x0, y0, z0, dx1, dy1, dz1);
    makeArc(x0, y0, z0, -dx1, -dy1, -dz1);
    makeArc(x0, y0, z0, dx2, dy2, dz2);
    makeArc(x0, y0, z0, -dx2, -dy2, -dz2);
  };

  makeArcs(0,1,0);
  makeArcs(-1,0,0);
  makeArcs(0,0,1);

  setLines(lines);
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

  drawGimbal(projection_mat, view_mat, model_mat);

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

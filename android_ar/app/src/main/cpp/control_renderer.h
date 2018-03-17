
#ifndef C_CONTROL_RENDERER_H_
#define C_CONTROL_RENDERER_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "arcore_c_api.h"
#include "glm.h"
#include "api.hpp"

class ControlRenderer {
public:
  ControlRenderer() = default;
  ~ControlRenderer() = default;

  void InitializeGlContent();

  void Draw(const glm::mat4& projection_mat,
            const glm::mat4& view_mat,
            const glm::mat4& model_mat);

private:
  void drawGimbal(const glm::mat4& projection_mat,
                  const glm::mat4& view_mat,
                  const glm::mat4& model_mat);

  void setLines(const std::vector< std::pair<glm::vec3, glm::vec3> > &lines);

  std::vector<GLfloat> vertices_;
  std::vector<GLushort> indices_;

  GLuint shader_program_;

  GLuint attri_vertices_;
  GLuint uniform_mvp_mat_;
  GLuint uniform_color_;
};

#endif
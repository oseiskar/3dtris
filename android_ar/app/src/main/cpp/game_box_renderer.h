
#ifndef C_GAME_BOX_RENDERER_H_
#define C_GAME_BOX_RENDERER_H_

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

class GameBoxRenderer {
public:
  GameBoxRenderer(Pos3d dimensions);
  ~GameBoxRenderer() = default;

  // Sets up OpenGL state used by the plane renderer.  Must be called on the
  // OpenGL thread.
  void InitializeGlContent(AAssetManager* asset_manager);

  // Draws the provided plane.
  void Draw(const glm::mat4& projection_mat,
            const glm::mat4& view_mat,
            const glm::mat4& model_mat,
            const float scale);

private:
  void generateGameBoxVertices();

  const Pos3d dimensions;

  std::vector<GLfloat> vertices_;
  //std::vector<GLfloat> uvs_;
  //std::vector<GLfloat> normals_;
  std::vector<GLushort> indices_;

  GLuint shader_program_;
  GLuint attri_vertices_;
  //GLuint attri_uvs_;
  //GLuint attri_normals_;
  GLuint uniform_mvp_mat_;
  GLuint uniform_mv_mat_;
  GLuint uniform_color_;
};

#endif
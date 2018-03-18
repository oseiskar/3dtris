
#ifndef C_GAME_CONTROLLER_H_
#define C_GAME_CONTROLLER_H_

#include <array>
#include <glm.h>
#include "api.hpp"

class GameController {
public:
  enum class State {
    WAITING_FOR_PLANE,
    WAITING_FOR_BOX,
    RUNNING, // includes game over state
    PAUSED_TRACKING_LOST
  };

  GameController();
  ~GameController() = default;

  const Game& getGame() const { return *game; }
  const State getState() const { return state; }

  bool getTrackingState() const;
  bool hasStarted() const;

  bool onFrame(uint64_t timestamp);
  void onTrackingState(bool isTracking);
  void onBoxFound();

  void setScene(glm::mat4x4 projection, glm::mat4x4 view, glm::mat4x4 model, int w, int h);
  void onTap(float x, float y);
  void onLongPress(float x, float y);
  void onScroll(float x1, float y1, float x2, float y2, float dx, float dy);
  void onTouchUp(float x, float y);

  struct RotationAnchor {
    glm::vec3 origin;
    glm::vec3 r;
    glm::vec2 r_screen;

    struct Arc {
      Axis rotation_axis;
      glm::vec3 dir;
      glm::vec2 dir_screen;
    };

    std::vector<Arc> arcs;
  };

  const std::array<RotationAnchor, 2> &getRotationAnchors() const {
    return rotation_anchors;
  };

  bool hasActiveRotationAnchor() const {
    return active_anchor_index >= 0;
  }

  RotationAnchor getActiveRotationAnchor() const;

private:
  std::unique_ptr<Game> game;
  State state;

  uint64_t prevTimestamp;
  bool changed_by_controls;

  glm::mat4x4 projection_mat, view_mat, model_mat;
  int screen_width, screen_height;

  std::array<RotationAnchor, 2> rotation_anchors;
  int active_anchor_index;
  RotationAnchor active_rotation_anchor;
  std::vector<float> drag_distances;

  void updateRotationAnchors();

  glm::vec2 ndcToScreen(glm::vec2 ndc) const;

  // controls
  void moveXY(int dx, int dy);
  void rotate(Axis ax, RotationDirection dir);
  void drop();
};


#endif
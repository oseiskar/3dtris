
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

  /**
   * Represents the state of the on-screen control that is used to rotate pieces.
   * Anchor has one or more arcs that correspond to possible rotation axes
   * controllable with that anchor.
   */
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

  struct DropArrow {
    glm::vec3 r;
    glm::vec3 dir;
    glm::vec3 side_dir;
    glm::vec2 r_screen;
  };

  const std::array<RotationAnchor, 2> &getRotationAnchors() const {
    return rotation_anchors;
  };

  bool hasActiveRotationAnchor() const {
    return active_anchor_index >= 0;
  }

  RotationAnchor getActiveRotationAnchor() const;

  DropArrow getDropArrow() const {
    return drop_arrow;
  }

private:
  std::unique_ptr<Game> game;
  State state;

  uint64_t prev_timestamp;
  bool changed_by_controls;

  glm::mat4x4 projection_mat, view_mat, model_mat;
  int screen_width, screen_height;

  // There are two anchors (things that can be dragged aroud to rotate).
  // The state of these members changes with camera pose but not by dragging
  std::array<RotationAnchor, 2> rotation_anchors;
  // Which of these anchors is currently being dragged (-1 if neither)
  int active_anchor_index;

  // The state of this member changes also changes with dragging. This state
  // should be rendered when dragging is on-going
  RotationAnchor dragged_rotation_anchor;
  // How much the active anchor has been dragged along each arc
  std::vector<float> drag_distances;

  DropArrow drop_arrow;

  void updateRotationAnchors();
  void updateDropArrow();

  glm::vec2 ndcToScreen(glm::vec2 ndc) const;

  // controls
  void moveXY(int dx, int dy);
  void rotate(Axis ax, RotationDirection dir);
  void drop();
};


#endif
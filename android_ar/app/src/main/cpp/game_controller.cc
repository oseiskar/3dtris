
#include "game_controller.h"
#include "util.h"

namespace {

inline static unsigned int getRandomSeedFromTime() {
  // use the current time modulo something as the random seed
  // ... for the game, this is not crypto stuff!
  struct timespec res;
  clock_gettime(CLOCK_REALTIME, &res);
  return static_cast<unsigned int>(res.tv_nsec ^ res.tv_sec);
}

static glm::vec3 axisToVec(Axis ax) {
  switch (ax) {
    case Axis::X:
      return glm::vec3(1,0,0);
    case Axis::Y:
      return glm::vec3(0,1,0);
    case Axis::Z:
      return glm::vec3(0,0,1);
  }
}

constexpr int64_t MAX_FRAME_TIME = static_cast<int64_t>(0.1 * 1e9);
}

GameController::GameController() :
    game(buildGame(getRandomSeedFromTime())),
    state(State::WAITING_FOR_PLANE),
    prevTimestamp(0),
    changed_by_controls(false),
    active_anchor_index(-1)
{}

void GameController::onTrackingState(bool isTracking) {
  if (isTracking) {
    switch (state) {
      case State::WAITING_FOR_PLANE:
        state = State::WAITING_FOR_BOX;
        LOGI("GameController: WAITING_FOR_BOX");
        break;
      case State::PAUSED_TRACKING_LOST:
        state = State::RUNNING;
        LOGI("GameController: recovered PAUSED_TRACKING_LOST -> RUNNING");
        break;
      default:
        break;
    }
  } else {
    switch (state) {
      case State::RUNNING:
        state = State::PAUSED_TRACKING_LOST;
        LOGI("GameController: RUNNING -> PAUSED_TRACKING_LOST");
        break;
      case State::WAITING_FOR_BOX:
        state = State::WAITING_FOR_PLANE;
        LOGI("GameController: WAITING_FOR_BOX -> WAITING_FOR_PLANE");
        break;
      default:
        break;
    }
  }
}

bool GameController::getTrackingState() const {
  return state == State::RUNNING || state == State::WAITING_FOR_BOX;
}

bool GameController::hasStarted() const {
  return state >= State::RUNNING;
}

void GameController::onBoxFound() {
  state = State::RUNNING;
  LOGI("GameController: box found, state RUNNING");
}

bool GameController::onFrame(uint64_t timestamp) {
  bool changed = changed_by_controls;
  if (state == State::RUNNING && !game->isOver()) {
    int64_t dt = timestamp - prevTimestamp;
    if (dt < 0) dt = 0;
    if (dt > MAX_FRAME_TIME) dt = MAX_FRAME_TIME;
    unsigned int dtMilliseconds = static_cast<unsigned int>(dt / 1000000);

    changed = changed || game->tick(dtMilliseconds);
  }
  changed_by_controls = false;
  prevTimestamp = timestamp;
  return changed;
}

void GameController::setScene(glm::mat4x4 projection, glm::mat4x4 view, glm::mat4x4 model, int w, int h) {
  screen_height = h;
  screen_width = w;
  projection_mat = projection;
  view_mat = view;
  model_mat = model;

  updateRotationAnchors();
  updateDropArrow();
}

glm::vec2 GameController::ndcToScreen(glm::vec2 ndc) const {
  //return glm::vec2(ndc.x/(float)screen_width, 1.0 - ndc.y/(float)screen_height)*2.0f - 1.0f;
  return glm::vec2((ndc.x*0.5 + 0.5)*screen_width, (-ndc.y*0.5 + 0.5)*screen_height);
}

extern const glm::mat4 GAME_MODEL_TRANSFORM;

void GameController::updateDropArrow() {

  const float x_ndc = -0.75f;
  const float y_ndc = 0.0;

  const glm::mat4 inv_view = glm::inverse(view_mat);
  const glm::vec4 v = inv_view
                      * glm::inverse(projection_mat)
                      * glm::vec4(x_ndc, y_ndc, 1.0, 1.0);
  const glm::vec3 dir = glm::normalize(glm::vec3(v.x, v.y, v.z));
  const glm::vec3 origin = util::GetTranslation(inv_view);

  const glm::mat4x4 model_mat = this->model_mat * GAME_MODEL_TRANSFORM; // hacky

  // anchor sphere position
  constexpr float distance = 0.4;
  constexpr float drop_arrow_scale = 0.03;

  drop_arrow.r = origin + dir*distance;
  drop_arrow.dir = util::RotateOnly(model_mat, glm::vec3(0,0,-1)) * drop_arrow_scale;
  drop_arrow.side_dir = glm::normalize(glm::vec3(dir.z, 0, -dir.x)) * drop_arrow_scale;

  glm::vec4 screen_coords = projection_mat * view_mat * glm::vec4(drop_arrow.r, 1);
  drop_arrow.r_screen = ndcToScreen(glm::vec2(screen_coords.x, screen_coords.y) / screen_coords.w);

}

void GameController::updateRotationAnchors() {

  // anchor sphere position
  const float x_ndc = 0.7;
  const float y_ndc = 0.0;

  const glm::mat4 inv_view = glm::inverse(view_mat);
  const glm::vec4 v = inv_view
                      * glm::inverse(projection_mat)
                      * glm::vec4(x_ndc, y_ndc, 1.0, 1.0);
  const glm::vec3 dir = glm::normalize(glm::vec3(v.x, v.y, v.z));
  const glm::vec3 origin = util::GetTranslation(inv_view);

  const float distance = 0.4;

  const glm::vec3 anchor_origin = origin + dir*distance;
  const glm::mat4x4 model_mat = this->model_mat * GAME_MODEL_TRANSFORM; // hacky
  const glm::mat4x4 view_mat = this->view_mat;
  const glm::mat4x4 projection_mat = this->projection_mat;
  auto that = this;

  auto make_anchor = [anchor_origin, model_mat, view_mat, projection_mat, that](
      glm::vec3 r0, std::vector<Axis> rotation_arcs, float r) {

    RotationAnchor g;
    g.origin = anchor_origin;

    g.r = util::RotateOnly(model_mat, r*r0);
    glm::vec4 screen_coords = projection_mat * view_mat * glm::vec4(g.origin + g.r, 1);
    g.r_screen = that->ndcToScreen(glm::vec2(screen_coords.x, screen_coords.y) / screen_coords.w);

    for (Axis ax : rotation_arcs) {
      RotationAnchor::Arc arc;
      arc.rotation_axis = ax;
      glm::vec3 v = glm::cross(axisToVec(ax), r0);
      arc.dir = util::RotateOnly(model_mat, r*v);
      glm::vec4 screen_v = projection_mat * view_mat * glm::vec4(g.origin + g.r + v, 1);
      arc.dir_screen = that->ndcToScreen(glm::vec2(screen_v.x, screen_v.y) / screen_v.w) - g.r_screen;
      g.arcs.push_back(arc);
    }

    // longer direction arrow in screen coordinates determines gesture directions
    if (g.arcs.size() > 1) {
      assert(g.arcs.size() == 2);
      int longer;
      if (glm::length(g.arcs[0].dir_screen) > glm::length(g.arcs[1].dir_screen)) {
        longer = 0;
      } else {
        longer = 1;
      }
      int shorter = 1 - longer;
      const glm::vec2 p = glm::normalize(g.arcs[longer].dir_screen);
      const glm::vec2 q = g.arcs[shorter].dir_screen;

      g.arcs[longer].dir_screen = p;
      g.arcs[shorter].dir_screen = glm::normalize(q - glm::dot(q, p)*p);
    } else {
      g.arcs[0].dir_screen = glm::normalize(g.arcs[0].dir_screen);
    }

    return g;
  };

  const glm::vec3 z_anchor_r = glm::normalize(util::RotateOnly(
      glm::inverse(model_mat),
      -glm::vec3(dir.x, 0, dir.z)));

  rotation_anchors[0] = make_anchor(z_anchor_r, {Axis::Z}, 0.05);
  rotation_anchors[1] = make_anchor(glm::vec3(0,0,1), {Axis::X, Axis::Y}, 0.05);
}

void GameController::onTap(float x, float y) {

  onTouchUp(x, y);

  // check drop arrow hit
  constexpr float MAX_DISTANCE_TO_DROP_ARROW = 60;
  const float drop_arrow_dist = glm::length(drop_arrow.r_screen - glm::vec2(x, y));
  if (drop_arrow_dist < MAX_DISTANCE_TO_DROP_ARROW && !hasActiveRotationAnchor()) {
    drop();
    return;
  }

  if (x/(float)screen_width < 0.3*0.5) {
    return; // don't move horizontally when barely missing the drop arrow
  }

  glm::vec3 touch_origin, touch_dir;
  util::GetTouchRay(projection_mat, view_mat, x, y, screen_width, screen_height, touch_origin, touch_dir);
  //LOGI("dir %f %f %f", touch_dir.x, touch_dir.y, touch_dir.z);
  //LOGI("origin %f %f %f", touch_origin.x, touch_origin.y, touch_origin.z);

  const glm::vec3 game_origin = util::GetTranslation(model_mat);
  const float delta_height = (touch_origin - game_origin).y;
  if (delta_height > 0 && touch_dir.y < 0) {
    const float dist = -delta_height / touch_dir.y;
    const glm::vec3 base_hit = dist*touch_dir + touch_origin - game_origin;

    //debug_renderer_.setLines({ std::make_pair(touch_origin, base_hit + game_origin) });

    const float hit_x = base_hit.x;
    const float hit_y = -base_hit.z;
    LOGI("hit %f %f", hit_x, hit_y);

    // determine quadrant
    if (abs(hit_x) > abs(hit_y)) {
      moveXY(hit_x > 0 ? 1 : -1, 0);
    } else {
      moveXY(0, hit_y > 0 ? 1 : -1);
    }
  } // else no front-face ray intersection
}


void GameController::onLongPress(float x, float y) {
  //LOGI("long press %f %f", x, y);
}

void GameController::onScroll(float x1, float y1, float x2, float y2, float dx, float dy) {
  glm::vec2 pos = glm::vec2(x2, y2);
  //LOGI("scroll pos %f %f", pos.x, pos.y);

  constexpr float MAX_DISTANCE_TO_ROTATION_ANCHOR = 150;

  float minDist = 0;
  if (active_anchor_index < 0) {
    for (int i = 0; i < rotation_anchors.size(); ++i) {
      float dist = glm::length(rotation_anchors[i].r_screen - pos);
      if (i == 0 || dist < minDist) {
        minDist = dist;
        if (dist < MAX_DISTANCE_TO_ROTATION_ANCHOR) {
          active_anchor_index = i;
        }
      }
    }
    if (active_anchor_index >= 0) {
      drag_distances = std::vector<float>(rotation_anchors[active_anchor_index].arcs.size(), 0.0);
    }
  } else {
    const glm::vec2 dvec(dx, dy);

    float max_drag = 0;
    int best_arc_index = 0;
    for (int i = 0; i < drag_distances.size(); ++i) {
      const RotationAnchor::Arc &arc = rotation_anchors[active_anchor_index].arcs[i];

      drag_distances[i] += glm::dot(arc.dir_screen, dvec);
      if (abs(drag_distances[i]) >= abs(max_drag)) {
        best_arc_index = i;
        max_drag = drag_distances[i];
      }
    }

    // set active anchor
    RotationAnchor anchor = rotation_anchors[active_anchor_index];

    const float PIXELS_TO_90_DEG_ROTATION = 200;
    const float ANGLE_PER_PIXEL = M_PI*0.5 / PIXELS_TO_90_DEG_ROTATION;

    const float ang = -max_drag * ANGLE_PER_PIXEL; // mystery sign flip

    const glm::vec3 r0 = anchor.r, v0 = anchor.arcs[best_arc_index].dir;
    anchor.r = cos(ang) * r0 + sin(ang) * v0;;
    anchor.arcs[best_arc_index].dir = -sin(ang) * r0 + cos(ang) * v0;

    active_rotation_anchor = anchor;

    const float ROTATION_THRESHOLD = M_PI*0.25; // 45 degrees

    if (abs(ang) > ROTATION_THRESHOLD) {
      rotate(anchor.arcs[best_arc_index].rotation_axis,
          ang > 0 ? RotationDirection::CCW : RotationDirection::CW);

      // reset drag distances
      drag_distances = std::vector<float>(rotation_anchors[active_anchor_index].arcs.size(), 0.0);
    }
  }
}

void GameController::onTouchUp(float x, float y) {
  active_anchor_index = -1;
  drag_distances.clear();
}

GameController::RotationAnchor GameController::getActiveRotationAnchor() const {
  assert(hasActiveRotationAnchor());
  return active_rotation_anchor;
}

void GameController::moveXY(int dx, int dy) {
  changed_by_controls = changed_by_controls || game->moveXY(dx, dy);
}

void GameController::rotate(Axis ax, RotationDirection dir) {
  changed_by_controls = changed_by_controls || game->rotate(ax, dir);
}

void GameController::drop() {
  game->drop();
  changed_by_controls = true;
}
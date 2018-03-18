
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
    active_gimbal_index(-1)
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

  setGimbals();
}

glm::vec2 GameController::ndcToScreen(glm::vec2 ndc) const {
  //return glm::vec2(ndc.x/(float)screen_width, 1.0 - ndc.y/(float)screen_height)*2.0f - 1.0f;
  return glm::vec2((ndc.x*0.5 + 0.5)*screen_width, (-ndc.y*0.5 + 0.5)*screen_height);
}

extern const glm::mat4 GAME_MODEL_TRANSFORM;

void GameController::setGimbals() {

  // gimbal sphere position
  const float x_ndc = 0.0;
  const float y_ndc = -0.7;

  const glm::mat4 inv_view = glm::inverse(view_mat);
  const glm::vec4 v = inv_view
                      * glm::inverse(projection_mat)
                      * glm::vec4(x_ndc, y_ndc, 1.0, 1.0);
  const glm::vec3 dir = glm::normalize(glm::vec3(v.x, v.y, v.z));
  const glm::vec3 origin = util::GetTranslation(inv_view);

  const float distance = 0.4;

  const glm::vec3 gimbal_origin = origin + dir*distance;
  const glm::mat4x4 model_mat = this->model_mat * GAME_MODEL_TRANSFORM; // TODO: hacky
  const glm::mat4x4 view_mat = this->view_mat;
  const glm::mat4x4 projection_mat = this->projection_mat;
  auto that = this;

  auto make_gimbal = [gimbal_origin, model_mat, view_mat, projection_mat, that](
      Axis axis,
      int dir) {
    constexpr float r = 0.05;

    GimbalControl g;
    switch (axis) {
      case Axis::X:
        g.u_axis = Axis::Y;
        g.v_axis = Axis::Z;
        break;
      case Axis::Y:
        g.u_axis = Axis::X;
        g.v_axis = Axis::Z;
        break;
      case Axis::Z:
        g.u_axis = Axis::X;
        g.v_axis = Axis::Y;
        break;
    }

    glm::vec3 r0 = (float)dir*axisToVec(axis);
    glm::vec3 u0 = glm::cross(axisToVec(g.u_axis), r0);
    glm::vec3 v0 = glm::cross(axisToVec(g.v_axis), r0);

    g.origin = gimbal_origin;
    g.r = util::RotateOnly(model_mat, r*r0);
    g.u = util::RotateOnly(model_mat, r*u0);
    g.v = util::RotateOnly(model_mat, r*v0);

    glm::vec4 screen_coords = projection_mat * view_mat * glm::vec4(g.origin + g.r, 1);
    glm::vec4 screen_u = projection_mat * view_mat * glm::vec4(g.origin + g.r + g.u, 1);
    glm::vec4 screen_v = projection_mat * view_mat * glm::vec4(g.origin + g.r + g.v, 1);

    g.r_screen = that->ndcToScreen(glm::vec2(screen_coords.x, screen_coords.y) / screen_coords.w);
    g.u_screen = that->ndcToScreen(glm::vec2(screen_u.x, screen_u.y) / screen_u.w) - g.r_screen;
    g.v_screen = that->ndcToScreen(glm::vec2(screen_v.x, screen_v.y) / screen_v.w) - g.r_screen;

    // longer direction arrow in screen coordinates determines gesture directions
    if (glm::length(g.u_screen) > glm::length(g.v_screen)) {
      g.u_screen = glm::normalize(g.u_screen);
      g.v_screen = glm::normalize(g.v_screen - glm::dot(g.v_screen, g.u_screen)*g.u_screen);
    } else {
      g.v_screen = glm::normalize(g.v_screen);
      g.u_screen = glm::normalize(g.u_screen - glm::dot(g.u_screen, g.v_screen)*g.v_screen);
    }

    /*LOGI("r_screen %f %f", g.r_screen.x, g.r_screen.y);
    LOGI("u_screen %f %f", g.u_screen.x, g.u_screen.y);
    LOGI("v_screen %f %f", g.v_screen.x, g.v_screen.y);*/

    return g;
  };

  gimbals[0] = make_gimbal(Axis::X, -1);
  gimbals[1] = make_gimbal(Axis::Y, 1);
  gimbals[2] = make_gimbal(Axis::Z, 1);
}

void GameController::onTap(float x, float y) {

  onTouchUp(x, y);

  glm::vec3 touch_origin, touch_dir;
  util::GetTouchRay(projection_mat, view_mat, x, y, screen_width, screen_height, touch_origin, touch_dir);
  LOGI("dir %f %f %f", touch_dir.x, touch_dir.y, touch_dir.z);
  LOGI("origin %f %f %f", touch_origin.x, touch_origin.y, touch_origin.z);

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

void GameController::onScroll(float x1, float y1, float x2, float y2, float dx, float dy) {
  glm::vec2 pos = glm::vec2(x2, y2);
  LOGI("scroll pos %f %f", pos.x, pos.y);
  float minDist = 0;
  if (active_gimbal_index < 0) {
    for (int i = 0; i < gimbals.size(); ++i) {
      float dist = glm::length(gimbals[i].r_screen - pos);
      if (i == 0 || dist < minDist) {
        minDist = dist;
        active_gimbal_index = i;
      }
    }
  } else {
    const glm::vec2 dvec(dx, dy);
    LOGI("dvec %f %f", dx, dy);
    active_u_dist += glm::dot(gimbals[active_gimbal_index].u_screen, dvec);
    active_v_dist += glm::dot(gimbals[active_gimbal_index].v_screen, dvec);

    // set active gimbal

    GimbalControl gimbal = gimbals[active_gimbal_index];

    const float PIXELS_TO_90_DEG_ROTATION = 200;
    const float ANGLE_PER_PIXEL = M_PI*0.5 / PIXELS_TO_90_DEG_ROTATION;

    int sign = -1; // mystery sign flip

    Axis ax;
    float ang;

    if (abs(active_u_dist) > abs(active_v_dist)) {
      ax = gimbal.u_axis;
      ang = sign * active_u_dist * ANGLE_PER_PIXEL;
      const glm::vec3 newU = -sin(ang) * gimbal.r + cos(ang) * gimbal.u;
      gimbal.r = cos(ang) * gimbal.r + sin(ang) * gimbal.u;
      gimbal.u = newU;
    } else {
      ax = gimbal.v_axis;
      ang = sign * active_v_dist * ANGLE_PER_PIXEL;
      const glm::vec3 newV = -sin(ang) * gimbal.r + cos(ang) * gimbal.v;
      gimbal.r = cos(ang) * gimbal.r + sin(ang) * gimbal.v;
      gimbal.v = newV;
    }

    active_gimbal = gimbal;
    const float ROTATION_THRESHOLD = M_PI*0.25; // 45 degrees

    if (abs(ang) > ROTATION_THRESHOLD) {
      rotate(ax, ang > 0 ? RotationDirection::CCW : RotationDirection::CW);
      onTouchUp(x1, y1);
    }
  }
}

void GameController::onTouchUp(float x, float y) {
  active_gimbal_index = -1;
  active_v_dist = 0;
  active_u_dist = 0;
}

GameController::GimbalControl GameController::getActiveGimbal() const {
  assert(hasActiveGimbal());
  return active_gimbal;
}

void GameController::moveXY(int dx, int dy) {
  changed_by_controls = changed_by_controls || game->moveXY(dx, dy);
}

void GameController::rotate(Axis ax, RotationDirection dir) {
  changed_by_controls = changed_by_controls || game->rotate(ax, dir);
}
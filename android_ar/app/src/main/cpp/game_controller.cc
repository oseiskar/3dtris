
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
    changed_by_controls(false)
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
  const glm::mat4x4 model_mat = this->model_mat;

  auto make_gimbal = [gimbal_origin, model_mat](
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
    return g;
  };

  gimbals[0] = make_gimbal(Axis::X, -1);
  gimbals[1] = make_gimbal(Axis::Y, 1);
  gimbals[2] = make_gimbal(Axis::Z, 1);

  active_gimbal = 0;
}

void GameController::onTap(float x, float y) {

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

}

void GameController::moveXY(int dx, int dy) {
  changed_by_controls = changed_by_controls || game->moveXY(dx, dy);
}
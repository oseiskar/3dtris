
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

constexpr int64_t MAX_FRAME_TIME = static_cast<int64_t>(0.1 * 1e9);
}

GameController::GameController() :
    game(buildGame(getRandomSeedFromTime())),
    state(State::WAITING_FOR_PLANE),
    prevTimestamp(0)
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

void GameController::onBoxFound() {
  state = State::RUNNING;
  LOGI("GameController: box found, state RUNNING");
}

bool GameController::onFrame(uint64_t timestamp) {
  bool changed = false;
  if (state == State::RUNNING) {
    int64_t dt = timestamp - prevTimestamp;
    if (dt < 0) dt = 0;
    if (dt > MAX_FRAME_TIME) dt = MAX_FRAME_TIME;
    unsigned int dtMilliseconds = static_cast<unsigned int>(dt / 1000000);

    changed = game->tick(dtMilliseconds);
  }
  prevTimestamp = timestamp;
  return changed;
}


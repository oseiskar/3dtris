
#ifndef C_GAME_CONTROLLER_H_
#define C_GAME_CONTROLLER_H_

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

private:
  std::unique_ptr<Game> game;
  State state;

  uint64_t prevTimestamp;
};


#endif
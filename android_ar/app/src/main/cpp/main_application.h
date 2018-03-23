/*
 * Copyright 2017 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Modifications copyright (C) Otto Seiskari 2018
 */

#ifndef C_MAIN_APPLICATION_H_
#define C_MAIN_APPLICATION_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <jni.h>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

#include "arcore_c_api.h"
#include "background_renderer.h"
#include "glm.h"
#include "game_box_renderer.h"
#include "game_renderer.h"
#include "control_renderer.h"
#include "debug_renderer.h"
#include "util.h"
#include "game_controller.h"

// MainApplication handles all application logics.
class MainApplication {
public:
  // Constructor and deconstructor.
  MainApplication() = default;
  MainApplication(AAssetManager* asset_manager);
  ~MainApplication();

  // OnPause is called on the UI thread from the Activity's onPause method.
  void OnPause();

  // OnResume is called on the UI thread from the Activity's onResume method.
  void OnResume(void* env, void* context, void* activity);

  // OnSurfaceCreated is called on the OpenGL thread when GLSurfaceView
  // is created.
  void OnSurfaceCreated();

  // OnDisplayGeometryChanged is called on the OpenGL thread when the
  // render surface size or display rotation changes.
  //
  // @param display_rotation: current display rotation.
  // @param width: width of the changed surface view.
  // @param height: height of the changed surface view.
  void OnDisplayGeometryChanged(int display_rotation, int width, int height);

  // OnDrawFrame is called on the OpenGL thread to render the next frame.
  void OnDrawFrame();

  // OnTap is called on the OpenGL thread after the user touches the screen.
  // @param x: x position on the screen (pixels).
  // @param y: y position on the screen (pixels).
  void OnTap(float x, float y);

  void OnLongPress(float x, float y);

  void OnTouchUp(float x, float y);

  void OnScroll(float x1, float y1, float x2, float y2, float dx, float dy);

  void OnFling(float x1, float y1, float x2, float y2, float vx, float vy);

  // Returns true if any planes have been detected.  Used for hiding the
  // "searching for planes" snackbar.
  bool IsTracking() const {
    return game_controller_.getTrackingState();
  }
  bool HasGameStarted() const {
    return game_controller_.hasStarted();
  }
  bool IsGameOver() const {
    return game_controller_.getGame().isOver();
  }
  int GetScore() const {
    return game_controller_.getGame().getScore();
  }

private:
  ArSession* ar_session_ = nullptr;
  ArFrame* ar_frame_ = nullptr;

  bool install_requested_ = false;
  int width_ = 1;
  int height_ = 1;
  int display_rotation_ = 0;

  AAssetManager* const asset_manager_;

  // The anchors at which we are drawing android models
  std::vector<ArAnchor*> tracked_obj_set_;

  GameController game_controller_;

  BackgroundRenderer background_renderer_;
  GameBoxRenderer game_box_renderer_;
  GameRenderer game_renderer_;
  ControlRenderer control_renderer_;
  DebugRenderer debug_renderer_;

  glm::mat4x4 game_model_mat_;
  float game_scale_;
};

#endif

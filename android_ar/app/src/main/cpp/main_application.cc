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

#include "main_application.h"

#include <android/asset_manager.h>
#include <time.h>
#include <string>

#include "util.h"

namespace {
constexpr size_t kMaxAnchors = 20;

template <class F>
void getHit(ArSession *ar_session_, ArFrame *ar_frame_, float x, float y, bool polygonTest, F f) {

  if (ar_frame_ != nullptr && ar_session_ != nullptr) {
    ArHitResultList* hit_result_list = nullptr;
    ArHitResultList_create(ar_session_, &hit_result_list);
    CHECK(hit_result_list);
    ArFrame_hitTest(ar_session_, ar_frame_, x, y, hit_result_list);

    int32_t hit_result_list_size = 0;
    ArHitResultList_getSize(ar_session_, hit_result_list,
        &hit_result_list_size);

    // The hitTest method sorts the resulting list by distance from the camera,
    // increasing.  The first hit result will usually be the most relevant when
    // responding to user input.

    ArHitResult* ar_hit_result = nullptr;
    for (int32_t i = 0; i < hit_result_list_size; ++i) {
      ArHitResult* ar_hit = nullptr;
      ArHitResult_create(ar_session_, &ar_hit);
      ArHitResultList_getItem(ar_session_, hit_result_list, i, ar_hit);

      if (ar_hit == nullptr) {
        LOGE("MainApplication::OnTouched ArHitResultList_getItem error");
        return;
      }

      ArTrackable* ar_trackable = nullptr;
      ArHitResult_acquireTrackable(ar_session_, ar_hit, &ar_trackable);
      ArTrackableType ar_trackable_type = AR_TRACKABLE_NOT_VALID;
      ArTrackable_getType(ar_session_, ar_trackable, &ar_trackable_type);
      // Creates an anchor if a plane or an oriented point was hit.
      if (AR_TRACKABLE_PLANE == ar_trackable_type) {
        ArPose* ar_pose = nullptr;
        ArPose_create(ar_session_, nullptr, &ar_pose);
        ArHitResult_getHitPose(ar_session_, ar_hit, ar_pose);
        int32_t in_polygon = 0;
        ArPlane* ar_plane = ArAsPlane(ar_trackable);
        ArPlane_isPoseInPolygon(ar_session_, ar_plane, ar_pose, &in_polygon);
        ArPose_destroy(ar_pose);
        if (!in_polygon && polygonTest) {
          continue;
        }

        ar_hit_result = ar_hit;
        break;
      } else if (AR_TRACKABLE_POINT == ar_trackable_type) {
        ArPoint* ar_point = ArAsPoint(ar_trackable);
        ArPointOrientationMode mode;
        ArPoint_getOrientationMode(ar_session_, ar_point, &mode);
        if (AR_POINT_ORIENTATION_ESTIMATED_SURFACE_NORMAL == mode) {
          ar_hit_result = ar_hit;
          break;
        }
      }
    }

    if (ar_hit_result) {
      // Note that the application is responsible for releasing the anchor
      // pointer after using it. Call ArAnchor_release(anchor) to release.
      ArAnchor* anchor = nullptr;
      if (ArHitResult_acquireNewAnchor(ar_session_, ar_hit_result, &anchor) !=
          AR_SUCCESS) {
        LOGE(
            "MainApplication::OnTouched ArHitResult_acquireNewAnchor error");
        return;
      }

      ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED;
      ArAnchor_getTrackingState(ar_session_, anchor, &tracking_state);
      if (tracking_state != AR_TRACKING_STATE_TRACKING) {
        ArAnchor_release(anchor);
        return;
      }

      if (!f(anchor)) {
        ArAnchor_release(anchor);
      }

      ArHitResult_destroy(ar_hit_result);
      ar_hit_result = nullptr;

      ArHitResultList_destroy(hit_result_list);
      hit_result_list = nullptr;
    }
  }
}

}  // namespace

MainApplication::MainApplication(AAssetManager* asset_manager)
    : asset_manager_(asset_manager),
      game_controller_(),
      game_box_renderer_(game_controller_.getGame().getDimensions()),
      debug_renderer_(),
      game_model_mat_(1.0f),
      game_scale_(1.0f)
{
  LOGI("OnCreate()");
}

MainApplication::~MainApplication() {
  if (ar_session_ != nullptr) {
    ArSession_destroy(ar_session_);
    ArFrame_destroy(ar_frame_);
  }
}

void MainApplication::OnPause() {
  LOGI("OnPause()");
  if (ar_session_ != nullptr) {
    ArSession_pause(ar_session_);
  }
}

void MainApplication::OnResume(void* env, void* context, void* activity) {
  LOGI("OnResume()");

  if (ar_session_ == nullptr) {
    ArInstallStatus install_status;
    // If install was not yet requested, that means that we are resuming the
    // activity first time because of explicit user interaction (such as
    // launching the application)
    bool user_requested_install = !install_requested_;

    // === ATTENTION!  ATTENTION!  ATTENTION! ===
    // This method can and will fail in user-facing situations.  Your
    // application must handle these cases at least somewhat gracefully.  See
    // HelloAR Java sample code for reasonable behavior.
    CHECK(ArCoreApk_requestInstall(env, activity, user_requested_install,
                                   &install_status) == AR_SUCCESS);

    switch (install_status) {
      case AR_INSTALL_STATUS_INSTALLED:
        break;
      case AR_INSTALL_STATUS_INSTALL_REQUESTED:
        install_requested_ = true;
        return;
    }

    // === ATTENTION!  ATTENTION!  ATTENTION! ===
    // This method can and will fail in user-facing situations.  Your
    // application must handle these cases at least somewhat gracefully.  See
    // HelloAR Java sample code for reasonable behavior.
    CHECK(ArSession_create(env, context, &ar_session_) == AR_SUCCESS);
    CHECK(ar_session_);

    ArConfig* ar_config = nullptr;
    ArConfig_create(ar_session_, &ar_config);
    CHECK(ar_config);

    const ArStatus status = ArSession_checkSupported(ar_session_, ar_config);
    CHECK(status == AR_SUCCESS);

    CHECK(ArSession_configure(ar_session_, ar_config) == AR_SUCCESS);

    ArConfig_destroy(ar_config);

    ArFrame_create(ar_session_, &ar_frame_);
    CHECK(ar_frame_);

    ArSession_setDisplayGeometry(ar_session_, display_rotation_, width_,
                                 height_);
  }

  const ArStatus status = ArSession_resume(ar_session_);
  CHECK(status == AR_SUCCESS);
}

void MainApplication::OnSurfaceCreated() {
  LOGI("OnSurfaceCreated()");

  background_renderer_.InitializeGlContent();
  game_box_renderer_.InitializeGlContent(asset_manager_);
  game_renderer_.InitializeGlContent(asset_manager_);
  debug_renderer_.InitializeGlContent(asset_manager_);
}

void MainApplication::OnDisplayGeometryChanged(int display_rotation,
                                                  int width, int height) {
  LOGI("OnSurfaceChanged(%d, %d)", width, height);
  glViewport(0, 0, width, height);
  display_rotation_ = display_rotation;
  width_ = width;
  height_ = height;
  if (ar_session_ != nullptr) {
    ArSession_setDisplayGeometry(ar_session_, display_rotation, width, height);
  }
}

void MainApplication::OnDrawFrame() {
  // Render the scene.
  glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (ar_session_ == nullptr) {
    game_controller_.onTrackingState(false);
    return;
  }

  ArSession_setCameraTextureName(ar_session_,
                                 background_renderer_.GetTextureId());

  // Update session to get current frame and render camera background.
  if (ArSession_update(ar_session_, ar_frame_) != AR_SUCCESS) {
    LOGE("MainApplication::OnDrawFrame ArSession_update error");
  }

  ArCamera* ar_camera;
  ArFrame_acquireCamera(ar_session_, ar_frame_, &ar_camera);

  glm::mat4 view_mat;
  glm::mat4 projection_mat;
  ArCamera_getViewMatrix(ar_session_, ar_camera, glm::value_ptr(view_mat));
  ArCamera_getProjectionMatrix(ar_session_, ar_camera,
                               /*near=*/0.1f, /*far=*/100.f,
                               glm::value_ptr(projection_mat));

  ArTrackingState camera_tracking_state;
  ArCamera_getTrackingState(ar_session_, ar_camera, &camera_tracking_state);
  ArCamera_release(ar_camera);

  background_renderer_.Draw(ar_session_, ar_frame_);

  // If the camera isn't tracking don't bother rendering other objects.
  if (camera_tracking_state != AR_TRACKING_STATE_TRACKING) {
    game_controller_.onTrackingState(false);
    return;
  }

  int64_t frame_timestamp;
  ArFrame_getTimestamp(ar_session_, ar_frame_, &frame_timestamp);
  bool changed = game_controller_.onFrame(frame_timestamp);
  if (changed) {
    LOGI("game state changed");
    game_renderer_.update(game_controller_.getGame(), game_scale_);
  }

  debug_renderer_.Draw(projection_mat, view_mat);

  // Get light estimation value.
  ArLightEstimate* ar_light_estimate;
  ArLightEstimateState ar_light_estimate_state;
  ArLightEstimate_create(ar_session_, &ar_light_estimate);

  ArFrame_getLightEstimate(ar_session_, ar_frame_, ar_light_estimate);
  ArLightEstimate_getState(ar_session_, ar_light_estimate,
                           &ar_light_estimate_state);

  // Set light intensity to default. Intensity value ranges from 0.0f to 1.0f.
  float light_intensity = 0.8f;
  if (ar_light_estimate_state == AR_LIGHT_ESTIMATE_STATE_VALID) {
    ArLightEstimate_getPixelIntensity(ar_session_, ar_light_estimate,
                                      &light_intensity);
  }

  ArLightEstimate_destroy(ar_light_estimate);
  ar_light_estimate = nullptr;

  for (const auto& obj_iter : tracked_obj_set_) {
    ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED;
    ArAnchor_getTrackingState(ar_session_, obj_iter, &tracking_state);
    if (tracking_state == AR_TRACKING_STATE_TRACKING) {
      util::GetTransformMatrixFromAnchor(ar_session_, obj_iter, &game_model_mat_);
    }
  }

  if (game_controller_.hasStarted()) {
    game_controller_.onTrackingState(true);
    game_box_renderer_.Draw(projection_mat, view_mat, game_model_mat_, game_scale_);
    game_renderer_.Draw(projection_mat, view_mat, game_model_mat_, light_intensity);
  }
  else {

    // Update and render planes.
    ArTrackableList* plane_list = nullptr;
    ArTrackableList_create(ar_session_, &plane_list);
    CHECK(plane_list != nullptr);

    ArTrackableType plane_tracked_type = AR_TRACKABLE_PLANE;
    ArSession_getAllTrackables(ar_session_, plane_tracked_type, plane_list);

    int32_t plane_list_size = 0;
    ArTrackableList_getSize(ar_session_, plane_list, &plane_list_size);

    bool tracking_any_planes = false;

    for (int i = 0; i < plane_list_size; ++i) {
      ArTrackable* ar_trackable = nullptr;
      ArTrackableList_acquireItem(ar_session_, plane_list, i, &ar_trackable);
      ArPlane* ar_plane = ArAsPlane(ar_trackable);
      ArTrackingState out_tracking_state;
      ArTrackable_getTrackingState(ar_session_, ar_trackable,
          &out_tracking_state);

      ArPlane* subsume_plane;
      ArPlane_acquireSubsumedBy(ar_session_, ar_plane, &subsume_plane);
      if (subsume_plane != nullptr) {
        ArTrackable_release(ArAsTrackable(subsume_plane));
        continue;
      }

      if (ArTrackingState::AR_TRACKING_STATE_TRACKING != out_tracking_state) {
        continue;
      }

      ArTrackingState plane_tracking_state;
      ArTrackable_getTrackingState(ar_session_, ArAsTrackable(ar_plane),
          &plane_tracking_state);
      if (plane_tracking_state == AR_TRACKING_STATE_TRACKING) {
        tracking_any_planes = true;
      }
    }

    ArTrackableList_destroy(plane_list);
    plane_list = nullptr;

    game_controller_.onTrackingState(tracking_any_planes);
  }

  if (game_controller_.getState() == GameController::State::WAITING_FOR_BOX) {
    ArSession* session = ar_session_;
    GameBoxRenderer& renderer = game_box_renderer_;
    glm::mat4x4& model_mat = game_model_mat_;
    float& scale = game_scale_;

    getHit(ar_session_, ar_frame_, 0.5f*width_, 0.5f*height_, false,
        [session, &renderer, projection_mat, view_mat, &model_mat, &scale](ArAnchor *anchor) {
          model_mat = glm::mat4(1.0f);
          util::GetTransformMatrixFromAnchor(session, anchor, &model_mat);

          const glm::vec3 delta = util::GetTranslation(view_mat * model_mat);
          scale = 0.05f * glm::length(delta);

          renderer.Draw(projection_mat, view_mat, model_mat, scale);
          return false;
        });
  }
}

void MainApplication::OnTouched(float x, float y) {
  std::vector<ArAnchor*>& anchors = tracked_obj_set_;
  GameController& controller = game_controller_;

  if (game_controller_.getState() == GameController::State::WAITING_FOR_BOX) {


    getHit(ar_session_, ar_frame_, /*x, y,*/ 0.5f*width_, 0.5f*height_, false,
        [&anchors, &controller](ArAnchor* anchor){
          if (anchors.size() >= kMaxAnchors) {
            ArAnchor_release(anchors[0]);
            anchors.erase(anchors.begin());
          }
          anchors.push_back(anchor);
          controller.onBoxFound();
          return true;
        });
  }
  else if (anchors.size() > 0) {
    glm::vec3 touch_origin, touch_dir;
    util::GetTouchRay(ar_session_, ar_frame_, x, y, width_, height_, touch_origin, touch_dir);
    LOGI("dir %f %f %f", touch_dir.x, touch_dir.y, touch_dir.z);
    LOGI("origin %f %f %f", touch_origin.x, touch_origin.y, touch_origin.z);

    const glm::vec3 game_origin = util::GetTranslation(game_model_mat_);
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
        game_controller_.moveXY(hit_x > 0 ? 1 : -1, 0);
      } else {
        game_controller_.moveXY(0, hit_y > 0 ? 1 : -1);
      }
    } // else no front-face ray intersection
  }
}

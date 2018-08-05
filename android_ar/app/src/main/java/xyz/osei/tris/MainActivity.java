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

package xyz.osei.tris;

import android.content.SharedPreferences;
import android.hardware.display.DisplayManager;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * This is a simple example that shows how to create an augmented reality (AR) application using the
 * ARCore C API.
 */
public class MainActivity extends AppCompatActivity
        implements GLSurfaceView.Renderer, DisplayManager.DisplayListener {

    private static final String TAG = MainActivity.class.getSimpleName();

    private GLSurfaceView mSurfaceView;

    private boolean mViewportChanged = false;
    private int mViewportWidth;
    private int mViewportHeight;

    // Opaque native pointer to the native application instance.
    private long mNativeApplication;
    private GestureDetector mGestureDetector;

    private TextView mScoreView;
    private TextView mHiscoreView;
    private TextView mStatusView;
    private Button mRestartButton;
    private String mStatusMessage = "";
    private int mScore = -1;
    private boolean mIsPaused = false;
    private boolean mGameRunning = false;
    private SharedPreferences mSharedPreferences;

    private Handler mUIRefreshHandler;
    private final Runnable mUIRefreshRunnable =
            new Runnable() {
                @Override
                public void run() {
                    // The runnable is executed on main UI thread.
                    try {
                        if (!mIsPaused) {
                            refreshUI();
                        }
                    } catch (Exception e) {
                        Log.e(TAG, e.getMessage());
                    }
                }
            };


    private void refreshUI() {
        final String text = getStatusMessage(mNativeApplication);
        final int score = JniInterface.getScore(mNativeApplication);
        final boolean nowRunning = !JniInterface.gameOver(mNativeApplication);

        if (nowRunning != mGameRunning) {
            if (nowRunning) {
                // on start
                mRestartButton.setVisibility(View.GONE);
                mScoreView.setVisibility(View.VISIBLE);
                mHiscoreView.setVisibility(View.GONE);
            } else {
                // on end
                mRestartButton.setVisibility(View.VISIBLE);
                mScoreView.setVisibility(View.GONE);
                mHiscoreView.setText(computeScoreText());
                mHiscoreView.setVisibility(View.VISIBLE);
            }
            mGameRunning = nowRunning;
            refreshSystemUiVisibility();
        }

        if (!text.equals(mStatusMessage)) {
            if (text.isEmpty()) {
                mStatusView.setVisibility(View.GONE);
            } else {
                mStatusView.setVisibility(View.VISIBLE);
                mStatusView.setText(text);
            }
            mStatusMessage = text;
        }

        if (score != mScore) {
            mScore = score;
            mScoreView.setText("" + mScore);
        }
    }

    private String computeScoreText() {
        final int finalScore = JniInterface.getScore(mNativeApplication);
        final int oldHiscore = getHiscore();
        if (finalScore > oldHiscore) {
            storeHiscore(finalScore);
            return "Score " + finalScore + ", new record!";
        }
        return  "Score: " + finalScore + " / " +
                "Record: " + oldHiscore;
    }

    private int getHiscore() {
        return mSharedPreferences.getInt("hiscore", 0);
    }

    private void storeHiscore(int newHiscore) {
        mSharedPreferences.edit().putInt("hiscore", newHiscore).apply();
    }

    private static String getStatusMessage(long nativeApplication) {
        final int arCoreErrorCode = JniInterface.getArCoreInstallError(nativeApplication);
        // "somewhat graceful" and "reasonable behavior"
        switch (arCoreErrorCode) {
            case 0:
                break; // AR_SUCCESS
            case -101: // AR_UNAVAILABLE_DEVICE_NOT_COMPATIBLE:
                return "Sorry, your phone does not support ARCore :,(";
            case -105: // AR_UNAVAILABLE_USER_DECLINED_INSTALLATION:
                return "You must install ARCore to play this AR game";
            case -103: // AR_UNAVAILABLE_APK_TOO_OLD:
                return "Please update ARCore";
            case -104: // AR_UNAVAILABLE_SDK_TOO_OLD:
                return "An ARCore update broke this app (AR_UNAVAILABLE_SDK_TOO_OLD), blame Google";
            case -9: // AR_ERROR_CAMERA_PERMISSION_NOT_GRANTED:
                return "No Camera permission, how did that happen?";
            default:
                return "ARCore is FUBAR (error " + arCoreErrorCode + ")";
        }

        if (JniInterface.isTracking(nativeApplication)) {
            if (JniInterface.gameStarted(nativeApplication)) {
                if (JniInterface.gameOver(nativeApplication)) {
                    return "Game over";
                } else {
                    return "";
                }
            } else {
                return "Tap to place & start";
            }
        } else {
            if (JniInterface.gameStarted(nativeApplication)) {
                return "Tracking lost :(";
            } else {
                return "Searching for surfaces...";
            }
        }
    }

    private void restartGame() {
        JniInterface.restartGame(mNativeApplication);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mSurfaceView = (GLSurfaceView) findViewById(R.id.surfaceview);
        mScoreView = (TextView) findViewById(R.id.scoreview);
        mHiscoreView = (TextView) findViewById(R.id.hiscoreview);
        mStatusView = (TextView) findViewById(R.id.statusview);
        mRestartButton = (Button) findViewById(R.id.restart_button);
        mSharedPreferences = getPreferences(MODE_PRIVATE);

        mRestartButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                restartGame();
            }
        });

        mRestartButton.setVisibility(View.GONE);
        mHiscoreView.setVisibility(View.GONE);

        // Set up tap listener.
        mGestureDetector =
                new GestureDetector(
                        this,
                        new GestureDetector.SimpleOnGestureListener() {
                            @Override
                            public boolean onSingleTapUp(final MotionEvent e) {
                                mSurfaceView.queueEvent(
                                        new Runnable() {
                                            @Override
                                            public void run() {
                                                JniInterface.onTap(mNativeApplication, e.getX(), e.getY());
                                            }
                                        });
                                return true;
                            }

                            @Override
                            public boolean onDoubleTapEvent(final MotionEvent e) {
                                if (e.getAction() != MotionEvent.ACTION_UP) {
                                    return false;  // Don't do anything for other actions
                                }

                                mSurfaceView.queueEvent(
                                        new Runnable() {
                                            @Override
                                            public void run() {
                                                JniInterface.onTap(mNativeApplication, e.getX(), e.getY());
                                            }
                                        });
                                return true;
                            }

                            @Override
                            public boolean onScroll(final MotionEvent e1,
                                                    final MotionEvent e2,
                                                    final float dx, final float dy) {
                                mSurfaceView.queueEvent(
                                        new Runnable() {
                                            @Override
                                            public void run() {
                                                JniInterface.onScroll(mNativeApplication,
                                                        e1.getX(), e1.getY(),
                                                        e2.getX(), e2.getY(),
                                                        dx, dy);
                                            }
                                        });
                                return true;
                            }

                            @Override
                            public boolean onDown(MotionEvent e) {
                                return true;
                            }
                        });

        mSurfaceView.setOnTouchListener(
                new View.OnTouchListener() {
                    @Override
                    public boolean onTouch(View v, final MotionEvent event) {

                        // deliver lower-level touch up events separately to handle scroll stops
                        if (event.getAction() == MotionEvent.ACTION_UP) {
                            mSurfaceView.queueEvent(
                                    new Runnable() {
                                        @Override
                                        public void run() {
                                            JniInterface.onTouchUp(mNativeApplication,
                                                    event.getX(), event.getY());
                                        }
                                    });
                        }
                        return mGestureDetector.onTouchEvent(event);
                    }

                });

        // Set up renderer.
        mSurfaceView.setPreserveEGLContextOnPause(true);
        mSurfaceView.setEGLContextClientVersion(2);
        mSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        mSurfaceView.setRenderer(this);
        mSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        JniInterface.assetManager = getAssets();
        mNativeApplication = JniInterface.createNativeApplication(getAssets());

        mUIRefreshHandler = new Handler();
    }

    @Override
    protected void onResume() {
        super.onResume();
        // ARCore requires camera permissions to operate. If we did not yet obtain runtime
        // permission on Android M and above, now is a good time to ask the user for it.
        if (!CameraPermissionHelper.hasCameraPermission(this)) {
            CameraPermissionHelper.requestCameraPermission(this);
            return;
        }

        JniInterface.onResume(mNativeApplication, getApplicationContext(), this);
        mSurfaceView.onResume();
        mIsPaused = false;

        mUIRefreshRunnable.run();

        // Listen to display changed events to detect 180° rotation, which does not cause a config
        // change or view resize.
        getSystemService(DisplayManager.class).registerDisplayListener(this, null);
    }

    @Override
    public void onPause() {
        super.onPause();
        mSurfaceView.onPause();
        JniInterface.onPause(mNativeApplication);
        mIsPaused = true;
        getSystemService(DisplayManager.class).unregisterDisplayListener(this);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        // Synchronized to avoid racing onDrawFrame.
        synchronized (this) {
            JniInterface.destroyNativeApplication(mNativeApplication);
            mNativeApplication = 0;
        }
    }

    private void refreshSystemUiVisibility() {
        final int flags;

        if (mGameRunning) {
            // Standard Android full-screen functionality.
            flags = View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
        } else {
            flags = View.SYSTEM_UI_FLAG_LAYOUT_STABLE;
        }

        getWindow().getDecorView().setSystemUiVisibility(flags);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            refreshSystemUiVisibility();
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        }
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        GLES20.glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        JniInterface.onGlSurfaceCreated(mNativeApplication);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        mViewportWidth = width;
        mViewportHeight = height;
        mViewportChanged = true;
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        final boolean changed;
        // Synchronized to avoid racing onDestroy.
        synchronized (this) {
            if (mNativeApplication == 0) {
                return;
            }
            if (mViewportChanged) {
                int displayRotation = getWindowManager().getDefaultDisplay().getRotation();
                JniInterface.onDisplayGeometryChanged(
                        mNativeApplication, displayRotation, mViewportWidth, mViewportHeight);
                mViewportChanged = false;
            }
            changed = JniInterface.onGlSurfaceDrawFrame(mNativeApplication);
        }
        if (changed) {
            mUIRefreshHandler.post(mUIRefreshRunnable);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        if (!CameraPermissionHelper.hasCameraPermission(this)) {
            Toast.makeText(this, "Camera permission is needed to run this application", Toast.LENGTH_LONG)
                    .show();
            if (!CameraPermissionHelper.shouldShowRequestPermissionRationale(this)) {
                // Permission denied with checking "Do not ask again".
                CameraPermissionHelper.launchPermissionSettings(this);
            }
            finish();
        }
    }

    // DisplayListener methods
    @Override
    public void onDisplayAdded(int displayId) {
    }

    @Override
    public void onDisplayRemoved(int displayId) {
    }

    @Override
    public void onDisplayChanged(int displayId) {
        mViewportChanged = true;
    }
}

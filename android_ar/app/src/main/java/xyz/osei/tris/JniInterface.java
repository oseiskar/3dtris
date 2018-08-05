package xyz.osei.tris;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;

/** JNI interface to native layer. */
public class JniInterface {
  static {
    System.loadLibrary("main_native");
  }

  static AssetManager assetManager;

  public static native long createNativeApplication(AssetManager assetManager);

  public static native void destroyNativeApplication(long nativeApplication);

  public static native void onPause(long nativeApplication);

  public static native void onResume(long nativeApplication, Context context, Activity activity);

  /** Allocate OpenGL resources for rendering. */
  public static native void onGlSurfaceCreated(long nativeApplication);

  /**
   * Called on the OpenGL thread before onGlSurfaceDrawFrame when the view port width, height, or
   * display rotation may have changed.
   */
  public static native void onDisplayGeometryChanged(
      long nativeApplication, int displayRotation, int width, int height);

  /** Main render loop, called on the OpenGL thread. */
  public static native boolean onGlSurfaceDrawFrame(long nativeApplication);

  /** OnTouch event, called on the OpenGL thread. */
  public static native void onTap(long nativeApplication, float x, float y);

  public static native void onTouchUp(long nativeApplication, float x, float y);

  public static native void onScroll(long nativeApplication,
                                     float x1, float y1,
                                     float x2, float y2,
                                     float dx, float dy);

  public static native boolean isTracking(long nativeApplication);
  public static native boolean gameStarted(long nativeApplication);
  public static native boolean gameOver(long nativeApplication);
  public static native void restartGame(long nativeApplication);
  public static native int getScore(long nativeApplication);
  public static native int getArCoreInstallError(long nativeApplication);
}

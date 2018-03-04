package xyz.osei.tris;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;

/** JNI interface to native layer. */
public class JniInterface {
  static {
    System.loadLibrary("hello_ar_native");
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
  public static native void onGlSurfaceDrawFrame(long nativeApplication);

  /** OnTouch event, called on the OpenGL thread. */
  public static native void onTouched(long nativeApplication, float x, float y);

  public static native boolean isTracking(long nativeApplication);
  public static native boolean gameStarted(long nativeApplication);
  public static native boolean gameOver(long nativeApplication);
}

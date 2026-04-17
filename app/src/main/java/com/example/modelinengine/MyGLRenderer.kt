package com.example.modelinengine

import android.content.Context
import android.opengl.GLSurfaceView
import android.view.MotionEvent
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class MyGLRenderer(private val context: Context) : GLSurfaceView.Renderer {

    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }

    external fun nativeInit()
    external fun nativeResize(width: Int, height: Int)
    external fun nativeRender()
    external fun nativeOnTouchEvent(action: Int, x: Float, y: Float)
    external fun nativeOnScale(scaleFactor: Float)
    external fun nativeSubdivide()
    external fun nativeExtrude(distance: Float)
    external fun nativeSetManipulatorMode(enabled: Boolean)
    external fun nativeDeleteSelected()
    external fun nativeSmooth()
    external fun nativeReset()
    external fun nativeGetSelectedVertex(): Int
    external fun nativeGetVertexPosition(vertexIdx: Int): FloatArray?
    external fun nativeMoveVertex(vertexIdx: Int, x: Float, y: Float, z: Float)

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        nativeInit()
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        nativeResize(width, height)
    }

    override fun onDrawFrame(gl: GL10?) {
        nativeRender()
    }

    fun handleTouchEvent(event: MotionEvent) {
        val action = event.actionMasked
        val x = event.x
        val y = event.y
        nativeOnTouchEvent(action, x, y)
    }

    fun handleScale(scaleFactor: Float) {
        nativeOnScale(scaleFactor)
    }
}

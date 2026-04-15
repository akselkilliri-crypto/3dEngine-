package com.example.modelinengine

import android.opengl.GLSurfaceView
import android.os.Bundle
import android.view.MotionEvent
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    private lateinit var glSurfaceView: GLSurfaceView
    private lateinit var renderer: MyGLRenderer

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        renderer = MyGLRenderer(this)
        glSurfaceView = GLSurfaceView(this).apply {
            setEGLContextClientVersion(3)
            setEGLConfigChooser(8, 8, 8, 8, 16, 4)
            setRenderer(renderer)
            renderMode = GLSurfaceView.RENDERMODE_WHEN_DIRTY // Рендерим только по запросу жестов
        }

        setContentView(glSurfaceView)
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        renderer.handleTouchEvent(event)
        glSurfaceView.requestRender() // Перерисовать после обработки жеста
        return true
    }

    override fun onPause() {
        super.onPause()
        glSurfaceView.onPause()
    }

    override fun onResume() {
        super.onResume()
        glSurfaceView.onResume()
    }
}

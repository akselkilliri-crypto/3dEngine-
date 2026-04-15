package com.example.modelinengine

import android.opengl.GLSurfaceView
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    private lateinit var glSurfaceView: GLSurfaceView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        glSurfaceView = GLSurfaceView(this).apply {
            setEGLContextClientVersion(3)
            // Включаем MSAA x4
            setEGLConfigChooser(8, 8, 8, 8, 16, 4)
            setRenderer(MyGLRenderer(this@MainActivity))
            // Рендерим только при необходимости (для этапа 1 — непрерывно, потом сменим)
            renderMode = GLSurfaceView.RENDERMODE_CONTINUOUSLY
        }

        setContentView(glSurfaceView)
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

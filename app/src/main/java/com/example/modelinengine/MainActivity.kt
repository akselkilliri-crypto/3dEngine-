package com.example.modelinengine

import android.graphics.Color
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.floatingactionbutton.FloatingActionButton
import android.text.InputType
import android.widget.EditText

class MainActivity : AppCompatActivity() {

    private lateinit var glSurfaceView: GLSurfaceView
    private lateinit var renderer: MyGLRenderer
    private lateinit var scaleGestureDetector: ScaleGestureDetector

    // Режим манипулятора (перемещение вместо вращения)
    private var isManipulatorMode = false
    private lateinit var fabManipulator: FloatingActionButton

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        renderer = MyGLRenderer(this)
        glSurfaceView = findViewById<GLSurfaceView>(R.id.glSurfaceView).apply {
            setEGLContextClientVersion(3)
            setEGLConfigChooser(8, 8, 8, 8, 16, 4)
            setRenderer(renderer)
            renderMode = GLSurfaceView.RENDERMODE_WHEN_DIRTY
        }

        scaleGestureDetector = ScaleGestureDetector(this, ScaleListener())

        val fabSubdivide = findViewById<FloatingActionButton>(R.id.fabSubdivide)
        fabSubdivide.setOnClickListener {
            renderer.nativeSubdivide()
            glSurfaceView.requestRender()
        }

        val fabExtrude = findViewById<FloatingActionButton>(R.id.fabExtrude)
        fabExtrude.setOnClickListener {
            showExtrudeDialog()
        }

        fabManipulator = findViewById(R.id.fabManipulator)
        fabManipulator.setOnClickListener {
            isManipulatorMode = !isManipulatorMode
            // Визуальное отображение активного режима
            if (isManipulatorMode) {
                fabManipulator.setBackgroundColor(Color.GREEN)
                Toast.makeText(this, "Режим перемещения ВКЛ", Toast.LENGTH_SHORT).show()
            } else {
                fabManipulator.setBackgroundColor(Color.parseColor("#FF6200EE")) // стандартный цвет
                Toast.makeText(this, "Режим вращения ВКЛ", Toast.LENGTH_SHORT).show()
            }
            renderer.nativeSetManipulatorMode(isManipulatorMode)
        }
    }

    private fun showExtrudeDialog() {
        val input = EditText(this)
        input.inputType = InputType.TYPE_CLASS_NUMBER or InputType.TYPE_NUMBER_FLAG_DECIMAL
        input.hint = "Расстояние (мм)"

        AlertDialog.Builder(this)
            .setTitle("Вытянуть полигон")
            .setMessage("Введите длину вытягивания в миллиметрах:")
            .setView(input)
            .setPositiveButton("Применить") { _, _ ->
                val text = input.text.toString()
                if (text.isNotEmpty()) {
                    try {
                        val distance = text.toFloat()
                        renderer.nativeExtrude(distance)
                        glSurfaceView.requestRender()
                    } catch (e: NumberFormatException) {
                        Toast.makeText(this, "Некорректное число", Toast.LENGTH_SHORT).show()
                    }
                }
            }
            .setNegativeButton("Отмена", null)
            .show()
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        scaleGestureDetector.onTouchEvent(event)

        if (!scaleGestureDetector.isInProgress) {
            renderer.handleTouchEvent(event)
        }

        glSurfaceView.requestRender()
        return true
    }

    private inner class ScaleListener : ScaleGestureDetector.SimpleOnScaleGestureListener() {
        override fun onScale(detector: ScaleGestureDetector): Boolean {
            renderer.handleScale(detector.scaleFactor)
            return true
        }
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

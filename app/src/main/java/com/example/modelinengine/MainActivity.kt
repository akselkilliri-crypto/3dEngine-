package com.example.modelinengine

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

    // Для панорамирования
    private var isPanning = false
    private var lastPanX = 0f
    private var lastPanY = 0f

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
        // Сначала передаём в ScaleGestureDetector (щипок)
        scaleGestureDetector.onTouchEvent(event)

        val pointerCount = event.pointerCount

        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                // Один палец — начинаем вращение (обрабатывается в renderer.handleTouchEvent)
                renderer.handleTouchEvent(event)
            }
            MotionEvent.ACTION_POINTER_DOWN -> {
                // Появился второй палец — начинаем панорамирование
                if (pointerCount == 2) {
                    isPanning = true
                    lastPanX = (event.getX(0) + event.getX(1)) / 2f
                    lastPanY = (event.getY(0) + event.getY(1)) / 2f
                }
            }
            MotionEvent.ACTION_MOVE -> {
                if (isPanning && pointerCount == 2) {
                    // Панорамирование двумя пальцами
                    val currentX = (event.getX(0) + event.getX(1)) / 2f
                    val currentY = (event.getY(0) + event.getY(1)) / 2f
                    val dx = currentX - lastPanX
                    val dy = currentY - lastPanY
                    renderer.nativePan(dx, dy, glSurfaceView.width, glSurfaceView.height)
                    lastPanX = currentX
                    lastPanY = currentY
                    glSurfaceView.requestRender()
                } else if (!scaleGestureDetector.isInProgress) {
                    // Один палец — вращение
                    renderer.handleTouchEvent(event)
                }
            }
            MotionEvent.ACTION_POINTER_UP -> {
                if (pointerCount == 2) {
                    // Один из пальцев поднят, выключаем панорамирование
                    isPanning = false
                }
            }
            MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                isPanning = false
                if (!scaleGestureDetector.isInProgress) {
                    renderer.handleTouchEvent(event)
                }
            }
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

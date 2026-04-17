package com.example.modelinengine

import android.graphics.Color
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.text.InputType
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.SeekBar
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.floatingactionbutton.FloatingActionButton

class MainActivity : AppCompatActivity() {

    private lateinit var glSurfaceView: GLSurfaceView
    private lateinit var renderer: MyGLRenderer
    private lateinit var scaleGestureDetector: ScaleGestureDetector

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

        findViewById<FloatingActionButton>(R.id.fabSubdivide).setOnClickListener {
            renderer.nativeSubdivide()
            glSurfaceView.requestRender()
        }

        findViewById<FloatingActionButton>(R.id.fabExtrude).setOnClickListener {
            showExtrudeDialog()
        }

        fabManipulator = findViewById(R.id.fabManipulator)
        fabManipulator.setOnClickListener {
            isManipulatorMode = !isManipulatorMode
            fabManipulator.setBackgroundColor(if (isManipulatorMode) Color.GREEN else Color.parseColor("#FF6200EE"))
            Toast.makeText(this, if (isManipulatorMode) "Режим перемещения ВКЛ" else "Режим вращения ВКЛ", Toast.LENGTH_SHORT).show()
            renderer.nativeSetManipulatorMode(isManipulatorMode)
        }

        findViewById<FloatingActionButton>(R.id.fabDelete).setOnClickListener {
            renderer.nativeDeleteSelected()
            glSurfaceView.requestRender()
        }

        findViewById<FloatingActionButton>(R.id.fabSmooth).setOnClickListener {
            renderer.nativeSmooth()
            glSurfaceView.requestRender()
            Toast.makeText(this, "Сглаживание применено", Toast.LENGTH_SHORT).show()
        }

        findViewById<FloatingActionButton>(R.id.fabReset).setOnClickListener {
            AlertDialog.Builder(this)
                .setTitle("Сброс модели")
                .setMessage("Вернуть исходный куб? Все изменения будут потеряны.")
                .setPositiveButton("Да") { _, _ ->
                    renderer.nativeReset()
                    glSurfaceView.requestRender()
                }
                .setNegativeButton("Отмена", null)
                .show()
        }

        // Добавим жест долгого нажатия для вызова перемещения вершины (если выделен полигон, берём первую вершину)
        glSurfaceView.setOnLongClickListener {
            showVertexMoveDialog()
            true
        }
    }

    private fun showExtrudeDialog() {
        val input = EditText(this)
        input.inputType = InputType.TYPE_CLASS_NUMBER or InputType.TYPE_NUMBER_FLAG_SIGNED or InputType.TYPE_NUMBER_FLAG_DECIMAL
        input.hint = "Расстояние (мм, можно отрицательное)"

        AlertDialog.Builder(this)
            .setTitle("Вытянуть/вдавить полигон")
            .setMessage("Введите длину вытягивания:")
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

    private fun showVertexMoveDialog() {
        // Проверяем, есть ли выделенная вершина (через JNI получаем индекс выделенной вершины)
        val selectedVertex = renderer.nativeGetSelectedVertex()
        if (selectedVertex < 0) {
            Toast.makeText(this, "Нет выделенной вершины. Выделите полигон и нажмите долго.", Toast.LENGTH_SHORT).show()
            return
        }

        val layout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(40, 20, 40, 20)
        }

        val labelX = TextView(this).apply { text = "X: 0.0" }
        val seekX = SeekBar(this).apply { max = 200; setProgress(100) }
        val labelY = TextView(this).apply { text = "Y: 0.0" }
        val seekY = SeekBar(this).apply { max = 200; setProgress(100) }
        val labelZ = TextView(this).apply { text = "Z: 0.0" }
        val seekZ = SeekBar(this).apply { max = 200; setProgress(100) }

        layout.addView(labelX)
        layout.addView(seekX)
        layout.addView(labelY)
        layout.addView(seekY)
        layout.addView(labelZ)
        layout.addView(seekZ)

        // Получаем текущую позицию вершины
        val pos = renderer.nativeGetVertexPosition(selectedVertex)
        var currentPos = pos ?: floatArrayOf(0f, 0f, 0f)
        val originalPos = currentPos.clone()

        fun updateLabels() {
            labelX.text = "X: %.2f".format(currentPos[0])
            labelY.text = "Y: %.2f".format(currentPos[1])
            labelZ.text = "Z: %.2f".format(currentPos[2])
        }
        updateLabels()

        val listener = object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                if (!fromUser) return
                val offset = (progress - 100) * 0.02f // диапазон смещения -2..2 мм
                when (seekBar?.id) {
                    seekX.id -> currentPos[0] = originalPos[0] + offset
                    seekY.id -> currentPos[1] = originalPos[1] + offset
                    seekZ.id -> currentPos[2] = originalPos[2] + offset
                }
                renderer.nativeMoveVertex(selectedVertex, currentPos[0], currentPos[1], currentPos[2])
                glSurfaceView.requestRender()
                updateLabels()
            }
            override fun onStartTrackingTouch(p0: SeekBar?) {}
            override fun onStopTrackingTouch(p0: SeekBar?) {}
        }

        seekX.id = 1; seekY.id = 2; seekZ.id = 3
        seekX.setOnSeekBarChangeListener(listener)
        seekY.setOnSeekBarChangeListener(listener)
        seekZ.setOnSeekBarChangeListener(listener)

        AlertDialog.Builder(this)
            .setTitle("Перемещение вершины")
            .setView(layout)
            .setPositiveButton("OK", null)
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

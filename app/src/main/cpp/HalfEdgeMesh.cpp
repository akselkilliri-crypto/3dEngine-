// ========== ИСПРАВЛЕННЫЙ extrudeSelected ==========
void HalfEdgeMesh::extrudeSelected(float distance) {
    if (distance == 0.0f) return;

    std::vector<Face> newFaces;
    std::vector<int> selectedFaceIndices;

    // Сохраняем индексы выделенных граней
    for (size_t i = 0; i < faces.size(); ++i) {
        if (faces[i].selected) {
            selectedFaceIndices.push_back(i);
        } else {
            newFaces.push_back(faces[i]);
        }
    }

    if (selectedFaceIndices.empty()) return;

    for (int faceIdx : selectedFaceIndices) {
        const Face& face = faces[faceIdx];
        int n = face.vertexIndices.size();
        if (n < 3) continue;

        Vec3 normal = face.normal;
        float gray[4] = {0.7f, 0.7f, 0.7f, 1.0f};

        // 1. Создаём новые вершины, смещённые по нормали
        std::vector<int> newVertices;
        for (int i = 0; i < n; ++i) {
            int oldIdx = face.vertexIndices[i];
            Vec3 oldPos = vertices[oldIdx].position;
            Vec3 newPos = oldPos + normal * distance;
            VertexHE newVert;
            newVert.position = newPos;
            newVert.normal = normal;
            for (int c = 0; c < 4; ++c) newVert.color[c] = gray[c];
            int newIdx = addVertex(newVert);
            newVertices.push_back(newIdx);
        }

        // 2. Исходная грань остаётся, но с обратным порядком вершин,
        //    чтобы она была видна с обратной стороны (правильная ориентация).
        Face backFace = face;
        std::reverse(backFace.vertexIndices.begin(), backFace.vertexIndices.end());
        backFace.selected = false;
        newFaces.push_back(backFace);

        // 3. Новая (передняя) грань из смещённых вершин
        Face frontFace;
        frontFace.vertexIndices = newVertices;
        frontFace.normal = normal;
        frontFace.selected = false;
        newFaces.push_back(frontFace);

        // 4. Боковые грани (четырёхугольники)
        for (int i = 0; i < n; ++i) {
            int old0 = face.vertexIndices[i];
            int old1 = face.vertexIndices[(i+1)%n];
            int new0 = newVertices[i];
            int new1 = newVertices[(i+1)%n];

            Face side;
            side.vertexIndices = {old0, old1, new1, new0};
            // Вычисляем нормаль боковой грани
            Vec3 e1 = vertices[old1].position - vertices[old0].position;
            Vec3 e2 = vertices[new0].position - vertices[old0].position;
            side.normal = e1.cross(e2).normalized();
            side.selected = false;
            newFaces.push_back(side);
        }
    }

    faces = std::move(newFaces);

    // Перестраиваем faceIndices (триангуляция веером)
    faceIndices.clear();
    for (size_t i = 0; i < faces.size(); ++i) {
        const auto& f = faces[i];
        int n = f.vertexIndices.size();
        if (n >= 3) {
            for (int j = 1; j < n - 1; ++j) {
                faceIndices.push_back(f.vertexIndices[0]);
                faceIndices.push_back(f.vertexIndices[j]);
                faceIndices.push_back(f.vertexIndices[j+1]);
            }
        }
    }

    dirty = true;
    edgesDirty = true;
    updateSelectedBuffers();
    LOGI("Extrude complete. Total faces: %zu", faces.size());
}

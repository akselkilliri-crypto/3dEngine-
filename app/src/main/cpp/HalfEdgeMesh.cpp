// ========== НОВЫЕ МЕТОДЫ ==========

void HalfEdgeMesh::deleteSelected() {
    std::vector<Face> newFaces;
    for (size_t i = 0; i < faces.size(); ++i) {
        if (!faces[i].selected) {
            newFaces.push_back(faces[i]);
        }
    }
    faces = std::move(newFaces);

    // Перестраиваем faceIndices
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
    LOGI("Deleted selected faces. Remaining: %zu", faces.size());
}

void HalfEdgeMesh::smoothMesh() {
    // Простое усреднение позиций вершин (Laplacian smoothing) для всей сетки
    if (vertices.empty()) return;

    std::vector<Vec3> newPositions(vertices.size(), Vec3(0,0,0));
    std::vector<int> neighborCount(vertices.size(), 0);

    // Для каждой грани добавляем соседей
    for (const auto& face : faces) {
        int n = face.vertexIndices.size();
        for (int i = 0; i < n; ++i) {
            int idx = face.vertexIndices[i];
            int next = face.vertexIndices[(i+1)%n];
            int prev = face.vertexIndices[(i-1+n)%n];
            newPositions[idx] = newPositions[idx] + vertices[next].position + vertices[prev].position;
            neighborCount[idx] += 2;
        }
    }

    for (size_t i = 0; i < vertices.size(); ++i) {
        if (neighborCount[i] > 0) {
            vertices[i].position = (vertices[i].position + newPositions[i] * (1.0f / neighborCount[i])) * 0.5f;
        }
    }

    dirty = true;
    edgesDirty = true;
    LOGI("Mesh smoothed.");
}

void HalfEdgeMesh::resetToCube() {
    createCube();
}

int HalfEdgeMesh::getSelectedVertex() const {
    // Возвращаем индекс первой вершины первого выделенного полигона
    for (const auto& face : faces) {
        if (face.selected && !face.vertexIndices.empty()) {
            return face.vertexIndices[0];
        }
    }
    return -1;
}

bool HalfEdgeMesh::getVertexPosition(int idx, Vec3& outPos) const {
    if (idx >= 0 && idx < (int)vertices.size()) {
        outPos = vertices[idx].position;
        return true;
    }
    return false;
}

void HalfEdgeMesh::moveVertex(int idx, const Vec3& newPos) {
    if (idx < 0 || idx >= (int)vertices.size()) return;
    vertices[idx].position = newPos;
    dirty = true;
    edgesDirty = true;
    // Пересчёт нормалей для граней, содержащих эту вершину
    for (auto& face : faces) {
        bool contains = false;
        for (int vIdx : face.vertexIndices) if (vIdx == idx) { contains = true; break; }
        if (contains && face.vertexIndices.size() >= 3) {
            Vec3 v0 = vertices[face.vertexIndices[0]].position;
            Vec3 v1 = vertices[face.vertexIndices[1]].position;
            Vec3 v2 = vertices[face.vertexIndices[2]].position;
            face.normal = (v1 - v0).cross(v2 - v0).normalized();
        }
    }
}

#include "stable.h"
#include "geometrygenerator.h"

void GeometryGenerator::CreateBox(float width, float height, float depth, MeshData& meshData)
{
    Vertex v[24];

    float w2 = 0.5f*width;
    float h2 = 0.5f*height;
    float d2 = 0.5f*depth;

    // Fill in the front face vertex data.
    v[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    v[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    v[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    v[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    // Fill in the back face vertex data.
    v[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    v[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    v[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    v[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // Fill in the top face vertex data.
    v[8] = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    v[9] = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    v[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    v[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    // Fill in the bottom face vertex data.
    v[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    v[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    v[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    v[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // Fill in the left face vertex data.
    v[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    v[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
    v[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

    // Fill in the right face vertex data.
    v[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    v[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    v[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
    v[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

    meshData.vertices.assign(&v[0], &v[24]);

    UINT i[36];

    // Fill in the front face index data
    i[0] = 0; i[1] = 1; i[2] = 2;
    i[3] = 0; i[4] = 2; i[5] = 3;

    // Fill in the back face index data
    i[6] = 4; i[7] = 5; i[8] = 6;
    i[9] = 4; i[10] = 6; i[11] = 7;

    // Fill in the top face index data
    i[12] = 8; i[13] = 9; i[14] = 10;
    i[15] = 8; i[16] = 10; i[17] = 11;

    // Fill in the bottom face index data
    i[18] = 12; i[19] = 13; i[20] = 14;
    i[21] = 12; i[22] = 14; i[23] = 15;

    // Fill in the left face index data
    i[24] = 16; i[25] = 17; i[26] = 18;
    i[27] = 16; i[28] = 18; i[29] = 19;

    // Fill in the right face index data
    i[30] = 20; i[31] = 21; i[32] = 22;
    i[33] = 20; i[34] = 22; i[35] = 23;

    meshData.indices.assign(&i[0], &i[36]);
}

void GeometryGenerator::CreateGrid(float width, float depth, UINT m, UINT n, MeshData& meshData)
{
    UINT vertexCount = m*n;
    UINT faceCount = (m - 1)*(n - 1) * 2;


    float halfWidth = 0.5f*width;
    float halfDepth = 0.5f*depth;

    float dx = width / (n - 1);
    float dz = depth / (m - 1);

    float du = 1.0f / (n - 1);
    float dv = 1.0f / (m - 1);

    meshData.vertices.resize(vertexCount);
    for (UINT i = 0; i < m; ++i)
    {
        float z = halfDepth - i*dz;
        for (UINT j = 0; j < n; ++j)
        {
            float x = -halfWidth + j*dx;

            meshData.vertices[i*n + j].position = XMFLOAT3(x, 0.0f, z);
            meshData.vertices[i*n + j].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
            meshData.vertices[i*n + j].tangentU = XMFLOAT3(1.0f, 0.0f, 0.0f);

            // Stretch texture over grid.
            meshData.vertices[i*n + j].tex.x = j*du;
            meshData.vertices[i*n + j].tex.y = i*dv;
        }
    }

    meshData.indices.resize(faceCount * 3);

    UINT index = 0;
    for (UINT i = 0; i < m - 1; ++i)
    {
        for (UINT j = 0; j < n - 1; ++j)
        {
            meshData.indices[index] = i*n + j;
            meshData.indices[index + 1] = i*n + j + 1;
            meshData.indices[index + 2] = (i + 1)*n + j;

            meshData.indices[index + 3] = (i + 1)*n + j;
            meshData.indices[index + 4] = i*n + j + 1;
            meshData.indices[index + 5] = (i + 1)*n + j + 1;

            index += 6; // next quad
        }
    }
}

void AddCylinderTop(float radius, float height, UINT sliceCount, GeometryGenerator::MeshData& meshData)
{
    size_t startIndex = meshData.vertices.size();
    float dTheta = 2.0f*XM_PI / sliceCount;

    for (size_t j = 0; j <= sliceCount; j++)
    {
        float theta = j * dTheta;
        GeometryGenerator::Vertex vertex;
        vertex.position = XMFLOAT3(radius * cosf(theta), height * 0.5f, radius* sinf(theta));
        vertex.tex.x = vertex.position.x / height + 0.5f;
        vertex.tex.y = vertex.position.z / height + 0.5f;
        vertex.normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

        meshData.vertices.push_back(vertex);
    }

    GeometryGenerator::Vertex center;
    center.position = XMFLOAT3(0.0f, height * 0.5f, 0.0f);
    center.tex = XMFLOAT2(0.5f, 0.5f);
    center.normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

    meshData.vertices.push_back(center);
    size_t centerIndex = meshData.vertices.size() - 1;

    for (size_t i = 0; i < sliceCount; ++i)
    {
        meshData.indices.push_back(centerIndex);
        meshData.indices.push_back(startIndex + i + 1);
        meshData.indices.push_back(startIndex + i);
    }
}

void AddCylinderBottom(float radius, float height, UINT sliceCount, GeometryGenerator::MeshData& meshData)
{
    size_t startIndex = meshData.vertices.size();
    float dTheta = 2.0f*XM_PI / sliceCount;

    for (size_t j = 0; j <= sliceCount; j++)
    {
        float theta = j * dTheta;
        GeometryGenerator::Vertex vertex;
        vertex.position = XMFLOAT3(radius * cosf(theta), -height * 0.5f, radius* sinf(theta));
        vertex.tex.x = vertex.position.x / height + 0.5f;
        vertex.tex.y = vertex.position.z / height + 0.5f;
        vertex.normal = XMFLOAT3(0.0f, -1.0f, 0.0f);

        meshData.vertices.push_back(vertex);
    }

    GeometryGenerator::Vertex center;
    center.position = XMFLOAT3(0.0f, -height * 0.5f, 0.0f);
    center.tex = XMFLOAT2(0.5f, 0.5f);
    center.normal = XMFLOAT3(0.0f, -1.0f, 0.0f);
    meshData.vertices.push_back(center);
    size_t centerIndex = meshData.vertices.size() - 1;

    for (size_t i = 0; i < sliceCount; ++i)
    {
        meshData.indices.push_back(centerIndex);
        meshData.indices.push_back(startIndex + i );
        meshData.indices.push_back(startIndex + i + 1);
    }
}

void GeometryGenerator::CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData)
{
    meshData.vertices.clear();
    meshData.indices.clear();

    float radiusStep = (bottomRadius - topRadius) / stackCount;
    float stackHeight = height / stackCount;

    UINT rows = stackCount + 1;

    UINT vertexCountPerRow = sliceCount + 1;

    for (size_t i = 0; i < rows; i++)
    {
        float y = 0.5f * height - i * stackHeight;
        float radius = topRadius + i * radiusStep;

        float dTheta = 2.0f*XM_PI / sliceCount;

        for (size_t j = 0; j < vertexCountPerRow; j++)
        {
            float theta = j * dTheta;
            Vertex vertex;
            vertex.position = XMFLOAT3(radius * cosf(theta), y, radius* sinf(theta));
            vertex.tex.x = (float)j / sliceCount;
            vertex.tex.y = (float)i / stackCount;

            vertex.tangentU = XMFLOAT3(-sinf(theta), 0.0f, cosf(theta));

            XMVECTOR N = XMVectorSet(cos(theta), (bottomRadius - topRadius) / height, sin(theta), 0.f);
            XMStoreFloat3(&vertex.normal, XMVector3Normalize(N));

            meshData.vertices.push_back(vertex);
        }
    }

    for (size_t i = 0; i < stackCount; ++i)
    {
        for (size_t j = 0; j < sliceCount; ++j)
        {
            meshData.indices.push_back(i * vertexCountPerRow + j);
            meshData.indices.push_back(i * vertexCountPerRow + j + 1);
            meshData.indices.push_back((i + 1) * vertexCountPerRow + j + 1);

            meshData.indices.push_back(i * vertexCountPerRow + j);
            meshData.indices.push_back((i + 1) * vertexCountPerRow + j + 1);
            meshData.indices.push_back((i + 1) * vertexCountPerRow + j);
        }
    }

    AddCylinderTop(topRadius, height, sliceCount, meshData);
    AddCylinderBottom(bottomRadius, height, sliceCount, meshData);
}

void GeometryGenerator::createSphere(float radius, UINT sliceCount, UINT stackCount, MeshData& meshData)
{
    meshData.vertices.clear();
    meshData.indices.clear();

    UINT vertexCountPerRow = sliceCount + 1;

    Vertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    meshData.vertices.push_back(topVertex);

    float dTheta = XM_2PI / sliceCount;

    for (size_t i = 1; i <= stackCount -1 ; ++i)
    {
        float phi = XM_PI * i / stackCount;
        float y = radius * cosf(phi);
        float rowRadius = radius * sinf(phi);

        for (size_t j = 0; j < vertexCountPerRow; ++j)
        {
            float theta = j *dTheta;

            Vertex vertex;
            vertex.position = XMFLOAT3(rowRadius * cosf(theta), y, rowRadius* sinf(theta));

            XMVECTOR p = XMLoadFloat3(&vertex.position);
            XMStoreFloat3(&vertex.normal, XMVector3Normalize(p));

            vertex.tex.x = (float)j / sliceCount;
            vertex.tex.y = phi / XM_PI;

            meshData.vertices.push_back(vertex);
        }
    }

    meshData.vertices.push_back(bottomVertex);

    for (UINT i = 1; i <= sliceCount; ++i)
    {
        meshData.indices.push_back(0);
        meshData.indices.push_back(i + 1);
        meshData.indices.push_back(i);
    }

    UINT baseIndex = 1;

    for (size_t i = 0; i < stackCount - 2; ++i)
    {
        for (size_t j = 0; j < sliceCount; ++j)
        {
            meshData.indices.push_back(baseIndex + i * vertexCountPerRow + j);
            meshData.indices.push_back(baseIndex + i * vertexCountPerRow + j + 1);
            meshData.indices.push_back(baseIndex + (i + 1) * vertexCountPerRow + j + 1);

            meshData.indices.push_back(baseIndex + i * vertexCountPerRow + j);
            meshData.indices.push_back(baseIndex + (i + 1) * vertexCountPerRow + j + 1);
            meshData.indices.push_back(baseIndex + (i + 1) * vertexCountPerRow + j);
        }
    }

    UINT southPoleIndex = (UINT)meshData.vertices.size() - 1;
    baseIndex = southPoleIndex - sliceCount - 1;
    for (UINT i = 1; i <= sliceCount; ++i)
    {
        meshData.indices.push_back(southPoleIndex);
        meshData.indices.push_back(baseIndex + i);
        meshData.indices.push_back(baseIndex + i + 1);
    }
}


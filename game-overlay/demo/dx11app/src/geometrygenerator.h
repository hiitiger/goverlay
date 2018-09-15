#pragma once


class GeometryGenerator
{
public:
    struct Vertex
    {
        Vertex(){}
        Vertex(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT3& t, const XMFLOAT2& uv)
            : position(p), normal(n), tangentU(t), tex(uv){}
        Vertex(
            float px, float py, float pz,
            float nx, float ny, float nz,
            float tx, float ty, float tz,
            float u, float v)
            : position(px, py, pz), normal(nx, ny, nz),
            tangentU(tx, ty, tz), tex(u, v){}

        XMFLOAT3 position;
        XMFLOAT3 normal;
        XMFLOAT3 tangentU;
        XMFLOAT2 tex;
    };

    struct MeshData
    {
        std::vector<Vertex> vertices;
        std::vector<UINT> indices;
    };

    void CreateBox(float width, float height, float depth, MeshData& meshData);

    ///<summary>
    /// Creates an mxn grid in the xz-plane with m rows and n columns, centered
    /// at the origin with the specified width and depth.
    ///</summary>
    void CreateGrid(float width, float depth, UINT m, UINT n, MeshData& meshData);

    ///<summary>
    /// Creates a cylinder parallel to the y-axis, and centered about the origin.  
    /// The bottom and top radius can vary to form various cone shapes rather than true
    // cylinders.  The slices and stacks parameters control the degree of tessellation.
    ///</summary>
    void CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);


    void createSphere(float radius, UINT sliceCount, UINT stackCount, MeshData& meshData);

private:

};

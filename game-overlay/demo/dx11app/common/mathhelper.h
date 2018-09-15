#pragma once

class MathHelper
{
public:
    // Returns random float in [0, 1).
    static float randfloat()
    {
        return (float)(rand()) / (float)RAND_MAX;
    }

    // Returns random float in [a, b).
    static float randfloat(float a, float b)
    {
        return a + randfloat()*(b - a);
    }

    static XMMATRIX InverseTranspose(CXMMATRIX M)
    {
        // Inverse-transpose is just applied to normals.  So zero out 
        // translation row so that it doesn't get into our inverse-transpose
        // calculation--we don't want the inverse-transpose of the translation.
        XMMATRIX A = M;
        A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

        XMVECTOR det = XMMatrixDeterminant(A);
        return XMMatrixTranspose(XMMatrixInverse(&det, A));
    }
};
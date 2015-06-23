#include "renderer/OpenGL.hpp"

namespace Renderer
{
    // create perspective projection matrix
    void MakePerspective(Math::Matrix4f &matrix, float fov, float scrRatio, float nearPlane, float farPlane)
    {
        matrix.Zero();

        float tanFov = tanf(0.5f * fov); // fov is in radians!

        matrix[0]  = 1.f / (scrRatio * tanFov);
        matrix[5]  = 1.f / tanFov;
        matrix[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
        matrix[11] = -1.f;
        matrix[14] = -2.f * farPlane * nearPlane / (farPlane - nearPlane);
    }


    // create orthographic projection matrix
    void MakeOrthogonal(Math::Matrix4f &matrix, float left, float right, float bottom, float top, float nearPlane, float farPlane)
    {
        matrix.Identity();

        matrix[0]  = 2.f / (right - left);
        matrix[5]  = 2.f / (top - bottom);
        matrix[10] = -2.f / (farPlane - nearPlane);
        matrix[12] = -(right + left) / (right - left);
        matrix[13] = -(top + bottom) / (top - bottom);
        matrix[14] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    }

    // create view matrix
    void MakeView(Math::Matrix4f &matrix, const Math::Vector3f &eye, const Math::Vector3f &target, const Math::Vector3f &up)
    {
        Math::Vector3f z = target;
        z.Normalize();
        Math::Vector3f x = z.CrossProduct(up);
        x.Normalize();
        Math::Vector3f y = x.CrossProduct(z);

        matrix.Identity();

        matrix[0] = x.m_x;
        matrix[4] = x.m_y;
        matrix[8] = x.m_z;

        matrix[1] = y.m_x;
        matrix[5] = y.m_y;
        matrix[9] = y.m_z;

        matrix[2]  = -z.m_x;
        matrix[6]  = -z.m_y;
        matrix[10] = -z.m_z;

        matrix[12] = -x.DotProduct(eye);
        matrix[13] = -y.DotProduct(eye);
        matrix[14] = z.DotProduct(eye);
    }
}

#include "renderer/OVRTrackerChaperone.hpp"
#include "renderer/ShaderManager.hpp"

#define ChaperonePlanePoints(topEdge, bottomEdge) \
        trackerPose.x, trackerPose.y, trackerPose.z, \
        trackerPose.x +    topEdge.x, trackerPose.y + topEdge.y, trackerPose.z + topEdge.z, \
        trackerPose.x +    topEdge.x / 16, trackerPose.y +    topEdge.y / 16, trackerPose.z + topEdge.z / 16, \
        trackerPose.x + bottomEdge.x / 16, trackerPose.y + bottomEdge.y / 16, trackerPose.z + bottomEdge.z / 16, \
        trackerPose.x +    topEdge.x / 8, trackerPose.y  +    topEdge.y / 8,  trackerPose.z + topEdge.z / 8, \
        trackerPose.x + bottomEdge.x / 8, trackerPose.y  + bottomEdge.y / 8,  trackerPose.z + bottomEdge.z / 8, \
        trackerPose.x +    topEdge.x / 4, trackerPose.y  +    topEdge.y / 4,  trackerPose.z + topEdge.z / 4, \
        trackerPose.x + bottomEdge.x / 4, trackerPose.y  + bottomEdge.y / 4,  trackerPose.z + bottomEdge.z / 4, \
        trackerPose.x +    topEdge.x / 2, trackerPose.y  +    topEdge.y / 2,  trackerPose.z + topEdge.z / 2, \
        trackerPose.x + bottomEdge.x / 2, trackerPose.y  + bottomEdge.y / 2,  trackerPose.z + bottomEdge.z / 2, \
        trackerPose.x +    topEdge.x, trackerPose.y + topEdge.y, trackerPose.z + topEdge.z, \
        trackerPose.x + bottomEdge.x, trackerPose.y + bottomEdge.y, trackerPose.z + bottomEdge.z, \
        trackerPose.x, trackerPose.y, trackerPose.z, \
        trackerPose.x +    topEdge.x + (bottomEdge.x - topEdge.x) / 2, trackerPose.y + topEdge.y + (bottomEdge.y - topEdge.y) / 2, trackerPose.z + topEdge.z + (bottomEdge.z - topEdge.z) / 2, \
        trackerPose.x, trackerPose.y, trackerPose.z, \
        trackerPose.x + bottomEdge.x, trackerPose.y + bottomEdge.y, trackerPose.z + bottomEdge.z

OVRTrackerChaperone::~OVRTrackerChaperone()
{
    if (glIsBuffer(m_vertexBuffers[0]))
    {
        glDeleteBuffers(4, m_vertexBuffers);
    }

    if (glIsVertexArray(m_vertexArray))
    {
        glDeleteVertexArrays(1, &m_vertexArray);
    }
}

void OVRTrackerChaperone::Recalculate(ovrSession session, const OVR::Vector3f &headPos)
{
    ovrTrackerDesc tDesc    = ovr_GetTrackerDesc(session, 0);
    ovrTrackerPose tPose    = ovr_GetTrackerPose(session, 0);
    OVR::Vector3f  trackerPose = OVR::Vector3f(tPose.LeveledPose.Position);

    float trackerFar  = tDesc.FrustumFarZInMeters;
    float trackerNear = tDesc.FrustumNearZInMeters;
    float trackerHFov = tDesc.FrustumHFovInRadians;
    float trackerVFov = tDesc.FrustumVFovInRadians;

    float hScale = tanf(trackerHFov / 2.f);
    float vScale = tanf(trackerVFov / 2.f);

    // camera orientation quaternion
    OVR::Quatf trackerOrientationQuat(tPose.LeveledPose.Orientation.x,
                                      tPose.LeveledPose.Orientation.y,
                                      tPose.LeveledPose.Orientation.z,
                                      tPose.LeveledPose.Orientation.w);

    // orientation indicator vector running from camera pose to near plane
    OVR::Vector3f trackerOrientationVec(0.f, 0.f, trackerNear);

    // near plane vertex positions
    OVR::Vector3f nearV1(-hScale * trackerNear, vScale * trackerNear, trackerNear);
    OVR::Vector3f nearV2(-hScale * trackerNear, -vScale * trackerNear, trackerNear);
    OVR::Vector3f nearV3(hScale * trackerNear, -vScale * trackerNear, trackerNear);
    OVR::Vector3f nearV4(hScale * trackerNear, vScale * trackerNear, trackerNear);

    // far plane vertex positions
    OVR::Vector3f farV1(-hScale * trackerFar, vScale * trackerFar, trackerFar);
    OVR::Vector3f farV2(-hScale * trackerFar, -vScale * trackerFar, trackerFar);
    OVR::Vector3f farV3(hScale * trackerFar, -vScale * trackerFar, trackerFar);
    OVR::Vector3f farV4(hScale * trackerFar, vScale * trackerFar, trackerFar);

    // reorient all vectors by current tracker camera orientation
    trackerOrientationVec = trackerOrientationQuat.Rotate(trackerOrientationVec);

    nearV1 = trackerOrientationQuat.Rotate(nearV1);
    nearV2 = trackerOrientationQuat.Rotate(nearV2);
    nearV3 = trackerOrientationQuat.Rotate(nearV3);
    nearV4 = trackerOrientationQuat.Rotate(nearV4);

    farV1 = trackerOrientationQuat.Rotate(farV1);
    farV2 = trackerOrientationQuat.Rotate(farV2);
    farV3 = trackerOrientationQuat.Rotate(farV3);
    farV4 = trackerOrientationQuat.Rotate(farV4);

    const GLfloat leftPlaneVertexData[]   = { ChaperonePlanePoints(farV1, farV2) };
    const GLfloat rightPlaneVertexData[]  = { ChaperonePlanePoints(farV4, farV3) };
    const GLfloat topPlaneVertexData[]    = { ChaperonePlanePoints(farV1, farV4) };
    const GLfloat bottomPlaneVertexData[] = { ChaperonePlanePoints(farV2, farV3) };

    // near plane of the tracking camera
    const GLfloat nearPlaneVertexData[] = {
        trackerPose.x + nearV1.x, trackerPose.y + nearV1.y, trackerPose.z + nearV1.z,
        trackerPose.x + nearV2.x, trackerPose.y + nearV2.y, trackerPose.z + nearV2.z,
        trackerPose.x + nearV3.x, trackerPose.y + nearV3.y, trackerPose.z + nearV3.z,
        trackerPose.x + nearV4.x, trackerPose.y + nearV4.y, trackerPose.z + nearV4.z
    };

    OVR::Vector3f planeNormal = (nearV2 - nearV1).Cross(nearV3 - nearV1);
    planeNormal.Normalize();
    float d = -(planeNormal.x * nearV1.x + planeNormal.y * nearV1.y + planeNormal.z * nearV1.z);

    float distFromPlane = planeNormal.x * (headPos.x - trackerPose.x) + planeNormal.y * (headPos.y - trackerPose.y) + planeNormal.z * (headPos.z - trackerPose.z) + d;

    // left plane
    OVR::Vector3f leftNormal = (farV1 - trackerPose).Cross(farV2 - trackerPose).Normalized();
    d = -(leftNormal.x * trackerPose.x + leftNormal.y * trackerPose.y + leftNormal.z * trackerPose.z);
    m_planeDistances[0] = leftNormal.x * headPos.x + leftNormal.y * headPos.y + leftNormal.z * headPos.z + d;

    // top plane (flip normal sign so it points inwards)
    OVR::Vector3f topNormal = -(farV1 - trackerPose).Cross(farV4 - trackerPose).Normalized();
    d = -(topNormal.x * trackerPose.x + topNormal.y * trackerPose.y + topNormal.z * trackerPose.z);
    m_planeDistances[1] = topNormal.x * headPos.x + topNormal.y * headPos.y + topNormal.z * headPos.z + d;

    // right plane (flip normal sign so it points inwards)
    OVR::Vector3f rightNormal = -(farV4 - trackerPose).Cross(farV3 - trackerPose).Normalized();
    d = -(rightNormal.x * trackerPose.x + rightNormal.y * trackerPose.y + rightNormal.z * trackerPose.z);
    m_planeDistances[2] = rightNormal.x * headPos.x + rightNormal.y * headPos.y + rightNormal.z * headPos.z + d;

    // bottom plane
    OVR::Vector3f bottomNormal = (farV2 - trackerPose).Cross(farV3 - trackerPose).Normalized();
    d = -(bottomNormal.x * trackerPose.x + bottomNormal.y * trackerPose.y + bottomNormal.z * trackerPose.z);
    m_planeDistances[3] = bottomNormal.x * headPos.x + bottomNormal.y * headPos.y + bottomNormal.z * headPos.z + d;

    // far plane of the tracking camera
    const GLfloat farPlaneVertexData[] = {
        trackerPose.x + farV1.x, trackerPose.y + farV1.y, trackerPose.z + farV1.z,
        trackerPose.x + farV2.x, trackerPose.y + farV2.y, trackerPose.z + farV2.z,
        trackerPose.x + farV3.x, trackerPose.y + farV3.y, trackerPose.z + farV3.z,
        trackerPose.x + farV4.x, trackerPose.y + farV4.y, trackerPose.z + farV4.z
    };

    if (glIsBuffer(m_vertexBuffers[0]))
        glDeleteBuffers(4, m_vertexBuffers);

    if (glIsVertexArray(m_vertexArray))
        glDeleteVertexArrays(1, &m_vertexArray);

    // create line VAO
    glGenVertexArrays(1, &m_vertexArray);
    glGenBuffers(4, m_vertexBuffers);

    glBindVertexArray(m_vertexArray);

    // Get a handle for our buffers (VBO)
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(leftPlaneVertexData), leftPlaneVertexData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(topPlaneVertexData), topPlaneVertexData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rightPlaneVertexData), rightPlaneVertexData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bottomPlaneVertexData), bottomPlaneVertexData, GL_STATIC_DRAW);
}

void OVRTrackerChaperone::OnRender()
{
    const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::OVRFrustumShader);
    GLuint vertexPositionAttr = glGetAttribLocation(shader.id, "inVertex");

    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // render camera bounds
    glBindVertexArray(m_vertexArray);
    glEnableVertexAttribArray(vertexPositionAttr);

    for (int i = 0; i < 4; i++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[i]);
        glVertexAttribPointer(vertexPositionAttr, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // draw the frustum lines
        const float frustumColor[4] = { 0.0f, 1.f, 1.f, 1.f - m_planeDistances[i] * 7.5f };
        glUniform4fv(shader.uniforms[VertexColor], 1, frustumColor);
        glDrawArrays(GL_LINES, 0, 16);
    }

    glDisableVertexAttribArray(vertexPositionAttr);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

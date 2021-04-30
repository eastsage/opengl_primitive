#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "common.h"
#include "shader.h"
#include "program.h"
#include "buffer.h"
#include "vertex_layout.h"
#include "texture.h"

CLASS_PTR(Context)
class Context {
public:
    static ContextUPtr Create();
    void CreateBox();
    void CreateCylinder(float upperRadius, float lowerRadius, int segment, float height);
    void CreateSphere(float radius, int sectorCount, int stackCount);
    void CreateDonut(float ringRadius, float tubeRadius, int rsegment, int csegment, int texture);
    void Render();    
    void ProcessInput(GLFWwindow* window);
    void Reshape(int width, int height);
    void MouseMove(double x, double y);
    void MouseButton(int button, int action, double x, double y);

private:
    Context() {}
    bool Init();
    ProgramUPtr m_program;

    VertexLayoutUPtr m_vertexLayout;
    BufferUPtr m_vertexBuffer;
    BufferUPtr m_indexBuffer;
    int m_clyinderIndexCount {6};
    int m_sphereIndexCount {6};

    //텍스처가 총 3개이기 때문에 변수 추가
    TextureUPtr m_texture0;
    TextureUPtr m_texture1;
    TextureUPtr m_texture2;

    // clear color
    glm::vec4 m_clearColor { glm::vec4(0.5f, 0.5f, 0.5f, 0.0f) };

    // camera parameter
    bool m_cameraControl { false };
    glm::vec2 m_prevMousePos { glm::vec2(0.0f) };
    float m_cameraPitch { 0.0f };
    float m_cameraYaw { 0.0f };
    glm::vec3 m_cameraPos { glm::vec3(0.0f, 0.0f, 3.0f) };
    glm::vec3 m_cameraFront { glm::vec3(0.0f, 0.0f, -1.0f) };
    glm::vec3 m_cameraUp { glm::vec3(0.0f, 1.0f, 0.0f) };

    //회전각
    glm::vec3 m_rotation { glm::vec3(0.0f, 0.0f, 0.0f) };
    //각도조절
    glm::vec3 m_radius1 { glm::vec3(0.0f, 0.0f, 0.0f) };
    //크기조절
    glm::vec3 m_scale1 { glm::vec3(1.0f, 1.0f, 1.0f) };

    int m_width { WINDOW_WIDTH };
    int m_height { WINDOW_HEIGHT };

    glm::mat4 m_projection = glm::perspective(glm::radians(45.0f),
                (float)m_width / (float)m_height, 0.01f, 30.0f);
    glm::mat4 m_view = glm::lookAt(
                m_cameraPos,
                m_cameraPos + m_cameraFront,
                m_cameraUp);
    glm::mat4 m_transform;

    const float pi = 3.141592f;


    //vertices count, triangles count
    int m_boxVerticesCount;
    int m_boxTrianglesCount;
    int m_cylinderVerticesCount;
    int m_ctylinderTrianglesCount;
    int m_sphereVerticesCount;
    int m_sphereTrianglesCount;
    //cylinder mem
    float c_upperRadius = 0.5f;
    float c_lowerRadius = 0.5f;
    int c_segment = 32; 
    float c_height = 1.0f;
    //sphere mem
    float s_radius = 0.5f;
    int s_sectorCount = 32;
    int s_stackCount = 16;
};

#endif // __CONTEXT_H__

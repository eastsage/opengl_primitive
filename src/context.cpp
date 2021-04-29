#include "context.h"
#include "image.h"
#include <imgui.h>

ContextUPtr Context::Create() {
  auto context = ContextUPtr(new Context());
  if (!context->Init())
    return nullptr;
  return std::move(context);
}

void Context::ProcessInput(GLFWwindow* window) {

    if (!m_cameraControl)
        return;

    const float cameraSpeed = 0.05f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * m_cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * m_cameraFront;

    auto cameraRight = glm::normalize(glm::cross(m_cameraUp, -m_cameraFront));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * cameraRight;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * cameraRight;    

    auto cameraUp = glm::normalize(glm::cross(-m_cameraFront, cameraRight));
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * cameraUp;
}

void Context::Reshape(int width, int height) {
    m_width = width;
    m_height = height;
    glViewport(0, 0, m_width, m_height);
}

void Context::MouseMove(double x, double y) {
    if (!m_cameraControl)
        return;
    auto pos = glm::vec2((float)x, (float)y);
    auto deltaPos = pos - m_prevMousePos;

    const float cameraRotSpeed = 0.8f;
    m_cameraYaw -= deltaPos.x * cameraRotSpeed;
    m_cameraPitch -= deltaPos.y * cameraRotSpeed;

    if (m_cameraYaw < 0.0f)   m_cameraYaw += 360.0f;
    if (m_cameraYaw > 360.0f) m_cameraYaw -= 360.0f;

    if (m_cameraPitch > 89.0f)  m_cameraPitch = 89.0f;
    if (m_cameraPitch < -89.0f) m_cameraPitch = -89.0f;

    m_prevMousePos = pos;    
}

void Context::MouseButton(int button, int action, double x, double y) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            // 마우스 조작 시작 시점에 현재 마우스 커서 위치 저장
            m_prevMousePos = glm::vec2((float)x, (float)y);
            m_cameraControl = true;
        }
        else if (action == GLFW_RELEASE) {
            m_cameraControl = false;
        }
    }
}

bool Context::Init() {

    ShaderPtr vertShader = Shader::CreateFromFile("./shader/texture.vs", GL_VERTEX_SHADER);
    ShaderPtr fragShader = Shader::CreateFromFile("./shader/texture.fs", GL_FRAGMENT_SHADER);
    if (!vertShader || !fragShader)
        return false;
    SPDLOG_INFO("vertex shader id: {}", vertShader->Get());
    SPDLOG_INFO("fragment shader id: {}", fragShader->Get());

    m_program = Program::Create({fragShader, vertShader});
    if (!m_program)
        return false;
    SPDLOG_INFO("program id: {}", m_program->Get());

    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    // 이미지 로딩
    auto image0 = Image::Load("./image/wood.png");
    if (!image0) 
        return false;
    SPDLOG_INFO("image: {}x{}, {} channels",
        image0->GetWidth(), image0->GetHeight(), image0->GetChannelCount());

    m_texture0 = Texture::CreateFromImage(image0.get());

    auto image1 = Image::Load("./image/metal.jpg");
    m_texture1 = Texture::CreateFromImage(image1.get());
    auto image2 = Image::Load("./image/earth.jpg");
    m_texture2 = Texture::CreateFromImage(image2.get());

    m_program->Use(); 	
    m_program->SetUniform("tex", 0);

    return true;
}

void Context::Render() {
    //imgui에 필요한 변수들
    const char* texture[] = { "wood", "metal", "earth"};
    static int texture_current = 0; 
    static bool animation = false;
    const char* primitive[] = { "box", "cylinder", "sphere", "donut"};
    static int primitive_current = 0; 
    
    //imgui 코드
    if (ImGui::Begin("ui window")) {
        if (ImGui::ColorEdit4("clear color", glm::value_ptr(m_clearColor))) {
            glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
        }
        ImGui::Separator();
        if (ImGui::Button("reset clear color")) {
            m_clearColor.r = 0.5f;
            m_clearColor.g = 0.5f;
            m_clearColor.b = 0.5f;
            m_clearColor.a = 0.0f;
            glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
        }
        ImGui::Separator();
        ImGui::DragFloat3("camera pos", glm::value_ptr(m_cameraPos), 0.1f);
        ImGui::DragFloat("camera yaw", &m_cameraYaw, 0.5f);
        ImGui::DragFloat("camera pitch", &m_cameraPitch, 0.5f, -89.0f, 89.0f);
        ImGui::Separator();
        if (ImGui::Button("reset camera")) {
            m_cameraYaw = 0.0f;
            m_cameraPitch = 0.0f;
            m_cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
        }
        ImGui::Separator();
        ImGui::Combo("primitive", &primitive_current, primitive, IM_ARRAYSIZE(primitive));
        switch (primitive_current) {
            case 0: ImGui::LabelText("# vertices", (const char)glBufferData(GL_BUFFER_DATA_SIZE));
                    ImGui::LabelText("trianles", "12");
                    break;
            case 1: ImGui::DragFloat("topradius", &c_topRadius, 0.1f, 0.1f, 100.0f);
                    ImGui::DragFloat("botradius", &c_botRadius, 0.1f, 0.1f, 100.0f);
                    ImGui::DragInt("segment", &c_segment, 1, 3, 128);
                    ImGui::DragFloat("height", &c_height, 0.1f, 0.1f, 100.0f);
                    if (ImGui::Button("reset cylinder")) {
                        c_topRadius = 0.5f; c_botRadius = 0.5f;
                        c_segment = 32; c_height = 1.0f;
                    } break;
            case 2: ImGui::DragFloat("radius", &s_radius, 0.1f, 0.1f, 100.0f);
                    ImGui::DragInt("stackcount", &s_stackCount, 1, 3, 100);
                    ImGui::DragInt("sectorcount", &s_sectorCount, 1, 3, 100);
                    if (ImGui::Button("reset sphere")) {
                        s_radius = 0.5f;
                        s_stackCount = 16; s_sectorCount = 32;
                    }
                    break;
        }
        ImGui::Combo("texture", &texture_current, texture, IM_ARRAYSIZE(texture));
        ImGui::Separator();
        ImGui::Checkbox("animation", &animation);
        ImGui::DragFloat3("rotation", glm::value_ptr(m_rotation), 0.01f);
        ImGui::DragFloat3("scale", glm::value_ptr(m_scale1), 0.01f);
        ImGui::Separator();
        if (ImGui::Button("reset transform")) {
            m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
            m_scale1 = glm::vec3(1.0f, 1.0f, 1.0f);
        }
        ImGui::Separator();
    }
    ImGui::End();

    //기능 구현 코드
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);


    //텍스처 선택
    switch(texture_current) {

        case 0: glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_texture0->Get());
                m_program->SetUniform("tex", 0);
                break;
        case 1: glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, m_texture1->Get());
                m_program->SetUniform("tex", 1);
                break;
        case 2: glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, m_texture2->Get());
                m_program->SetUniform("tex", 2);
                break;
    }

    m_cameraFront =
    glm::rotate(glm::mat4(1.0f), glm::radians(m_cameraYaw), glm::vec3(0.0f, 1.0f, 0.0f)) *
    glm::rotate(glm::mat4(1.0f), glm::radians(m_cameraPitch), glm::vec3(1.0f, 0.0f, 0.0f)) *
    glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

    auto m_projection = glm::perspective(glm::radians(45.0f),
        (float)m_width / (float)m_height, 0.01f, 30.0f);
    auto m_view = glm::lookAt(
        m_cameraPos,
        m_cameraPos + m_cameraFront,
        m_cameraUp);

    //스케일 조절 변수
    glm::mat4 m_scale2 { glm::scale(glm::mat4(1.0f), m_scale1) };

    //도형 선택 및 애니메이션 적용
    switch (primitive_current) {

        case 0: CreateBox();
                if(!animation){
                    m_transform = m_projection * m_view * m_scale2;
                    m_program->SetUniform("transform", m_transform);
                    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
                    }
                    else {
                        if (m_rotation == glm::vec3( 0.0f, 0.0f, 0.0f)) {
                            m_transform = m_projection * m_view * m_scale2;
                            m_program->SetUniform("transform", m_transform);
                            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
                        }
                        else {
                            auto model = glm::rotate(glm::mat4(1.0f),
                                glm::radians((float)glfwGetTime() * 120.0f), m_rotation);
                            m_transform = m_projection * m_view * m_scale2 * model;
                            m_program->SetUniform("transform", m_transform);
                            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
                        }
                }break;
        case 1: CreateCylinder(c_topRadius, c_botRadius, c_segment, c_height);
                if(!animation){
                    m_transform = m_projection * m_view * m_scale2;
                    m_program->SetUniform("transform", m_transform);
                    glDrawElements(GL_TRIANGLES, m_clyinderIndexCount, GL_UNSIGNED_INT, 0);
                }
                else {
                    if (m_rotation == glm::vec3( 0.0f, 0.0f, 0.0f)) {
                        m_transform = m_projection * m_view * m_scale2;
                        m_program->SetUniform("transform", m_transform);
                        glDrawElements(GL_TRIANGLES, m_clyinderIndexCount, GL_UNSIGNED_INT, 0);
                    }
                    else {
                        auto model = glm::rotate(glm::mat4(1.0f),
                            glm::radians((float)glfwGetTime() * 120.0f), m_rotation);
                        m_transform = m_projection * m_view * m_scale2 * model;
                        m_program->SetUniform("transform", m_transform);
                        glDrawElements(GL_TRIANGLES, m_clyinderIndexCount, GL_UNSIGNED_INT, 0);
                    }
                }break;
        case 2: CreateSphere(s_radius, s_sectorCount, s_stackCount);
                if(!animation){
                    m_transform = m_projection * m_view * m_scale2;
                    m_program->SetUniform("transform", m_transform);
                    glDrawElements(GL_TRIANGLES, m_sphereIndexCount, GL_UNSIGNED_INT, 0);
                }
                else {
                    if (m_rotation == glm::vec3( 0.0f, 0.0f, 0.0f)) {
                        m_transform = m_projection * m_view * m_scale2;
                        m_program->SetUniform("transform", m_transform);
                        glDrawElements(GL_TRIANGLES, m_sphereIndexCount, GL_UNSIGNED_INT, 0);
                    }
                    else {
                        auto model = glm::rotate(glm::mat4(1.0f),
                            glm::radians((float)glfwGetTime() * 120.0f), m_rotation);
                        m_transform = m_projection * m_view * m_scale2 * model;
                        m_program->SetUniform("transform", m_transform);
                        glDrawElements(GL_TRIANGLES, m_sphereIndexCount, GL_UNSIGNED_INT, 0);
                    }
                }break;
    }
}

//box 구현 코드
void Context::CreateBox() {
    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f,

        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,

         0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,

        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
    };

    uint32_t indices[] = {
        0,  2,  1,  2,  0,  3,
        4,  5,  6,  6,  7,  4,
        8,  9, 10, 10, 11,  8,
        12, 14, 13, 14, 12, 15,
        16, 17, 18, 18, 19, 16,
        20, 22, 21, 22, 20, 23,
    };

    m_vertexLayout = VertexLayout::Create();
    m_vertexBuffer = Buffer::CreateWithData(GL_ARRAY_BUFFER, 
        GL_STATIC_DRAW, vertices, sizeof(float) * 120);

    m_vertexLayout->SetAttrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);
    m_vertexLayout->SetAttrib(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, sizeof(float) * 3);

    m_indexBuffer = Buffer::CreateWithData(GL_ELEMENT_ARRAY_BUFFER,
        GL_STATIC_DRAW, indices, sizeof(uint32_t) * 36);
}

//실린더 구현 코드
void Context::CreateCylinder(float topRadius, float botRadius, int segment, float height) {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    //z축이 -인 원
    vertices.push_back(0.0f);
    vertices.push_back(-height/2);
    vertices.push_back(0.0f);
    for (int i = 0; i < segment ; i++) {
        float angle = (360.0f / segment * i) * pi / 180.0f;
        float x = cosf(angle) * botRadius;
        float z = sinf(angle) * botRadius;
        vertices.push_back(x);
        vertices.push_back(-height/2);
        vertices.push_back(z);
    }
    //z축이 +인 원
    vertices.push_back(0.0f);
    vertices.push_back(height/2);
    vertices.push_back(0.0f);
    for (int i = 0; i < segment ; i++) {
        float angle = (360.0f / segment * i) * pi / 180.0f;
        float x = cosf(angle) * topRadius;
        float z = sinf(angle) * topRadius;
        vertices.push_back(x);
        vertices.push_back(height/2);
        vertices.push_back(z);
    }
    //z축이 -인 원
    for (int i = 0; i < segment; i++) {
        indices.push_back(0);
        indices.push_back(i + 1);
        if ( i == segment - 1)
            indices.push_back(1);
        else
            indices.push_back(i + 2);
    }
    //z축이 +인 원
    for (int i = segment; i < segment * 2 + 1; i++) {
        indices.push_back(segment + 1);
        indices.push_back(i + 1);
        if ( i == segment * 2)
            indices.push_back(segment + 2);
        else
            indices.push_back(i + 2);
    }
    //두 원 사이를 채우는 코드
    for (int i = 1; i < segment; i++) {
        indices.push_back(i);
        indices.push_back(i + 1);
        indices.push_back(i + segment + 1);

        indices.push_back(i + segment + 1);
        indices.push_back(i + segment + 2);
        indices.push_back(i + 1);
    }
    indices.push_back(1);
    indices.push_back(segment);
    indices.push_back(segment * 2 + 1);
    
    indices.push_back(1);
    indices.push_back(segment + 2);
    indices.push_back(segment * 2 + 1);


    
    m_vertexLayout = VertexLayout::Create();
    m_vertexBuffer = Buffer::CreateWithData(GL_ARRAY_BUFFER,
        GL_STATIC_DRAW, vertices.data(), sizeof(float) * vertices.size());
    
    m_vertexLayout->SetAttrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

    m_indexBuffer = Buffer::CreateWithData(GL_ELEMENT_ARRAY_BUFFER,
        GL_STATIC_DRAW, indices.data(), sizeof(uint32_t) * indices.size());

    m_clyinderIndexCount = (int)indices.size();
}

//스피어 구현 코드
void Context::CreateSphere(float radius, int sectorCount, int stackCount) {
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<uint32_t> indices;

    float x = 0, y = 0, z = 0, xy = 0;
    float nx = 0, ny = 0, nz = 0, lengthInv = 1.0f / radius;
    float s = 0, t = 0;
    int k1 = 0, k2 = 0;

    float sectorStep = 2 * pi / s_sectorCount;
    float stackStep = pi / s_stackCount;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= s_stackCount; ++i) {
        stackAngle = pi / 2 - i * stackStep;
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);

        for(int j = 0; j <= s_sectorCount; ++j) {
            sectorAngle = j * sectorStep;

            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            normals.push_back(nx);
            normals.push_back(ny);
            normals.push_back(nz);

            s = (float)j / s_sectorCount;
            t = (float)i / s_stackCount;
            texCoords.push_back(s);
            texCoords.push_back(t);
        }
    }

    for(int i = 0; i < s_stackCount; ++i) {
        k1 = i * (s_sectorCount + 1);
        k2 = k1 + s_sectorCount + 1;

        for(int j = 0; j < s_sectorCount; ++j, ++k1, ++k2) {
            if(i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if(i != (s_stackCount-1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    m_vertexLayout = VertexLayout::Create();
    m_vertexBuffer = Buffer::CreateWithData(GL_ARRAY_BUFFER,
        GL_STATIC_DRAW, vertices.data(), sizeof(float) * vertices.size());
    
    m_vertexLayout->SetAttrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

    m_indexBuffer = Buffer::CreateWithData(GL_ELEMENT_ARRAY_BUFFER,
        GL_STATIC_DRAW, indices.data(), sizeof(uint32_t) * indices.size());

    m_sphereIndexCount = (int)indices.size();
}

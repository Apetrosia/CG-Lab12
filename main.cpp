#include <SFML/Graphics.hpp>
#include <GL/glew.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>

// IDs ��������� ���������, VAO, VBO � EBO
GLuint Program, VAO, VBO, EBO;
GLuint modelLoc;

GLfloat M_PI = 3.14159265358979323846;
int segments = 100;

GLfloat angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
GLfloat rotationMatrix[16];

int figure_mode = 0;
std::vector<GLuint> indices;

void CreateRotationMatrix()
{
    // ������� �������� ������ ����
    GLfloat rotationX[16] = {
        1, 0, 0, 0,
        0, cos(angleX), -sin(angleX), 0,
        0, sin(angleX), cos(angleX), 0,
        0, 0, 0, 1
    };

    GLfloat rotationY[16] = {
        cos(angleY), 0, sin(angleY), 0,
        0, 1, 0, 0,
        -sin(angleY), 0, cos(angleY), 0,
        0, 0, 0, 1
    };

    GLfloat rotationZ[16] = {
        cos(angleZ), -sin(angleZ), 0, 0,
        sin(angleZ), cos(angleZ), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    // ��������� ��������� �������
    GLfloat temp[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    // ����������� ������� (Y * X)
    for (int i = 0; i < 16; i++) {
        temp[i] =
            rotationY[i % 4] * rotationX[i / 4 * 4] +
            rotationY[i % 4 + 4] * rotationX[i / 4 * 4 + 1] +
            rotationY[i % 4 + 8] * rotationX[i / 4 * 4 + 2] +
            rotationY[i % 4 + 12] * rotationX[i / 4 * 4 + 3];
    }

    // ����������� ��������� � Z
    for (int i = 0; i < 16; i++) {
        rotationMatrix[i] =
            rotationZ[i % 4] * temp[i / 4 * 4] +
            rotationZ[i % 4 + 4] * temp[i / 4 * 4 + 1] +
            rotationZ[i % 4 + 8] * temp[i / 4 * 4 + 2] +
            rotationZ[i % 4 + 12] * temp[i / 4 * 4 + 3];
    }
}

void ResetAngles()
{
    angleX = 0.0;
    angleY = 0.0;
    angleZ = 0.0;

    CreateRotationMatrix();
}

std::vector<GLfloat> vertices = {
    // ������ ����� ����
    -0.4f, -0.4f,  0.4f,  0.0f, 0.0f, 0.0f, // ������ ����� (���� �����, � � ������ �� �����)
     0.4f, -0.4f,  0.4f,  0.0f, 0.0f, 1.0f, // ������ ������ (�����)
     0.4f,  0.4f,  0.4f,  0.0f, 1.0f, 1.0f, // ������� ������ (�������)
    -0.4f,  0.4f,  0.4f,  0.0f, 1.0f, 0.0f, // ������� ����� (�������)

    // �������� ����� ����
    -0.4f, -0.4f, -0.4f,  1.0f, 0.0f, 0.0f, // ������ ����� (�������)
     0.4f, -0.4f, -0.4f,  1.0f, 0.0f, 1.0f, // ������ ������ (���������)
     0.4f,  0.4f, -0.4f,  1.0f, 1.0f, 1.0f, // ������� ������ (�����)
    -0.4f,  0.4f, -0.4f,  1.0f, 1.0f, 0.0f, // ������� ����� (������)

    //������� ���������
    -0.5f,  0.7f,  0.6f,  1.0f, 0.0f, 0.0f, // ������� (�������)
    -0.6f, -0.7f,  0.6f,  0.0f, 1.0f, 0.0f, // ������ (�������)
     0.7f, -0.1f,  0.6f,  0.0f, 0.0f, 1.0f, // ������ (�����)
     0.0f,  0.0f, -0.6f,  1.0f, 1.0f, 1.0f, // �������� (�����)
};

void GenerateCircleVertexes()
{
    vertices.push_back(0.0f); // ����� ����� (X)
    vertices.push_back(0.0f); // ����� ����� (Y)
    vertices.push_back(0.0f); // ����� ����� (Z)
    vertices.push_back(1.0f); // ����� ���� (R)
    vertices.push_back(1.0f); // ����� ���� (G)
    vertices.push_back(1.0f); // ����� ���� (B)

    for (int i = 0; i <= segments; ++i)
    {
        float angle = 2.0f * M_PI * i / segments;
        float x = cos(angle) * 0.5f;
        float y = sin(angle) * 0.5f;

        float hue = static_cast<float>(i) / segments;
        float r = fabs(hue * 6.0f - 3.0f) - 1.0f;
        float g = 2.0f - fabs(hue * 6.0f - 2.0f);
        float b = 2.0f - fabs(hue * 6.0f - 4.0f);
        r = std::clamp(r, 0.0f, 1.0f);
        g = std::clamp(g, 0.0f, 1.0f);
        b = std::clamp(b, 0.0f, 1.0f);

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(0.0f);
        vertices.push_back(r);
        vertices.push_back(g);
        vertices.push_back(b);
    }
}

// �������� ��� ���������� �������
const char* VertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec3 coord;
    layout(location = 1) in vec3 color;

    uniform mat4 model;
    out vec3 fragColor;

    void main() {
        gl_Position = model * vec4(coord, 1.0);
        fragColor = color;
    }
)";

// �������� ��� ������������ �������
const char* FragShaderSource = R"(
    #version 330 core
    in vec3 fragColor;
    out vec4 color;

    void main() {
        color = vec4(fragColor, 1.0);
    }
)";

void ShaderLog(unsigned int shader) {
    int infologLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);

    if (infologLen > 1) {
        int charsWritten = 0;
        std::vector<char> infoLog(infologLen);
        glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog.data());
        std::cout << "infoLog: " << infoLog.data() << std::endl;
    }
}

void InitShader() {
    // ������� ��������� ������
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &VertexShaderSource, NULL);
    glCompileShader(vShader);
    std::cout << "vertex shader:\n";
    ShaderLog(vShader);

    // ������� ����������� ������
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &FragShaderSource, NULL);
    glCompileShader(fShader);
    std::cout << "fragment shader:\n";
    ShaderLog(fShader);

    // ������� ��������� ���������
    Program = glCreateProgram();
    glAttachShader(Program, vShader);
    glAttachShader(Program, fShader);
    glLinkProgram(Program);

    int link_ok;
    glGetProgramiv(Program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        std::cout << "error linking shaders.\n";
        return;
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);
}

void InitBuffers()
{
    // ������� ��� ��������� �����
    indices.clear();
    switch (figure_mode)
    {
    case 0: // ��������
        indices = {
            // ������ �����
            8, 9, 10,
            // ����� �����
            8, 9, 11,
            // ������ �����
            9, 10, 11,
            // �������� �����
            8, 10, 11
        };
        break;
    case 1: // ���
        indices = {
            // �������� �����
            0, 1, 2,
            2, 3, 0,
            // ������ �����
            4, 5, 6,
            6, 7, 4,
            // ����� �����
            0, 3, 7,
            7, 4, 0,
            // ������ �����
            1, 2, 6,
            6, 5, 1,
            // ������� �����
            3, 2, 6,
            6, 7, 3,
            // ������ �����
            0, 1, 5,
            5, 4, 0
        };
        break;
    case 2: // ��� � ���������
        indices = {
            // �������� �����
            0, 1, 2,
            2, 3, 0,
            // ������ �����
            4, 5, 6,
            6, 7, 4,
            // ����� �����
            0, 3, 7,
            7, 4, 0,
            // ������ �����
            1, 2, 6,
            6, 5, 1,
            // ������� �����
            3, 2, 6,
            6, 7, 3,
            // ������ �����
            0, 1, 5,
            5, 4, 0
        };
        break;
    case 3: // ����
        for (int i = 1; i <= segments; i++)
        {
            indices.push_back(12);
            indices.push_back(i + 12);
            indices.push_back(i + 13);
        }
        break;
    }

    // ������� VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // ������� VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    // ������� EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // ������� ���������
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // ������� �����
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Draw() {
    glUseProgram(Program);

    CreateRotationMatrix();
    modelLoc = glGetUniformLocation(Program, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, rotationMatrix);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Release() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(Program);
}

void Init()
{
    // ������������� �������� � �������
    InitShader();
    InitBuffers();

    glEnable(GL_DEPTH_TEST);
}

int main() {
    sf::Window window(sf::VideoMode(800, 800), "3D figures", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);

    GenerateCircleVertexes();

    glewInit();

    Init();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                case sf::Keyboard::Num1:
                    figure_mode = 0;
                    ResetAngles();
                    InitBuffers();
                    break;
                case sf::Keyboard::Num2:
                    figure_mode = 1;
                    ResetAngles();
                    InitBuffers();
                    break;
                case sf::Keyboard::Num3:
                    figure_mode = 2;
                    ResetAngles();
                    InitBuffers();
                    break;
                case sf::Keyboard::Num4:
                    figure_mode = 3;
                    ResetAngles();
                    InitBuffers();
                    break;
                case sf::Keyboard::A:
                    if (figure_mode < 3)
                    {
                        angleY -= 0.01;
                        CreateRotationMatrix();
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, rotationMatrix);
                    }
                    break;
                case sf::Keyboard::D:
                    if (figure_mode < 3)
                    {
                        angleY += 0.01;
                        CreateRotationMatrix();
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, rotationMatrix);
                    }
                    break;
                case sf::Keyboard::W:
                    if (figure_mode < 3)
                    {
                        angleX -= 0.01;
                        CreateRotationMatrix();
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, rotationMatrix);
                    }
                    break;
                case sf::Keyboard::S:
                    if (figure_mode < 3)
                    {
                        angleX += 0.01;
                        CreateRotationMatrix();
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, rotationMatrix);
                    }
                    break;
                case sf::Keyboard::Q:
                    if (figure_mode < 3)
                    {
                        angleZ -= 0.01;
                        CreateRotationMatrix();
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, rotationMatrix);
                    }
                    break;
                case sf::Keyboard::E:
                    if (figure_mode < 3)
                    {
                        angleZ += 0.01;
                        CreateRotationMatrix();
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, rotationMatrix);
                    }
                    break;
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Draw();
        window.display();
    }

    Release();
    return 0;
}

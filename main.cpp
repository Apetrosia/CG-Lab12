#include <SFML/Graphics.hpp>
#include <GL/glew.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>

// IDs шейдерной программы, VAO, VBO и EBO
GLuint Program, VAO, VBO, EBO;
GLuint modelLoc;

GLuint texture1;
GLuint texture2;
float mixFactor = 1.0f;

GLfloat M_PI = 3.14159265358979323846;
int segments = 100;

GLfloat angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
GLfloat scaleX = 1.0f, scaleY = 1.0f, scaleZ = 1.0f;
GLfloat transformMatrix[16];

int figure_mode = 0;
std::vector<GLuint> indices;

void CreateTransformMatrix()
{
    // Матрицы вращения и масштабирования
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

    GLfloat scaleMatrix[16] = {
        scaleX, 0, 0, 0,
        0, scaleY, 0, 0,
        0, 0, scaleZ, 0,
        0, 0, 0, 1
    };

    // Умножение матриц (Rotation * Scale)
    GLfloat temp[16];
    for (int i = 0; i < 16; i++) {
        temp[i] =
            rotationY[i % 4] * rotationX[i / 4 * 4] +
            rotationY[i % 4 + 4] * rotationX[i / 4 * 4 + 1] +
            rotationY[i % 4 + 8] * rotationX[i / 4 * 4 + 2] +
            rotationY[i % 4 + 12] * rotationX[i / 4 * 4 + 3];
    }

    GLfloat temp1[16];
    for (int i = 0; i < 16; i++) {
        temp1[i] =
            rotationZ[i % 4] * temp[i / 4 * 4] +
            rotationZ[i % 4 + 4] * temp[i / 4 * 4 + 1] +
            rotationZ[i % 4 + 8] * temp[i / 4 * 4 + 2] +
            rotationZ[i % 4 + 12] * temp[i / 4 * 4 + 3];
    }

    for (int i = 0; i < 16; i++) {
        transformMatrix[i] =
            scaleMatrix[i % 4] * temp1[i / 4 * 4] +
            scaleMatrix[i % 4 + 4] * temp1[i / 4 * 4 + 1] +
            scaleMatrix[i % 4 + 8] * temp1[i / 4 * 4 + 2] +
            scaleMatrix[i % 4 + 12] * temp1[i / 4 * 4 + 3];
    }
}

void checkOpenGLerror() {
    /*
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL Error: " << error << std::endl;

        switch (error) {
        case GL_INVALID_OPERATION:
            std::cout << "GL_INVALID_OPERATION: Неверная операция." << std::endl;
            break;
        case GL_INVALID_VALUE:
            std::cout << "GL_INVALID_VALUE: Неверное значение." << std::endl;
            break;
        case GL_INVALID_ENUM:
            std::cout << "GL_INVALID_ENUM: Неверный перечисляемый тип." << std::endl;
            break;
        case GL_OUT_OF_MEMORY:
            std::cout << "GL_OUT_OF_MEMORY: Недостаточно памяти." << std::endl;
            break;
        default:
            std::cout << "Неизвестный код ошибки: " << error << std::endl;
            break;
        }
    }
    */
}

void ResetAnglesAndScales(bool textires = false)
{
    angleX = 0.0f;
    angleY = 0.0f;
    angleZ = 0.0f;

    scaleX = 1.0f;
    scaleY = 1.0f;
    scaleZ = 1.0f;

    if (textires)
        mixFactor = 0.5f;
    else
        mixFactor = 1.0f;

    glUniform1f(glGetUniformLocation(Program, "mixFactor"), mixFactor);
    CreateTransformMatrix();
}

GLuint LoadTexture()
{
    sf::Image img, img2;
    if (!img.loadFromFile("pic1.jpg")) {
        std::cerr << "Error loading texture file: " << "pic1.jpg" << std::endl;
        return 0;
    }
    if (!img2.loadFromFile("pic2.jpg")) {
        std::cerr << "Error loading texture file: " << "pic2.jpg" << std::endl;
        return 0;
    }
    img.flipVertically();
    img2.flipVertically();

    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.getSize().x, img.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());

    glGenerateMipmap(GL_TEXTURE_2D);
    // glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img2.getSize().x, img2.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, img2.getPixelsPtr());

    glGenerateMipmap(GL_TEXTURE_2D);
    // glBindTexture(GL_TEXTURE_2D, 1);

    checkOpenGLerror();
}

std::vector<GLfloat> verticesForTex1 = {
    // Задняя грань куба
    -0.4f, -0.4f,  0.4f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f, // Нижняя левая (тоже белый, т к черный не видно)
     0.4f, -0.4f,  0.4f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Нижняя правая (синий)
     0.4f,  0.4f,  0.4f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, // Верхняя правая (голубой)
    -0.4f,  0.4f,  0.4f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, // Верхняя левая (зеленый)

    // Передняя грань куба
    -0.4f, -0.4f, -0.4f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, // Нижняя левая (красный)
     0.4f, -0.4f, -0.4f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Нижняя правая (малиновый)
     0.4f,  0.4f, -0.4f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f, // Верхняя правая (белый)
    -0.4f,  0.4f, -0.4f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,  // Верхняя левая (желтый)

    // Левая грань куба
    -0.4f, -0.4f,  0.4f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f, // Нижняя левая (тоже белый, т к черный не видно)
    -0.4f, -0.4f, -0.4f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, // Нижняя левая (красный)
    -0.4f,  0.4f, -0.4f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, // Верхняя левая (желтый)
    -0.4f,  0.4f,  0.4f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, // Верхняя левая (зеленый)

    // Правая грань куба
     0.4f, -0.4f, -0.4f,  1.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Нижняя правая (малиновый)
     0.4f, -0.4f,  0.4f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Нижняя правая (синий)
     0.4f,  0.4f,  0.4f,  0.0f, 1.0f, 1.0f,  1.0f, 1.0f, // Верхняя правая (голубой)
     0.4f,  0.4f, -0.4f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f, // Верхняя правая (белый)

     // Верхняя грань куба
    -0.4f,  0.4f, -0.4f,  1.0f, 1.0f, 0.0f,  0.0f, 0.0f, // Верхняя левая (желтый)
     0.4f,  0.4f, -0.4f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f, // Верхняя правая (белый)
     0.4f,  0.4f,  0.4f,  0.0f, 1.0f, 1.0f,  1.0f, 1.0f, // Верхняя правая (голубой)
    -0.4f,  0.4f,  0.4f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, // Верхняя левая (зеленый)

    // Нижняя грань куба
    -0.4f, -0.4f, -0.4f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, // Нижняя левая (красный)
    -0.4f, -0.4f,  0.4f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f, // Нижняя левая (тоже белый, т к черный не видно)
     0.4f, -0.4f,  0.4f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Нижняя правая (синий)
     0.4f, -0.4f, -0.4f,  1.0f, 0.0f, 1.0f,  1.0f, 1.0f // Нижняя правая (малиновый)
};

std::vector<GLfloat> verticesForTex2 = {
    // Задняя грань куба
    -0.4f, -0.4f,  0.4f,  1.0f, 0.0f, // Нижняя левая (тоже белый, т к черный не видно)
     0.4f, -0.4f,  0.4f,  0.0f, 0.0f, // Нижняя правая (синий)
     0.4f,  0.4f,  0.4f,  0.0f, 1.0f, // Верхняя правая (голубой)
    -0.4f,  0.4f,  0.4f,  1.0f, 1.0f, // Верхняя левая (зеленый)

    // Передняя грань куба
    -0.4f, -0.4f, -0.4f,  0.0f, 0.0f, // Нижняя левая (красный)
     0.4f, -0.4f, -0.4f,  1.0f, 0.0f, // Нижняя правая (малиновый)
     0.4f,  0.4f, -0.4f,  1.0f, 1.0f, // Верхняя правая (белый)
    -0.4f,  0.4f, -0.4f,  0.0f, 1.0f,  // Верхняя левая (желтый)

    // Левая грань куба
    -0.4f, -0.4f,  0.4f,  0.0f, 0.0f, // Нижняя левая (тоже белый, т к черный не видно)
    -0.4f, -0.4f, -0.4f,  1.0f, 0.0f, // Нижняя левая (красный)
    -0.4f,  0.4f, -0.4f,  1.0f, 1.0f, // Верхняя левая (желтый)
    -0.4f,  0.4f,  0.4f,  0.0f, 1.0f, // Верхняя левая (зеленый)

    // Правая грань куба
     0.4f, -0.4f, -0.4f,  0.0f, 0.0f, // Нижняя правая (малиновый)
     0.4f, -0.4f,  0.4f,  1.0f, 0.0f, // Нижняя правая (синий)
     0.4f,  0.4f,  0.4f,  1.0f, 1.0f, // Верхняя правая (голубой)
     0.4f,  0.4f, -0.4f,  0.0f, 1.0f, // Верхняя правая (белый)

     // Верхняя грань куба
    -0.4f,  0.4f, -0.4f,  0.0f, 0.0f, // Верхняя левая (желтый)
     0.4f,  0.4f, -0.4f,  1.0f, 0.0f, // Верхняя правая (белый)
     0.4f,  0.4f,  0.4f,  1.0f, 1.0f, // Верхняя правая (голубой)
    -0.4f,  0.4f,  0.4f,  0.0f, 1.0f, // Верхняя левая (зеленый)

    // Нижняя грань куба
    -0.4f, -0.4f, -0.4f,  0.0f, 1.0f, // Нижняя левая (красный)
    -0.4f, -0.4f,  0.4f,  0.0f, 0.0f, // Нижняя левая (тоже белый, т к черный не видно)
     0.4f, -0.4f,  0.4f,  1.0f, 0.0f, // Нижняя правая (синий)
     0.4f, -0.4f, -0.4f,  1.0f, 1.0f // Нижняя правая (малиновый)
};

std::vector<GLfloat> vertices = {
    //Вершины тетраэдра
    -0.5f,  0.7f,  0.6f,  1.0f, 0.0f, 0.0f,  // Верхняя (красный)
    -0.6f, -0.7f,  0.6f,  0.0f, 1.0f, 0.0f,  // Нижняя (зеленый)
     0.7f, -0.1f,  0.6f,  0.0f, 0.0f, 1.0f,  // Правая (синий)
     0.0f,  0.0f, -0.6f,  1.0f, 1.0f, 1.0f  // Передняя (белый)
};

void GenerateCircleVertexes()
{
    vertices.push_back(0.0f); // Центр круга (X)
    vertices.push_back(0.0f); // Центр круга (Y)
    vertices.push_back(0.0f); // Центр круга (Z)
    vertices.push_back(1.0f); // Белый цвет (R)
    vertices.push_back(1.0f); // Белый цвет (G)
    vertices.push_back(1.0f); // Белый цвет (B)

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

// Цвет
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

const char* FragShaderSource = R"(
    #version 330 core
    in vec3 fragColor;
    out vec4 color;

    uniform float mixFactor;

    void main() {
        color = vec4(fragColor, 1.0);
    }
)";

// Цвет + текстура
const char* VertexShaderSourceTex1 = R"(
    #version 330 core
    layout(location = 0) in vec3 coord;
    layout(location = 1) in vec3 color;
    layout(location = 2) in vec2 texCoord;

    uniform mat4 model;
    out vec3 fragColor;
    out vec2 fragTexCoord;

    void main() {
        gl_Position = model * vec4(coord, 1.0);
        fragColor = color;
        fragTexCoord = texCoord;
    }
)";

const char* FragShaderSourceTex1 = R"(
    #version 330 core
    in vec3 fragColor;
    in vec2 fragTexCoord;
    out vec4 color;

    uniform sampler2D texture1;
    uniform float mixFactor;

    void main() {
        vec4 texColor = texture(texture1, fragTexCoord);
        color = mix(texColor, vec4(fragColor, 1.0), mixFactor);
    }
)";

// Текстура + текстура
const char* VertexShaderSourceTex2 = R"(
    #version 330 core
    layout(location = 0) in vec3 coord;
    layout(location = 1) in vec2 texCoord;

    uniform mat4 model;
    out vec2 fragTexCoord;

    void main() {
        gl_Position = model * vec4(coord, 1.0);
        fragTexCoord = texCoord;
    }
)";

const char* FragShaderSourceTex2 = R"(
    #version 330 core
    in vec2 fragTexCoord;
    out vec4 color;

    uniform sampler2D texture1;
    uniform sampler2D texture2;
    uniform float mixFactor;

    void main() {
        vec4 texColor1 = texture(texture1, fragTexCoord);
        vec4 texColor2 = texture(texture2, fragTexCoord);
        color = mix(texColor1, texColor2, mixFactor);
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
    // Создаем вершинный шейдер
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    if (figure_mode == 0 || figure_mode == 3)
        glShaderSource(vShader, 1, &VertexShaderSource, NULL);
    else if (figure_mode == 1)
        glShaderSource(vShader, 1, &VertexShaderSourceTex1, NULL);
    else
        glShaderSource(vShader, 1, &VertexShaderSourceTex2, NULL);
    glCompileShader(vShader);
    std::cout << "vertex shader:\n";
    ShaderLog(vShader);

    // Создаем фрагментный шейдер
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (figure_mode == 0 || figure_mode == 3)
        glShaderSource(fShader, 1, &FragShaderSource, NULL);
    else if (figure_mode == 1)
        glShaderSource(fShader, 1, &FragShaderSourceTex1, NULL);
    else
        glShaderSource(fShader, 1, &FragShaderSourceTex2, NULL);
    glCompileShader(fShader);
    std::cout << "fragment shader:\n";
    ShaderLog(fShader);

    // Создаем шейдерную программу
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

    checkOpenGLerror();
}

void InitBuffers()
{
    // Индексы для отрисовки фигур
    indices.clear();
    switch (figure_mode)
    {
    case 0: // Тетраэдр
        indices = {
            // Задняя грань
            0, 1, 2,
            // Левая грань
            0, 1, 3,
            // Нижняя грань
            1, 2, 3,
            // Верхнаяя грань
            0, 2, 3
        };
        break;
    case 1: // Куб
        indices = {
            // Задняя грань
            0, 1, 2,
            2, 3, 0,
            // Передяя грань
            4, 5, 6,
            6, 7, 4,
            // Левая грань
            8, 9, 10,
            10, 11, 8,
            // Правая грань
            12, 13, 14,
            14, 15, 12,
            // Верхняя грань
            16, 17, 18,
            18, 19, 16,
            // Нижняя грань
            20, 21, 22,
            22, 23, 20
        };
        break;
    case 2: // Куб с текстурой
        indices = {
            // Задняя грань
            0, 1, 2,
            2, 3, 0,
            // Передяя грань
            4, 5, 6,
            6, 7, 4,
            // Левая грань
            8, 9, 10,
            10, 11, 8,
            // Правая грань
            12, 13, 14,
            14, 15, 12,
            // Верхняя грань
            16, 17, 18,
            18, 19, 16,
            // Нижняя грань
            20, 21, 22,
            22, 23, 20
        };
        break;
    case 3: // Круг
        for (int i = 1; i <= segments; i++)
        {
            indices.push_back(4);
            indices.push_back(i + 4);
            indices.push_back(i + 5);
        }
        break;
    }

    // Создаем VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Создаем VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    if (figure_mode == 0 || figure_mode == 3)
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    else if (figure_mode == 1)
        glBufferData(GL_ARRAY_BUFFER, verticesForTex1.size() * sizeof(GLfloat), verticesForTex1.data(), GL_STATIC_DRAW);
    else
        glBufferData(GL_ARRAY_BUFFER, verticesForTex2.size() * sizeof(GLfloat), verticesForTex2.data(), GL_STATIC_DRAW);

    // Создаем EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    if (figure_mode == 0 || figure_mode == 3)
    {
        // Атрибут координат
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        // Атрибут цвета
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
    }
    else if (figure_mode == 1)
    {
        // Атрибут координат
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        // Атрибут цвета
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        // Атрибут текстурных координат
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);
    }
    else
    {
        // Атрибут координат
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        // Атрибут текстурных координат
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
    }

    glBindVertexArray(0);

    checkOpenGLerror();
}

void InitTexture()
{
    LoadTexture();
}

void Draw() {
    glUseProgram(Program);

    CreateTransformMatrix();
    modelLoc = glGetUniformLocation(Program, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, transformMatrix);

    if (figure_mode == 1 || figure_mode == 2)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glUniform1i(glGetUniformLocation(Program, "texture1"), 0);

        if (figure_mode == 2)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture2);
            glUniform1i(glGetUniformLocation(Program, "texture2"), 1);
        }
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    checkOpenGLerror();
}

void Release() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(Program);
}

void Init()
{
    // Инициализация шейдеров и буферов
    InitTexture();
    InitShader();
    InitBuffers();

    glEnable(GL_DEPTH_TEST);
}

int main()
{
    setlocale(LC_ALL, "ru");

    sf::Window window(sf::VideoMode(800, 800), "3D figures", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);

    GenerateCircleVertexes();

    glewInit();

    Init();

    while (window.isOpen())
    {
        glUniform1f(glGetUniformLocation(Program, "mixFactor"), mixFactor);
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
                    ResetAnglesAndScales();
                    InitShader();
                    InitBuffers();
                    break;
                case sf::Keyboard::Num2:
                    figure_mode = 1;
                    ResetAnglesAndScales(true);
                    InitShader();
                    InitBuffers();
                    break;
                case sf::Keyboard::Num3:
                    figure_mode = 2;
                    ResetAnglesAndScales(true);
                    InitShader();
                    InitBuffers();
                    break;
                case sf::Keyboard::Num4:
                    figure_mode = 3;
                    ResetAnglesAndScales();
                    InitShader();
                    InitBuffers();
                    break;
                case sf::Keyboard::A:
                    if (figure_mode < 3)
                        angleY -= 0.01;
                    else
                        scaleX -= 0.01;
                    CreateTransformMatrix();
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, transformMatrix);
                    break;
                case sf::Keyboard::D:
                    if (figure_mode < 3)
                        angleY += 0.01;
                    else
                        scaleX += 0.01;
                    CreateTransformMatrix();
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, transformMatrix);
                    break;
                case sf::Keyboard::W:
                    if (figure_mode < 3)
                        angleX -= 0.01;
                    else
                        scaleY += 0.01;
                    CreateTransformMatrix();
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, transformMatrix);
                    break;
                case sf::Keyboard::S:
                    if (figure_mode < 3)
                        angleX += 0.01;
                    else
                        scaleY -= 0.01;
                    CreateTransformMatrix();
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, transformMatrix);
                    break;
                case sf::Keyboard::Q:
                    if (figure_mode < 3)
                    {
                        angleZ -= 0.01;
                        CreateTransformMatrix();
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, transformMatrix);
                    }
                    break;
                case sf::Keyboard::E:
                    if (figure_mode < 3)
                    {
                        angleZ += 0.01;
                        CreateTransformMatrix();
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, transformMatrix);
                    }
                    break;
                case sf::Keyboard::Up:
                    if (figure_mode == 1 || figure_mode == 2)
                    {
                        mixFactor = std::clamp(mixFactor + 0.02f, 0.0f, 1.0f);
                        glUniform1f(glGetUniformLocation(Program, "mixFactor"), mixFactor);
                    }
                    break;
                case sf::Keyboard::Down:
                    if (figure_mode == 1 || figure_mode == 2)
                    {
                        mixFactor = std::clamp(mixFactor - 0.02f, 0.0f, 1.0f);
                        glUniform1f(glGetUniformLocation(Program, "mixFactor"), mixFactor);
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

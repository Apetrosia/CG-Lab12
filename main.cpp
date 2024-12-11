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

GLuint LoadTexture(const char* filePath)
{
    sf::Image img;
    if (!img.loadFromFile(filePath)) {
        std::cerr << "Error loading texture file: " << filePath << std::endl;
        return 0;
    }
    img.flipVertically();

    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.getSize().x, img.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());

    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    // std::cout << *(img.getPixelsPtr() + 1) << std::endl;

    checkOpenGLerror();

    return texture1;
}

std::vector<GLfloat> vertices = {
    // Задняя грань куба
    -0.4f, -0.4f,  0.4f,  0.0f, 0.0f, 0.0f,  1 - 0.0f, 0.0f, // Нижняя левая (тоже белый, т к черный не видно)
     0.4f, -0.4f,  0.4f,  0.0f, 0.0f, 1.0f,  1 - 1.0f, 0.0f, // Нижняя правая (синий)
     0.4f,  0.4f,  0.4f,  0.0f, 1.0f, 1.0f,  1 - 1.0f, 1.0f, // Верхняя правая (голубой)
    -0.4f,  0.4f,  0.4f,  0.0f, 1.0f, 0.0f,  1 - 0.0f, 1.0f, // Верхняя левая (зеленый)

    // Передняя грань куба
    -0.4f, -0.4f, -0.4f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, // Нижняя левая (красный)
     0.4f, -0.4f, -0.4f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Нижняя правая (малиновый)
     0.4f,  0.4f, -0.4f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f, // Верхняя правая (белый)
    -0.4f,  0.4f, -0.4f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f, // Верхняя левая (желтый)

    //Вершины тетраэдра
    -0.5f,  0.7f,  0.6f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,  // Верхняя (красный)
    -0.6f, -0.7f,  0.6f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, // Нижняя (зеленый)
     0.7f, -0.1f,  0.6f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,// Правая (синий)
     0.0f,  0.0f, -0.6f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,// Передняя (белый)
};

void GenerateCircleVertexes()
{
    vertices.push_back(0.0f); // Центр круга (X)
    vertices.push_back(0.0f); // Центр круга (Y)
    vertices.push_back(0.0f); // Центр круга (Z)
    vertices.push_back(1.0f); // Белый цвет (R)
    vertices.push_back(1.0f); // Белый цвет (G)
    vertices.push_back(1.0f); // Белый цвет (B)
    vertices.push_back(0.0f); // Текстурные координаты (костыль)
    vertices.push_back(0.0f); // Текстурные координаты (костыль)

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
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
    }
}

// Исходный код вершинного шейдера
const char* VertexShaderSource = R"(
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

// Исходный код фрагментного шейдера
const char* FragShaderSource = R"(
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
    glShaderSource(vShader, 1, &VertexShaderSource, NULL);
    glCompileShader(vShader);
    std::cout << "vertex shader:\n";
    ShaderLog(vShader);

    // Создаем фрагментный шейдер
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &FragShaderSource, NULL);
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

    //glUniform1i(glGetUniformLocation(Program, "texture1"), texture1);

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
            8, 9, 10,
            // Левая грань
            8, 9, 11,
            // Нижняя грань
            9, 10, 11,
            // Верхнаяя грань
            8, 10, 11
        };
        break;
    case 1: // Куб
        indices = {
            // Передняя грань
            0, 1, 2,
            2, 3, 0,
            // Задняя грань
            4, 5, 6,
            6, 7, 4,
            // Левая грань
            0, 3, 7,
            7, 4, 0,
            // Правая грань
            1, 2, 6,
            6, 5, 1,
            // Верхняя грань
            3, 2, 6,
            6, 7, 3,
            // Нижняя грань
            0, 1, 5,
            5, 4, 0
        };
        break;
    case 2: // Куб с текстурой
        indices = {
            // Передняя грань
            0, 1, 2,
            2, 3, 0,
            // Задняя грань
            4, 5, 6,
            6, 7, 4,
            // Левая грань
            0, 3, 7,
            7, 4, 0,
            // Правая грань
            1, 2, 6,
            6, 5, 1,
            // Верхняя грань
            3, 2, 6,
            6, 7, 3,
            // Нижняя грань
            0, 1, 5,
            5, 4, 0
        };
        break;
    case 3: // Круг
        for (int i = 1; i <= segments; i++)
        {
            indices.push_back(12);
            indices.push_back(i + 12);
            indices.push_back(i + 13);
        }
        break;
    }

    // Создаем VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Создаем VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    // Создаем EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // Атрибут координат
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // Атрибут цвета
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Атрибут текстурных координат
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // glUniform1i(glGetUniformLocation(Program, "texture1"), texture1);

    checkOpenGLerror();
}

void InitTexture()
{
    texture1 = LoadTexture("pic2.jpg");
}

void Draw() {
    glUseProgram(Program);

    CreateTransformMatrix();
    modelLoc = glGetUniformLocation(Program, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, transformMatrix);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //glActiveTexture(GL_TEXTURE0); // Выбор текстурного блока 0
    //glBindTexture(GL_TEXTURE_2D, texture1);

    //glUniform1i(glGetUniformLocation(Program, "texture1"), texture1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glUniform1i(glGetUniformLocation(Program, "texture1"), 0);

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
                    InitBuffers();
                    break;
                case sf::Keyboard::Num2:
                    figure_mode = 1;
                    ResetAnglesAndScales(true);
                    InitBuffers();
                    break;
                case sf::Keyboard::Num3:
                    figure_mode = 2;
                    ResetAnglesAndScales(true);
                    InitBuffers();
                    break;
                case sf::Keyboard::Num4:
                    figure_mode = 3;
                    ResetAnglesAndScales();
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

#include <SFML/Graphics.hpp>
#include <GL/glew.h>
#include <iostream>
#include <vector>

// IDs ��������� ���������, VAO, VBO � EBO
GLuint Program, VAO, VBO, EBO;

int figure_mode = 0;

// �������� ��� ���������� �������
const char* VertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec3 coord; // ���������� �������
    layout(location = 1) in vec3 color; // ���� �������

    out vec3 fragColor; // �������� ���� �� ����������� ������

    void main() {
        gl_Position = vec4(coord, 1.0); // ������ ��������� �������
        fragColor = color; // �������� ����
    }
)";

// �������� ��� ������������ �������
const char* FragShaderSource = R"(
    #version 330 core
    in vec3 fragColor; // �������� ���� �� ���������� �������
    out vec4 color;

    void main() {
        color = vec4(fragColor, 1.0); // ������ �������� ����
    }
)";

void ShaderLog(unsigned int shader) {
    int infologLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);

    if (infologLen > 1) {
        int charsWritten = 0;
        std::vector<char> infoLog(infologLen);
        glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog.data());
        std::cout << "InfoLog: " << infoLog.data() << std::endl;
    }
}

void InitShader() {
    // ������� ��������� ������
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &VertexShaderSource, NULL);
    glCompileShader(vShader);
    std::cout << "Vertex shader:\n";
    ShaderLog(vShader);

    // ������� ����������� ������
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &FragShaderSource, NULL);
    glCompileShader(fShader);
    std::cout << "Fragment shader:\n";
    ShaderLog(fShader);

    // ������� ��������� ���������
    Program = glCreateProgram();
    glAttachShader(Program, vShader);
    glAttachShader(Program, fShader);
    glLinkProgram(Program);

    int link_ok;
    glGetProgramiv(Program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        std::cout << "Error linking shaders.\n";
        return;
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);
}

void InitBuffers() {
    // ������� ����: ���������� (x, y, z) � ���� (r, g, b)
    GLfloat vertices[] = {
        // ������ ����� ����
        -0.4f, -0.4f,  0.6f,  1.0f, 1.0f, 1.0f, // ������ �����  (���� �����, � � ������ �� �����)
         0.6f, -0.4f,  0.6f,  0.0f, 0.0f, 1.0f, // ������ ������ (�����)
         0.6f,  0.6f,  0.6f,  0.0f, 1.0f, 1.0f, // ������� ������ (�������)
        -0.4f,  0.6f,  0.6f,  0.0f, 1.0f, 0.0f, // ������� �����  (�������)

        // �������� ����� ����
        -0.6f, -0.6f, -0.6f,  1.0f, 0.0f, 0.0f, // ������ ����� (�������)
         0.4f, -0.6f, -0.6f,  1.0f, 0.0f, 1.0f, // ������ ������ (���������)
         0.4f,  0.4f, -0.6f,  1.0f, 1.0f, 1.0f, // ������� ������ (�����)
        -0.6f,  0.4f, -0.6f,  1.0f, 1.0f, 0.0f, // ������� �����  (������)

        //������� ���������
        -0.6f,  0.6f, -0.6f,  1.0f, 0.0f, 0.0f, // ������� (�������)
        -0.6f, -0.6f, -0.6f,  0.0f, 1.0f, 0.0f, // ������ (�������)
         0.6f, -0.3f, -0.6f,  0.0f, 0.0f, 1.0f, // ����� (�����)
         0.3f,  0.0f,  0.6f,  1.0f, 1.0f, 1.0f, // �������� (�����)
    };

    // ������� ��� ��������� ������ ����
    GLuint indices[] = {
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

    // ������� VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // ������� VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // ������� EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
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
                    figure_mode = 1;
                    break;
                case sf::Keyboard::Num2:
                    figure_mode = 1;
                    break;
                case sf::Keyboard::Num3:
                    figure_mode = 2;
                    break;
                case sf::Keyboard::Num4:
                    figure_mode = 3;
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

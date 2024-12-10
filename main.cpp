#include <SFML/Graphics.hpp>
#include <GL/glew.h>
#include <iostream>
#include <vector>

// IDs шейдерной программы, VAO, VBO и EBO
GLuint Program, VAO, VBO, EBO;

int figure_mode = 0;

// Исходный код вершинного шейдера
const char* VertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec3 coord; // Координаты вершины
    layout(location = 1) in vec3 color; // Цвет вершины

    out vec3 fragColor; // Передаем цвет во фрагментный шейдер

    void main() {
        gl_Position = vec4(coord, 1.0); // Задаем положение вершины
        fragColor = color; // Передаем цвет
    }
)";

// Исходный код фрагментного шейдера
const char* FragShaderSource = R"(
    #version 330 core
    in vec3 fragColor; // Получаем цвет из вершинного шейдера
    out vec4 color;

    void main() {
        color = vec4(fragColor, 1.0); // Задаем итоговый цвет
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
    // Создаем вершинный шейдер
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &VertexShaderSource, NULL);
    glCompileShader(vShader);
    std::cout << "Vertex shader:\n";
    ShaderLog(vShader);

    // Создаем фрагментный шейдер
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &FragShaderSource, NULL);
    glCompileShader(fShader);
    std::cout << "Fragment shader:\n";
    ShaderLog(fShader);

    // Создаем шейдерную программу
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
    // Вершины куба: координаты (x, y, z) и цвет (r, g, b)
    GLfloat vertices[] = {
        // Задняя грань куба
        -0.4f, -0.4f,  0.6f,  1.0f, 1.0f, 1.0f, // Нижняя левая  (тоже белый, т к черный не видно)
         0.6f, -0.4f,  0.6f,  0.0f, 0.0f, 1.0f, // Нижняя правая (синий)
         0.6f,  0.6f,  0.6f,  0.0f, 1.0f, 1.0f, // Верхняя правая (голубой)
        -0.4f,  0.6f,  0.6f,  0.0f, 1.0f, 0.0f, // Верхняя левая  (зеленый)

        // Передняя грань куба
        -0.6f, -0.6f, -0.6f,  1.0f, 0.0f, 0.0f, // Нижняя левая (красный)
         0.4f, -0.6f, -0.6f,  1.0f, 0.0f, 1.0f, // Нижняя правая (малиновый)
         0.4f,  0.4f, -0.6f,  1.0f, 1.0f, 1.0f, // Верхняя правая (белый)
        -0.6f,  0.4f, -0.6f,  1.0f, 1.0f, 0.0f, // Верхняя левая  (желтый)

        //Вершины тетраэдра
        -0.6f,  0.6f, -0.6f,  1.0f, 0.0f, 0.0f, // Верхняя (красный)
        -0.6f, -0.6f, -0.6f,  0.0f, 1.0f, 0.0f, // Нижняя (зеленый)
         0.6f, -0.3f, -0.6f,  0.0f, 0.0f, 1.0f, // Левая (синий)
         0.3f,  0.0f,  0.6f,  1.0f, 1.0f, 1.0f, // Передняя (белый)
    };

    // Индексы для отрисовки граней куба
    GLuint indices[] = {
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

    // Создаем VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Создаем VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Создаем EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Атрибут координат
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // Атрибут цвета
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
    // Инициализация шейдеров и буферов
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

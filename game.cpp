#include "game.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <pthread.h>
#include <thread>
#include <random>
#include <vector>

struct game_state state;

bool between(float a, float b, float c) {
    return b >= a && b <= c;
}

void Square::move(float delta_x, float delta_y) {
    for (int i = 0; i < len_vertices; i++) {
        // these are the x coordinates
        if (i % 3 == 0) {
            vertices[i] += delta_x;
        }

        // these are the y coordinates
        else if (i % 3 == 1) {
            vertices[i] += delta_y;
        }
    }
}


float Apple::rand_float() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-0.95f, 0.95f);

    float value = dist(gen);
    return value;
}


void Apple::draw() {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glDrawElements(GL_TRIANGLES, len_indices, GL_UNSIGNED_INT, 0);
}

void Apple::reset_vertices() {
    float x_coord = Apple::rand_float();
    float y_coord = Apple::rand_float();

    vertices[0] = x_coord;
    vertices[1] = y_coord;
    vertices[2] = 0.0f;

    vertices[3] = x_coord + SQUARE_SIDE_LEN;
    vertices[4] = y_coord;
    vertices[5] = 0.0f;

    vertices[6] = x_coord;
    vertices[7] = y_coord + SQUARE_SIDE_LEN;
    vertices[8] = 0.0f;

    vertices[9]  = x_coord + SQUARE_SIDE_LEN;
    vertices[10] = y_coord + SQUARE_SIDE_LEN;
    vertices[11] = 0.0f;

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, len_vertices * sizeof(float), vertices, GL_STATIC_DRAW);
}

Game::Game() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 800, "Snake", NULL, NULL);
    if (window == NULL) {
        std::cout << "ERROR: Failed to create a window\n";
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD\n";
        glfwTerminate();
        exit(1);
    }


    glViewport(0, 0, 800, 800);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    std::string vert_shader_src = load_shader_src(VERT_SHADER_PATH);
    std::string frag_shader_src = load_shader_src(FRAG_SHADER_PATH);

    const char* vert_shader_src_cstr = vert_shader_src.c_str();
    const char* frag_shader_src_cstr = frag_shader_src.c_str();

    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vert_shader, 1, &vert_shader_src_cstr, NULL);
    glShaderSource(frag_shader, 1, &frag_shader_src_cstr, NULL);

    glCompileShader(vert_shader);
    glCompileShader(frag_shader);

    if (check_shader_error(vert_shader)) {
        glfwTerminate();
        exit(1);
    }
    if (check_shader_error(frag_shader)) {
        glfwTerminate();
        exit(1);
    }

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vert_shader);
    glAttachShader(shader_program, frag_shader);
    glLinkProgram(shader_program);
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    snake = new Snake(0.0f, 0.0f);

    snake->num_buffers = snake->length;
    snake->VAOs = std::vector<GLuint>(snake->num_buffers);
    snake->VBOs = std::vector<GLuint>(snake->num_buffers);
    snake->EBOs = std::vector<GLuint>(snake->num_buffers);

    snake->gen_vertex_objs(snake->VAOs, snake->VBOs, snake->EBOs);
    float apple_x = Apple::rand_float();
    float apple_y = Apple::rand_float();
    apple = new Apple(apple_x, apple_y);

    my_id = snake->id;

    update_game_state(snake, apple);

    connect_to_server();
}

void Game::process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        if (snake->direction == 'w') {
            return;
        }

        snake->direction = 'e';

    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        if (snake->direction == 'e') {
            return;
        }

        snake->direction = 'w';
    }
    else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        if (snake->direction == 's') {
            return;
        }

        snake->direction = 'n';

    }
    else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        if (snake->direction == 'n') {
            return;
        }

        snake->direction = 's';

    }
}

void Game::launch() {
    while (!glfwWindowShouldClose(window)) {
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);

        apple->draw();
        snake->draw();
        snake->move();

        if (snake->check_collision(apple)) {
            apple->reset_vertices();
            snake->grow();
        }

        update_game_state(snake, apple);

        glfwSwapBuffers(window);
        glfwPollEvents();

        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }

    pthread_cancel(sender_thrd);
    pthread_cancel(listener_thrd);
}

std::string Game::load_shader_src(std::string path) {
    std::ifstream stream(path);
    if (!stream.is_open()) {
        std::cout << "ERROR: Failed to load shader " << path << '\n';
        exit(1);
    }
    std::string src;
    std::string line;

    while (!stream.eof()) {
        std::getline(stream, line);
        src.append(line);
        src.append("\n");
    }
    stream.close();
    return src;
}

bool Game::check_shader_error(GLuint shader) {
    int32_t success;
    char infoLog[512];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR: Shader Compilation Failed.\n" << infoLog << '\n';
        return true;
    }
    return false;
}

#include "game.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <thread>
#include <vector>
#include <random>

void Square::move(float delta_x, float delta_y) {
    x += delta_x;
    y += delta_y;

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
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    float value = dist(gen);
    return value;
}


void Apple::draw() {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glDrawElements(GL_TRIANGLES, len_indices, GL_UNSIGNED_INT, 0);
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
        snake->draw(VAOs, VBOs, EBOs);
        snake->move();
        
        glfwSwapBuffers(window);
        glfwPollEvents();

        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
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

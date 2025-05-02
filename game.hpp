#ifndef GAME_HPP
#define GAME_HPP

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#define SQUARE_SIDE_LEN 0.03f

// Struct declarations to store game state (will be converted to json)
struct coord {
    float x;
    float y;
};

struct snake {
    std::vector<struct coord> coords; 
    uint32_t id;
};


struct game_state {
    struct coord apple_location;
    std::vector<struct snake> players;
    uint32_t num_players;
};


class Square {
public:
    float* vertices;
    uint32_t* indices;
    uint32_t len_vertices, len_indices, x, y;

    Square(float x_coord, float y_coord) {
        len_vertices = 12;
        len_indices  = 6;

        x = x_coord;
        y = y_coord;

        vertices = new float[len_vertices];
        indices  = new uint32_t[len_indices];

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

        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 2;

        indices[3] = 1;
        indices[4] = 2;
        indices[5] = 3;

    }

    ~Square() {
        delete[] vertices;
        delete[] indices;
    }

    void move(float delta_x, float delta_y);
};


class Apple : public Square {
public:
    GLuint VAO, VBO, EBO;

    Apple(float x_coord, float y_coord) : Square(x_coord, y_coord) {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, len_vertices * sizeof(float), vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, len_indices * sizeof(float), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    ~Apple() {
        glDeleteBuffers(1, &EBO);
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);

        delete[] vertices;
        delete[] indices;
    }
    
    static float rand_float();

    // Draws the vertices stored in VAO, VBO, and EBO
    void draw();

    // Randomly generates a new pair of coordinates and resets the vertices
    void reset_vertices();
};


class Snake {
private:
    const uint32_t init_snake_len = 3;

public:
    std::vector<Square*> squares;
    uint32_t length;
    char direction;

    Snake(float x, float y) {
        squares = std::vector<Square*>();
        for (int i = 0; i < init_snake_len; i++) {
            Square* square = new Square(x + i * SQUARE_SIDE_LEN, y);
            squares.push_back(square);
        }
        length = init_snake_len;
        direction = 'w';
    }

    ~Snake() {
        for (int i = 0; i < squares.size(); i++) {
            delete squares[i];
        } 
    }

    void print_snake() {
        for (int i = 0; i < squares.size(); i++) {
            std::cout << "(" << squares[i]->vertices[0] << ", " << squares[i]->vertices[1] << "), ";
        }
        std::cout << '\n';
    }

    // This function doesn't work
    bool equals(Snake* other);

    // Generates VAOs and VBOs and EBOs for the snake (1 VAO, VBO, and EBO for each Square) and writes the initial data
    void gen_vertex_objs(std::vector<GLuint> &VAOs, std::vector<GLuint> &VBOs, std::vector<GLuint> &EBOs);

    // Writes data to the VAO and VBO
    void draw(std::vector<GLuint> VAOs, std::vector<GLuint> VBOs, std::vector<GLuint> EBOs);

    // Moves the snake one square forward in it's current direction
    void move();

    bool check_collision();
};


// Class to represent the game
class Game {
private:
    std::vector<GLuint> VAOs;
    std::vector<GLuint> VBOs;
    std::vector<GLuint> EBOs;
    uint32_t num_buffers;
    GLFWwindow* window;
    GLuint shader_program;

    Snake* snake;
    Apple* apple;
    struct game_state state;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }

    void process_input(GLFWwindow* window);

public:
    Game() {
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


        std::string vert_shader_src = load_shader_src("glsl/vert_shader.glsl");
        std::string frag_shader_src = load_shader_src("glsl/frag_shader.glsl");

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

        num_buffers = snake->length;
        VAOs = std::vector<GLuint>(num_buffers);
        VBOs = std::vector<GLuint>(num_buffers);
        EBOs = std::vector<GLuint>(num_buffers);

        snake->gen_vertex_objs(VAOs, VBOs, EBOs);
        apple = new Apple(Apple::rand_float(), Apple::rand_float());
    }

    ~Game() {
        glfwTerminate();
    }

    std::string load_shader_src(std::string path);

    bool check_shader_error(uint32_t shader);

    void launch();
    

// Functions to handle networking:
    
    // Initializes a connection with the server
    int connect_to_server();
    
    // Writes the current game state to the server
    int write_state();

    // Reads the game state from the server and updates its game state
    int read_state();
};

#endif

#ifndef GAME_HPP
#define GAME_HPP

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "cjson.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#define SQUARE_SIDE_LEN 0.03f
#define VERT_SHADER_PATH "/home/sypalj/snake/glsl/vert_shader.glsl"
#define FRAG_SHADER_PATH "/home/sypalj/snake/glsl/frag_shader.glsl"

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

// FUNCTIONS TO TRANSLATE TO/FROM CJSON COMPATIBLE TYPES

Map* to_map(struct game_state state);

struct game_state from_map(Map* state);


// Returns true if b is on the interval [a,c]
bool between(float a, float b, float c);


class Square {
public:
    float* vertices;
    uint32_t* indices;
    uint32_t len_vertices, len_indices;

    Square(float x_coord, float y_coord) {
        len_vertices = 12;
        len_indices  = 6;

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

    float x() {
        return vertices[0];
    }

    float y() {
        return vertices[1];
    }
};


class Apple : public Square {
public:
    GLuint VAO, VBO, color_VBO, EBO;
    float color_array[3];

    Apple(float x_coord, float y_coord) : Square(x_coord, y_coord) {
        color_array[0] = 0.0f;
        color_array[1] = 0.0f;
        color_array[2] = 1.0f;

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

        glGenBuffers(1, &color_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, color_VBO);
        glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float), color_array, GL_STATIC_DRAW);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
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
    const uint32_t init_snake_len = 1;

public:
    std::vector<GLuint> VAOs;
    std::vector<GLuint> VBOs;
    std::vector<GLuint> EBOs;
    uint32_t num_buffers;

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
    void draw();

    // Moves the snake one square forward in it's current direction
    void move();

    // Returns true if the snake head overlaps with the square
    bool check_collision(Square* square);

    // Adds one square to the tail of the snake
    void grow();
};


// Class to represent the game
class Game {
private:
    GLFWwindow* window;
    GLuint shader_program;

    Snake* snake;
    Apple* apple;
    struct game_state state;

    pthread_t listener_thrd, sender_thrd;

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
        apple = new Apple(Apple::rand_float(), Apple::rand_float());

        connect_to_server();
    }

    ~Game() {
        glfwTerminate();
    }

    std::string load_shader_src(std::string path);

    bool check_shader_error(uint32_t shader);

    void launch();
    

// Functions to handle networking:
    
    // Initializes a connection with the server
    void connect_to_server();

    static void* listener(void* args);

    static void* sender(void* args);

    std::string serialize_state(struct game_state state);

    struct game_state deserialize_state(std::string json_data);
    
    // Writes the current game state to the server
    int write_state();

    // Reads the game state from the server and updates its game state
    int read_state();
};

#endif

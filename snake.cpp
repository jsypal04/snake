#include "game.hpp"
#include <cstdint>

void Snake::gen_vertex_objs(std::vector<GLuint> &VAOs, std::vector<GLuint> &VBOs, std::vector<GLuint> &EBOs) {
    for (int i = 0; i < VAOs.size(); i++) {
        glGenVertexArrays(1, &VAOs[i]);
        glBindVertexArray(VAOs[i]);

        glGenBuffers(1, &VBOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);

        uint32_t num_verts = squares[i]->len_vertices;
        float* verts  = squares[i]->vertices;
        glBufferData(GL_ARRAY_BUFFER, num_verts * sizeof(float), verts, GL_DYNAMIC_COPY);

        glGenBuffers(1, &EBOs[i]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[i]);

        uint32_t num_indices = squares[i]->len_indices;
        uint32_t* indices    = squares[i]->indices;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(uint32_t), indices, GL_DYNAMIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }  
}

void Snake::draw() {
    for (int i = 0; i < VAOs.size(); i++) {
        glBindVertexArray(VAOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
        
        int num_verts = squares[i]->len_vertices;
        float* verts = squares[i]->vertices;
        glBufferData(GL_ARRAY_BUFFER, num_verts * sizeof(float), verts, GL_DYNAMIC_DRAW);

        glDrawElements(GL_TRIANGLES, squares[i]->len_indices, GL_UNSIGNED_INT, 0);
    }
}

bool Snake::equals(Snake* other) {
    if (squares.size() != other->squares.size()) {
        return false;
    }

    for (int i = 0; i < squares.size(); i++) {
      if (other->squares[i]->vertices[0] != squares[i]->vertices[0] || squares[i]->vertices[1] != other->squares[i]->vertices[1]) {
        return false;
      }
    }

    return true;
}

void Snake::move() {
    Square* tail = squares.back();
    squares.pop_back();

    Square* head = squares.front();
    float x = head->vertices[0];
    float y = head->vertices[1];

    float delta_x = x - tail->vertices[0];
    float delta_y = y - tail->vertices[1];
    switch (direction) {
        case 'w': {
            delta_x -= SQUARE_SIDE_LEN;
            break;
        }
        case 'e': {
            delta_x += SQUARE_SIDE_LEN;
            break;
        }
        case 'n': {
            delta_y += SQUARE_SIDE_LEN;
            break;
        }
        case 's': {
            delta_y -= SQUARE_SIDE_LEN;
            break;
        }
        default: {
            std::cout << "ERROR: unknown direction " << direction << " in Snake::move()\n";
            exit(1);
        }
    }
    tail->move(delta_x, delta_y);
    squares.insert(squares.begin(), tail);
}

bool Snake::check_collision(Square* square) {
    if (!between(square->x(), squares.front()->x(), square->x() + SQUARE_SIDE_LEN) && 
        !between(square->x(), squares.front()->x() + SQUARE_SIDE_LEN, square->x() + SQUARE_SIDE_LEN)) {
        return false;
    }

    if (!between(square->y(), squares.front()->y(), square->y() + SQUARE_SIDE_LEN) &&
        !between(square->y(), squares.front()->y() + SQUARE_SIDE_LEN, square->y() + SQUARE_SIDE_LEN)) {
        return false;
    }
    return true;
}

void Snake::grow() {
    GLuint new_VAO, new_VBO, new_EBO;
    float new_x, new_y;
    Square* new_square;

    // Compute the new coordinates;
    switch (direction) {
        case 'n': {
            new_x = squares.front()->x();
            new_y = squares.front()->y() + SQUARE_SIDE_LEN;
            break;
        }
        case 's': {
            new_x = squares.front()->x();
            new_y = squares.front()->y() - SQUARE_SIDE_LEN;
            break;
        }
        case 'e': {
            new_x = squares.front()->x() + SQUARE_SIDE_LEN;
            new_y = squares.front()->y();
            break;
        }
        case 'w': {
            new_x = squares.front()->x() - SQUARE_SIDE_LEN;
            new_y = squares.front()->y();
            break;
        }
    }

    new_square = new Square(new_x, new_y);
    squares.insert(squares.begin(), new_square);

    glGenVertexArrays(1, &new_VAO);
    glGenBuffers(1, &new_VBO);
    glGenBuffers(1, &new_EBO);

    glBindVertexArray(new_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, new_VBO);
    glBufferData(GL_ARRAY_BUFFER, new_square->len_vertices * sizeof(float), new_square->vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, new_square->len_indices * sizeof(uint32_t), new_square->indices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    VAOs.push_back(new_VAO);
    VBOs.push_back(new_VBO);
    EBOs.push_back(new_EBO);
}

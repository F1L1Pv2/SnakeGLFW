#include <glad/glad.h>
#include <cstdlib>
#include <GLFW/glfw3.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinUser.h>

#include <string>
#include <sstream>


#include <game_state.h>
#include <stdint.h>

#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <graphic_math.h>
#include <random>
#include <time.h>

using namespace std;

#define INITIAL_RESOLUTION_WIDTH 640
#define INITIAL_RESOLUTION_HEIGHT 480
#define GRID_SIZE 10

void error(string why) {
    MessageBox(NULL, why.c_str(), "Snake Error", MB_OK);
    exit(1);
}

enum GRID_CELL {
    BLANK,
    FRUIT,
    PLAYER_TAIL,
    PLAYER_HEAD_RIGHT,
    PLAYER_HEAD_UP,
    PLAYER_HEAD_LEFT,
    PLAYER_HEAD_DOWN,
};

GRID_CELL grid[GRID_SIZE * GRID_SIZE];



void init();
void draw();
void update();

uint32_t window_width = INITIAL_RESOLUTION_WIDTH, window_height = INITIAL_RESOLUTION_HEIGHT;
uint8_t proj_loc;
uint8_t global_scale_loc;
vector2 global_scale(1, 1);
mat4 proj = mat4::identity();

void reset_global_scale() {
    vector2 reset(1, 1);
    glUniform2fv(global_scale_loc, 1, reset.value_ptr());
}

void update_global_scale(vector2 new_scale) {
    glUniform2fv(global_scale_loc, 1, global_scale.value_ptr());
}

void update_proj() {
    
    vector2 proj_scale(10, 10);

    if (window_width > window_height) {
        float aspect_ratio = (float)window_width / window_height;
        proj = mat4::ortho2D(-aspect_ratio / 2 * proj_scale.x, aspect_ratio / 2 * proj_scale.x, 0.5 * proj_scale.y, -0.5 * proj_scale.y);
    }
    else {
        float aspect_ratio = (float)window_height / window_width;
        proj = mat4::ortho2D(-0.5*proj_scale.x, 0.5*proj_scale.x, aspect_ratio / 2*proj_scale.y, -aspect_ratio / 2 * proj_scale.y);
    }
    

    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, proj.value_ptr());
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    window_width = width;
    window_height = height;
    //aspect_ratio = (float)window_height / window_width;
    update_proj();
    draw();
}


GLFWwindow* window;
uint32_t vao;
uint32_t vbo, ibo;
uint32_t instance_buffer;
uint8_t program;

vector<vertex> vertices = {
        vertex(vector2(-0.5, -0.5), vector2(0.0f,0.0f)),
        vertex(vector2( 0.5, -0.5), vector2(1.0f,0.0f)),
        vertex(vector2( 0.5,  0.5), vector2(1.0f,1.0f)),
        vertex(vector2(-0.5,  0.5), vector2(0.0f,1.0f)),
};

vector<char> indices = {
    0,1,3,
    1,2,3,
};


struct instance {
    vector2 position;
    vector2 scale;
    int32_t textureID;

    instance(vector2 position, vector2 scale, int32_t textureID) : position(position), scale(scale), textureID(textureID) {}
};

vector<instance> instances = {};

vector2 grid_to_space(vector2 cordinate) {
    return vector2(cordinate.x - GRID_SIZE / 2 + 0.5, cordinate.y - GRID_SIZE / 2 + 0.5);
}

static inline uint32_t grid_to_index(vector2 cordinate) {
    return cordinate.y * GRID_SIZE + cordinate.x;
}

vector2 generate_random_cordinates();

void clear_grid() {
    for (int32_t i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        grid[i] = BLANK;
    }
}

double old_time;
double dt = 0;

void draw_grid() {
    instances.clear();

    for (int32_t i = 0; i < GRID_SIZE; i++) {
        for (int32_t j = 0; j < GRID_SIZE; j++) {

            vector2 position(grid_to_space(vector2(i, j)));
            vector2 scale(1, 1);
            uint32_t texture = grid[grid_to_index(vector2(i, j))];

            if (texture == FRUIT) {
                instances.push_back(instance(position, scale, 0));
            }

            instances.push_back(instance(position, scale, texture));
        }
    }
}



void key_fun_callback(GLFWwindow* window, int key, int scancode, int action, int mods);


int main(void)
{

    srand(time(0));

    draw_grid();
    
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    /* Initialize the library */
    if (!glfwInit())
        error("Couldn't initialize GLFW");

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(INITIAL_RESOLUTION_WIDTH, INITIAL_RESOLUTION_HEIGHT, "Snake", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        error("Couldn't create window");
    }


    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        error("Couldn't initialize glad");

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_fun_callback);
    glfwSwapInterval(0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const char* vertex_shader_src = R"(
        #version 400 core

        layout(location = 0) in vec2 v_position;
        layout(location = 1) in vec2 v_uv;

        layout(location = 2) in vec2 v_instance_offset;
        
        layout(location = 3) in vec2 v_scale;

        layout(location = 4) in int v_texture_id;

        uniform mat4 proj;

        uniform vec2 global_scale = vec2(1.0f,1.0f);

        out vec2 uv;
        flat out int texture_id;

        void main(){
            uv = v_uv;
            texture_id = v_texture_id;
            gl_Position = proj * vec4(global_scale*v_scale*(v_position+v_instance_offset),0.0f,1.0f);
        }
    )";

    const char* fragment_shader_src = R"(
        #version 400 core

        in vec2 uv;
        flat in int texture_id;

        uniform sampler2D ourTexture;

        out vec4 color;

        void main(){


            if(texture_id>=0){
                int grid_size = 4;
                vec2 new_uv;
                new_uv.x = ceil((uv.x / grid_size + (1.0 / grid_size) * (texture_id / grid_size))*100000.0)/100000.0;
                new_uv.y = ceil((uv.y / grid_size + (1.0 / grid_size) * (texture_id % grid_size))*100000.0)/100000.0;

                color = texture(ourTexture, new_uv);
            }else{
                color = vec4(1.0f,1.0f,1.0f,1.0f);
            }

        }
    )";

    program = glCreateProgram();

    uint8_t vert_shader = glCreateShader(GL_VERTEX_SHADER);
    uint8_t frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vert_shader, 1, &vertex_shader_src, nullptr);
    glShaderSource(frag_shader, 1, &fragment_shader_src, nullptr);

    glCompileShader(vert_shader);

    int32_t isCompiled = 0;
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        int32_t maxLength = 0;
        glGetShaderiv(vert_shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<char> errorLog(maxLength);
        glGetShaderInfoLog(vert_shader, maxLength, &maxLength, &errorLog[0]);

        error(errorLog.data());
    }

    glCompileShader(frag_shader);

    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        int32_t maxLength = 0;
        glGetShaderiv(frag_shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<char> errorLog(maxLength);
        glGetShaderInfoLog(frag_shader, maxLength, &maxLength, &errorLog[0]);

        error(errorLog.data());
    }

    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);

    glUseProgram(program);
    proj_loc = glGetUniformLocation(program, "proj");
    global_scale_loc = glGetUniformLocation(program, "global_scale");
    update_proj();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);


    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);
    glGenBuffers(1, &instance_buffer);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex,position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, uv));

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(char), indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, instance_buffer);
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(instance), (void*)offsetof(instance,position));
    glVertexAttribDivisor(2, 1);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(instance), (void*)offsetof(instance, scale));
    glVertexAttribDivisor(3, 1);

    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 1, GL_INT, sizeof(instance), (void*)offsetof(instance, textureID));
    glVertexAttribDivisor(4, 1);

    glBufferData(GL_ARRAY_BUFFER, 1024 * sizeof(instance), NULL, GL_DYNAMIC_DRAW);

    // ---------------------------------------------------------------------------------------------------
    // 
    // 
    //load texture

    int width, height, channels;

    stbi_set_flip_vertically_on_load(true);

    uint8_t* texture_data = stbi_load("assets/textures/glfwsnek.png", &width, &height, &channels, 4);
    if (texture_data == NULL)
        error("Couldn't load texture");

    uint32_t texture;
    glGenTextures(1, &texture);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(texture_data);

    init();

    old_time = glfwGetTime();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        //calculating dt
        double current_time = glfwGetTime();

        dt = current_time - old_time;

        old_time = current_time;


        update();

        draw();

        /* Poll for and process events */
    }

    glfwTerminate();
    return 0;
}

vector2 offset(0, 0);

void draw() {
    /* Render here */
    draw_grid();

    glClear(GL_COLOR_BUFFER_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, instance_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instances.size() * sizeof(instance), instances.data());

    glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_BYTE, NULL, instances.size());

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

}

vector2 player_vel(1, 0);
vector2 player_coordinates(GRID_SIZE/2, GRID_SIZE/2);
vector2 fruit_cordinates(0, 0);
GRID_CELL player_texture = PLAYER_HEAD_RIGHT;

#define WAIT_TIME 0.25f
double acumulator = WAIT_TIME;

#define VEL_LEFT vector2(-1, 0)
#define VEL_RIGHT vector2(1, 0)
#define VEL_UP vector2(0, 1)
#define VEL_DOWN vector2(0, -1)

bool moved = false;

void key_fun_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    
    switch (key) {
    case('A'): if(!moved && player_vel!=VEL_RIGHT){moved = true; player_vel = VEL_LEFT; player_texture = PLAYER_HEAD_LEFT;} break;
    case('D'): if(!moved && player_vel!=VEL_LEFT) {moved = true; player_vel = VEL_RIGHT; player_texture = PLAYER_HEAD_RIGHT;} break;
    case('W'): if(!moved && player_vel!=VEL_DOWN) {moved = true; player_vel = VEL_UP; player_texture = PLAYER_HEAD_UP;} break;
    case('S'): if(!moved && player_vel != VEL_UP) {moved = true; player_vel = VEL_DOWN; player_texture = PLAYER_HEAD_DOWN; } break;
    default: break;
    }
    
}

void init() {
    player_coordinates = generate_random_cordinates();
    fruit_cordinates = generate_random_cordinates();
}



struct tail {
    vector2 position;
    tail(vector2 position): position(position) {}
};

vector<tail> player_tail;

vector2 generate_random_cordinates() {
    
    vector2 pos(rand() % GRID_SIZE, rand() % GRID_SIZE);
    
    while (grid[grid_to_index(pos)] != BLANK) {
        pos = vector2(rand() % GRID_SIZE, rand() % GRID_SIZE);
    }
    
    return pos;
}

void update_tail() {
    if (player_tail.size() == 0) return;
    for (uint32_t i = player_tail.size()-1; i >= 0; i--) {
        if (i == 0) {
            player_tail[i].position = player_coordinates;
            break;
        }
        player_tail[i].position = player_tail[i - 1].position;
    }
}

bool check_die() {
    for (auto& it : player_tail) {
        if (player_coordinates == it.position) {
            return true;
        }
    }
    return false;
}

void update() {

    acumulator += dt;

    if(acumulator > WAIT_TIME){
        moved = false;
        clear_grid();

        acumulator = 0.0f;
        update_tail();
        
        player_coordinates =  player_coordinates + player_vel;
        if (player_coordinates.x > GRID_SIZE-1) player_coordinates.x = 0;
        if (player_coordinates.y > GRID_SIZE-1) player_coordinates.y = 0;
        if (player_coordinates.x < 0) player_coordinates.x = GRID_SIZE - 1;
        if (player_coordinates.y < 0) player_coordinates.y = GRID_SIZE - 1;

        grid[grid_to_index(fruit_cordinates)] = FRUIT;
        grid[grid_to_index(player_coordinates)] = player_texture;


        for (auto& it : player_tail) {
            grid[grid_to_index(it.position)] = PLAYER_TAIL;
        }

        if (player_coordinates == fruit_cordinates) {
            fruit_cordinates = generate_random_cordinates();
            player_tail.push_back(tail(vector2(0,0)));
        }

        if (check_die()) {
            player_tail.clear();
            clear_grid();
            player_coordinates = generate_random_cordinates();
            fruit_cordinates = generate_random_cordinates();
            acumulator = WAIT_TIME * 2;
        }
    }

}
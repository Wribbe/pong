#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define UNUSED(x) (void) x

#define SIZE(x) sizeof(x)/sizeof(x[0])

typedef GLfloat m4[4][4];


/* Enumerate unique objects. */
enum {
    ID_RIGHT_PADDLE,
    ID_LEFT_PADDLE,
    ID_BALL,
    ID_NUM,
};


typedef struct Item_Data {
    GLint width;
    GLint height;
    GLint speed;
    GLuint id;
    GLuint offset;
} Item_Data;


typedef struct Environment_Data {
    GLint width;
    GLint height;
    GLfloat delta_width;
    GLfloat delta_height;
} Environment_Data;


typedef struct Event_Data {
    GLFWwindow * window;
    m4 * transformation_matrices;
    Item_Data * items;
} Event_Data;


bool map_keys[1024];

void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods) {

    UNUSED(scancode);
    UNUSED(mods);
    UNUSED(window);

    if (action == GLFW_PRESS) {
        map_keys[key] = true;
    } else if (action == GLFW_RELEASE && map_keys[key]) {
        map_keys[key] = false;
    }
}


void error(const char * message, bool fatal) {
    fprintf(stderr, "ERROR: %s", message);
    if (fatal) {
        fprintf(stderr, "ERROR: Shutting down.\n");
        exit(EXIT_FAILURE);
    }
}


void react_to_events(Event_Data event_data, Item_Data * items, Environment_Data env) {

    GLFWwindow * window = event_data.window;
    m4 * transformation_matrices = event_data.transformation_matrices;

    /* Close window on ESC. */
    if (map_keys[GLFW_KEY_ESCAPE]) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    /* Get item objects for left and right paddle. */
    Item_Data item_right = items[ID_RIGHT_PADDLE];
    Item_Data item_left = items[ID_LEFT_PADDLE];

    /* Get delta height from environment. */
    GLfloat delta_height = env.delta_height;

    /* Get pixel speeds for paddles. */
    GLint speed_right_pixel = item_right.speed;
    GLint speed_left_pixel = item_left.speed;

    /* Convert pixel speed.to float speed for both paddles. */
    GLfloat speed_right_float = speed_right_pixel * delta_height;
    GLfloat speed_left_float = speed_left_pixel * delta_height;

    /* Grab pointers to height values for right and left pad. */
    GLfloat * ptr_pos_right = &transformation_matrices[ID_RIGHT_PADDLE][1][3];
    GLfloat * ptr_pos_left = &transformation_matrices[ID_LEFT_PADDLE][1][3];

    /* Calculate current positions in pixels. */
    GLint pos_right = *ptr_pos_right/delta_height;
    GLint pos_left = *ptr_pos_left/delta_height;

    /* Create variables for storing the next height value. */
    GLint next_right_pos, next_left_pos;

    /* Move right paddle up and down with arrow keys. */
    if (map_keys[GLFW_KEY_UP]) {
        /* Calculate the next position based on pixel movement. */
        next_right_pos = pos_right + speed_right_pixel;
        /* Add the height of the paddle and check bounds. */
        GLint top_of_right = next_right_pos+item_right.height/2;
        if (top_of_right < env.height/2) {
            *ptr_pos_right += speed_right_float;
        } else {
            *ptr_pos_right = 1.0f-item_right.height/2*delta_height;
        }
    } else if (map_keys[GLFW_KEY_DOWN]) {
        /* Calculate the next position based on pixel movement. */
        next_right_pos = pos_right - speed_right_pixel;
        /* Add the height of the paddle and check bounds. */
        GLint bottom_of_right = next_right_pos-item_right.height/2;
        if (bottom_of_right > -env.height/2) {
            *ptr_pos_right -= speed_right_float;
        } else {
            *ptr_pos_right = -1.0f+item_right.height/2*delta_height;
        }
    }
}


const GLchar * source_vertex_shader = \
    "#version 330 core\n"
    "layout (location=0) in vec3 position;\n"
    "uniform mat4 transform;\n"
    "\n"
    "void main() {"
    "   gl_Position = transform * vec4(position, 1.0f);\n"
    "}\n";


const GLchar * source_fragment_shader = \
    "#version 330 core\n"
    "out vec4 color;\n"
    "void main() {\n"
    "   color = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
    "}\n";


GLint shader_compile(GLuint shader_id, char * buffer_info, size_t s_buffer_info) {
    /* TODO: Make this function on a list in the same way that shaders_delete
     * does. */

    /* Compile shader. */
    glCompileShader(shader_id);

    /* Retrieve context data. */
    GLint success = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);

    /* Get error if there was one. */
    if (!success) {
        glGetShaderInfoLog(shader_id, s_buffer_info, NULL, buffer_info);
    }
    return success;
}


GLint program_link(GLuint program_shader,
                   GLuint * ids,
                   size_t num_shaders,
                   char * buffer_info,
                   size_t size_buffer) {
    /* Attach and link the shaders in ids array. Check status and gather any
     * information on failure and store in buffer_info. Return status. */

    /* Attach all shaders ids. */
    for(size_t i=0; i<num_shaders; i++) {
        glAttachShader(program_shader, ids[i]);
    }

    /* Link all the attached shades. */
    glLinkProgram(program_shader);

    /* Examine the linking status. */
    GLint success;
    glGetProgramiv(program_shader, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(program_shader, size_buffer, NULL, buffer_info);
    }
    return success;
}


void shaders_delete(GLuint * ids, size_t num_ids) {
    /* Delete all shaders in the ids list. */
    for(size_t i=0; i<num_ids; i++) {
        glDeleteShader(ids[i]);
    }
}


void render(GLuint vertex_array,
            GLuint program_shader,
            size_t s_vertices,
            GLuint uloc_transform,
            GLfloat matrix_transform[4][4]) {

        /* Set the current linker program that should be used. */
        glUseProgram(program_shader);

        /* Set transformation matrix. */
        size_t count = 1;
        GLboolean transpose = GL_TRUE;
        GLfloat * ptr_value = &matrix_transform[0][0];
        glUniformMatrix4fv(uloc_transform, count, transpose, ptr_value);

        /* Bind the VAO that should be used. */
        glBindVertexArray(vertex_array);

        /* Draw the vertices as triangles. */
        glDrawArrays(GL_TRIANGLES, 0, s_vertices/3);

        /* Unset the shader program */
        glUseProgram(program_shader);

        /* Unbind the vertex array. */
        glBindVertexArray(0);
}


void square(GLfloat * buffer, Item_Data item, Environment_Data env){
    /* Populate vertices with data points corresponding to a square with
     * width pixel_width and height pixel_height. */

    /* Calculate pixel/screen-axis ratio. */
    float half_width = item.width * env.delta_width * 0.5f;
    float half_height = item.height * env.delta_height * 0.5f;

    float temp_buffer[] = {
        /* First triangle. */
        -half_width, half_height, 1.0f,
        -half_width,-half_height, 1.0f,
         half_width, half_height, 1.0f,

        /* Second triangle. */
        +half_width,+half_height, 1.0f,
        -half_width,-half_height, 1.0f,
        +half_width,-half_height, 1.0f,
    };

    /* Copy values from temp_buffer to buffer. */
    size_t temp_elements = SIZE(temp_buffer);
    size_t index = 0;
    for (size_t i=0; i<temp_elements; i++) {
        index = i + item.offset*temp_elements;
        buffer[index] = temp_buffer[i];
    }
}


typedef enum entities {
    PADDLE,
    BALL,
    NUM_ENTITIES,
} entities;


#define m4_unity (m4){\
    {1.0f, 0.0f, 0.0f, 0.0f}, \
    {0.0f, 1.0f, 0.0f, 0.0f}, \
    {0.0f, 0.0f, 1.0f, 0.0f}, \
    {0.0f, 0.0f, 0.0f, 1.0f}, \
}


void m4_set(m4 dest, m4 source) {
    for(size_t i=0; i<4; i++) {
        for(size_t j=0; j<4; j++) {
            dest[i][j] = source[i][j];
        }
    }
}


int main(void) {

    // ================================================================
    // == Window and context setup.
    // ================================================================

    GLFWwindow * window;

    if(!glfwInit()) {
        error("Could not initialize GLFW.\n", true);
    }

    /* Window dimensions. */
    GLint WIDTH = 800;
    GLint HEIGHT = 600;

    /* OpenGL window context hints. */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const char * title = "PONG.";

    window = glfwCreateWindow(WIDTH, HEIGHT, title, NULL, NULL);

    if(!window) {
        error("Could not create a window.\n", true);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK) {
        error("Could not initialize glew.\n", true);
    }

    /* Set clearing color. */
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // ================================================================
    // == Data and matrix setup.
    // ================================================================

    /* Create and populate environment data. */
    Environment_Data environment_data = {
        .width = WIDTH,
        .height = HEIGHT,
        .delta_width = 2.0f/WIDTH,
        .delta_height = 2.0f/HEIGHT,
    };

    /* Create array with transformation matrices. */
    m4 transformation_matrices[ID_NUM];

    /* Initialize all transformation matrices to unity matrix. */
    for (size_t i=0; i<ID_NUM; i++) {
        m4_set(transformation_matrices[i], m4_unity);
    }

    /* Set starting positions for each object. */
    transformation_matrices[ID_RIGHT_PADDLE][0][3] = 0.8f;
    transformation_matrices[ID_LEFT_PADDLE][0][3] = -0.8f;

    /* Paddle dimensions in pixels. */
    GLuint paddle_width = 20;
    GLuint paddle_height = 50;
    GLuint paddle_speed = 17;
    GLuint paddle_offset = 0;

    /* Ball dimensions in pixels. */
    GLuint ball_width = 15;
    GLuint ball_height = 15;
    GLuint ball_speed = 10;
    GLuint ball_offset = 1;

    /* Create Item_Data list items */
    Item_Data items[ID_NUM] = {0};

    /* Set item data for right paddle. */
    items[ID_RIGHT_PADDLE] = (Item_Data){
        .width=paddle_width,
        .height=paddle_height,
        .speed=paddle_speed,
        .offset=paddle_offset,
    };

    /* Set item data for left paddle. */
    items[ID_LEFT_PADDLE] = (Item_Data){
        .width=paddle_width,
        .height=paddle_height,
        .speed=paddle_speed,
        .offset=paddle_offset,
    };

    /* Set item data for ball. */
    items[ID_BALL] = (Item_Data){
        .width=ball_width,
        .height=ball_height,
        .speed=ball_speed,
        .offset=ball_offset,
    };

    Event_Data event_data = {0};
    event_data.window = window;
    event_data.transformation_matrices = &transformation_matrices[0];

    // ================================================================
    // == Buffers.
    // ================================================================

    /* Calculate and store values related to vertices. */
    size_t num_squares = 2;
    size_t num_vertices = 6*num_squares;
    size_t num_floats = 3*num_vertices;
    size_t num_floats_in_square = num_floats/num_squares;

    /* Create vertices. */
    GLfloat vertices[num_floats];
    square(vertices, items[ID_LEFT_PADDLE], environment_data);
    square(vertices, items[ID_BALL], environment_data);

    /* Create buffers. */
    GLuint VBOs[NUM_ENTITIES];
    GLuint VAOs[NUM_ENTITIES];

    /* Generate empty vertex and buffer object. */
    glGenVertexArrays(NUM_ENTITIES, VAOs);
    glGenBuffers(NUM_ENTITIES, VBOs);

    /* Set up main buffer object and populate. */

    /* Bind array buffer. */
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[PADDLE]);

    /* Populate buffer with data. */
    glBufferData(GL_ARRAY_BUFFER, num_floats*sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    /* Unbind buffer object. */
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* Set up the different Vertex Array Objects. */

    /* Set up binds for PADDLE VAO object. */
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[PADDLE]);

    /* Bind PADDLE vertex array object. */
    glBindVertexArray(VAOs[PADDLE]);

    /* Set vertex attribute pointer for position attribute. */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);

    /* Enable vertex attribute location pointer. */
    glEnableVertexAttribArray(0);

    /* Unbind buffer object. */
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* Unbind vertex array object. */
    glBindVertexArray(0);

    /* Set up binds for BALL VAO object. */

    /* Bind BALL vertex array object. */
    glBindVertexArray(VAOs[BALL]);

    /* Re-use the PADDLE VBO since it contains alla data. */
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[PADDLE]);

    /* Set vertex attribute pointer for position at an offset. */
    size_t offset_bytes = BALL * num_floats_in_square * sizeof(GLfloat);
    GLvoid * ptr_offset = (GLvoid *)offset_bytes;

    /* Set vertex attribute pointer for position at an offset. */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), ptr_offset);

    /* Enable vertex attribute pointer. */
    glEnableVertexAttribArray(0);

    /* Unbind vertex and buffer array. */
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // ================================================================
    // == Shaders.
    // ================================================================

    /* Create variables for fragment and vertex shader. */
    GLuint shader_fragment, shader_vertex;

    /* Create storage for shaders. */
    shader_vertex = glCreateShader(GL_VERTEX_SHADER);
    shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);

    /* Bind shader sources. */
    glShaderSource(shader_vertex, 1, &source_vertex_shader, 0);
    glShaderSource(shader_fragment, 1, &source_fragment_shader, 0);

    /* Compile shaders. */
    GLint success;
    size_t s_buffer_info = 1024;
    GLchar buffer_info[s_buffer_info];

    /* Compile vertex shader and print errors. */
    success = shader_compile(shader_vertex, buffer_info , s_buffer_info);
    if (!success) {
        error("Error in vertex shader:\n", false);
        error(buffer_info, false);
    }

    /* Compile fragment shader and print errors. */
    success = shader_compile(shader_fragment, buffer_info , s_buffer_info);
    if (!success) {
        error("Error in fragment shader:\n", false);
        error(buffer_info, false);
    }

    /* Set up shader program. */
    GLuint program_shader = glCreateProgram();

    /* Make shader list */
    GLuint shaders[] = {shader_vertex, shader_fragment};

    /* Attach shaders to shader program and link. */
    success = program_link(program_shader,
                           shaders,
                           SIZE(shaders),
                           buffer_info,
                           s_buffer_info);

    if(!success) {
        error("Error at program linkage:\n", false);
        error(buffer_info, false);
    }

    /* Delete linked shaders. */
    shaders_delete(shaders, SIZE(shaders));

    /* Get vertex shader transformation location. */
    GLuint uloc_transform = glGetUniformLocation(program_shader, "transform");

    // ================================================================
    // == Main loop.
    // ================================================================

    while(!glfwWindowShouldClose(window)) {

        /* Poll for events. */
        glfwPollEvents();

        /* React to polled events. */
        react_to_events(event_data, items, environment_data);

        /* Clear screen. */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Render the right paddle. */
        render(VAOs[PADDLE],
               program_shader,
               num_floats*sizeof(GLfloat),
               uloc_transform,
               transformation_matrices[ID_RIGHT_PADDLE]);

        /* Render the left paddle. */
        render(VAOs[PADDLE],
               program_shader,
               num_floats*sizeof(GLfloat),
               uloc_transform,
               transformation_matrices[ID_LEFT_PADDLE]);

        /* Render the ball. */
        render(VAOs[BALL],
               program_shader,
               num_floats*sizeof(GLfloat),
               uloc_transform,
               transformation_matrices[ID_BALL]);

        /* Swap buffers. */
        glfwSwapBuffers(window);
    }
}

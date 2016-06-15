#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define UNUSED(x) (void) x
#define SIZE(x) sizeof(x)/sizeof(x[0])

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


void react_to_events(GLFWwindow * window) {
    if (map_keys[GLFW_KEY_ESCAPE]) {
        glfwSetWindowShouldClose(window, GL_TRUE);
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


void square(GLfloat * buffer, size_t current_num, float width, float height) {
    /* Create square around center point (0, 0) with height and width. */
    float half_width = width/2.0f;
    float half_height = height/2.0f;

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
        index = i + current_num*temp_elements;
        buffer[index] = temp_buffer[i];
    }
}


GLfloat matrix_unity[4][4] = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};


typedef enum entities {
    PADDLE,
    BALL,
    NUM_ENTITIES,
} entities;


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
    // == Buffers.
    // ================================================================

    /* Create vertices. */
    size_t num_squares = 1;
    size_t num_vertices = 6*num_squares;
    size_t num_floats = 3*num_vertices;
    GLfloat vertices[num_floats];
    square(vertices, 0, 0.05f, 0.2f);

    /* Create buffers. */
    GLuint VBOs[NUM_ENTITIES];
    GLuint VAOs[NUM_ENTITIES];

    /* Generate empty vertex and buffer object. */
    glGenVertexArrays(NUM_ENTITIES, VAOs);
    glGenBuffers(NUM_ENTITIES, VBOs);

    /* Bind vertex array object. */
    glBindVertexArray(VAOs[PADDLE]);

    /* Bind buffer object */
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[PADDLE]);
    /* Populate buffer with data. */
    glBufferData(GL_ARRAY_BUFFER, num_floats*sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    /* Set vertex attribute pointer for position attribute. */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);
    /* Enable vertex attribute location pointer. */
    glEnableVertexAttribArray(0);

    /* Unbind buffer object. */
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
        react_to_events(window);

        /* Clear screen. */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Render the things. */
        render(VAOs[PADDLE],
               program_shader,
               num_floats*sizeof(GLfloat),
               uloc_transform,
               matrix_unity);

        /* Swap buffers. */
        glfwSwapBuffers(window);
    }
}

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define UNUSED(x) (void) x

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
    "\n"
    "void main() {"
    "   gl_Position = vec4(position, 1.0f);\n"
    "}\n";

const GLchar * source_fragment_shader = \
    "#version 330 core\n"
    "out vec4 color;\n"
    "void main() {\n"
    "   color = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
    "}\n";

const GLfloat vertices[] = {
     -0.5f, 0.5f, 1.0f,
      0.5f, 0.5f, 1.0f,
      0.0f,-0.5f, 1.0f,
};

GLint shader_compile(GLuint shader_id, char * buffer_info, size_t size_buffer) {

    /* Compile shader. */
    glCompileShader(shader_id);

    /* Retrieve context data. */
    GLint success = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);

    /* Get error if there was one. */
    if (!success) {
        glGetShaderInfoLog(shader_id, size_buffer, NULL, buffer_info);
    }
    return success;
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

    // ================================================================
    // == Buffers.
    // ================================================================

    /* Create buffers. */
    GLuint VBO, VAO;

    /* Generate empty vertex and buffer object. */
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    /* Bind vertex array object. */
    glBindVertexArray(VAO);

    /* Bind buffer object */
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    /* Populate buffer with data. */
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    /* Set vertex attribute pointer for position attribute. */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);
    /* Enable vertex attribute pointer. */
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
    GLint status_shader;
    size_t size_buffer = 1024;
    GLchar buffer_error[size_buffer];

    /* Compile vertex shader and print errors. */
    status_shader = shader_compile(shader_vertex, buffer_error , size_buffer);
    if (!status_shader) {
        error("Error in vertex shader:\n", false);
        error(buffer_error, false);
    }

    /* Compile fragment shader and print errors. */
    status_shader = shader_compile(shader_fragment, buffer_error , size_buffer);
    if (!status_shader) {
        error("Error in fragment shader:\n", false);
        error(buffer_error, false);
    }

    // ================================================================
    // == Main loop.
    // ================================================================

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        react_to_events(window);

        glfwSwapBuffers(window);
    }
}

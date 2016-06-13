#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define UNUSED(x) (void) x

void error(const char * message, bool fatal) {
    fprintf(stderr, "ERROR: %s", message);
    if (fatal) {
        fprintf(stderr, "ERROR: Shutting down.\n");
        exit(EXIT_FAILURE);
    }
}

int main(void) {

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

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK) {
        error("Could not initialize glew.\n", true);
    }

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glfwSwapBuffers(window);
    }
}

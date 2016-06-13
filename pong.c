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

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK) {
        error("Could not initialize glew.\n", true);
    }

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        react_to_events(window);

        glfwSwapBuffers(window);
    }
}

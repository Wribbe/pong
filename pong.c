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

    UNUSED(window);

//    if(!glfwInit()) {

    error("Non fatal error.\n", false);
    error("Fatal error.\n", true);

}

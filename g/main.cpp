/*
 * main.cpp
 *
 *  Created on: 2017年11月17日
 *      Author: liwei
 */

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "world.h"
int draw()
{
    return 0;
}
int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    world w;
    if(!w.load_data(NULL))
    {
        glfwTerminate();
        return -1;
    }
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(600, 600, "World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {

        w.draw();
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

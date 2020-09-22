#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

int main()
{
    int success = glfwInit();
    if (!success)
    {
        throw std::runtime_error("glfwInit failed");
    }

    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
    if (!window)
    {
        throw std::runtime_error("glfwCreateWindow failed");
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwWaitEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

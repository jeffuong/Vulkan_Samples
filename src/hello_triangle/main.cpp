#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdexcept>
#include <functional>
#include <cstdlib>

#include "../common/debug.h"

//Resources
GLFWwindow *g_window = nullptr;
int g_wndWidth = 800;
int g_wndHeight = 600;

void glfwErrorCallback(int code, const char *str)
{
    DBGLOG("GLFW error  %d: %s", code, str);
}

int initWindow(const char *appName)
{
    int ret = glfwInit();
    if(ret == GLFW_FALSE)
    {
        DBGLOG("GLFW init failed");
        return EXIT_FAILURE;
    }

    glfwSetErrorCallback(glfwErrorCallback);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    g_window = glfwCreateWindow(g_wndWidth, g_wndHeight, appName, nullptr, nullptr);

    return g_window != nullptr ? EXIT_SUCCESS : EXIT_FAILURE;
}

void destroyWindow()
{
    glfwDestroyWindow(g_window);
    DBGLOG("Window destroyed");
    
    glfwTerminate();
}

int initVulkan(int argc, const char *argv[])
{
    int ret = initWindow(argv[0]);
    if(ret != EXIT_SUCCESS)
        return ret;

    DBGLOG("Finished initialization");
    return ret;
}

int updateLoop()
{
    while(!glfwWindowShouldClose(g_window))
    {
        glfwPollEvents();
    }
    return EXIT_SUCCESS;
}

void releaseVulkan()
{
    //release resources

    destroyWindow();
    DBGLOG("Resources released");
}

int main(int argc, const char *argv[]) {
    int ret = initVulkan(argc, argv);
    if(ret != EXIT_SUCCESS)
    {
        releaseVulkan();
        return ret;
    }

    ret = updateLoop();
    releaseVulkan();
    return ret;
}
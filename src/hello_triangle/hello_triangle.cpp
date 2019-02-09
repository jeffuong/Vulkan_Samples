#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <string.h>
#include <vector>
#include <cstdlib>

#include "../common/debug.h"

//Resources
//Window
GLFWwindow *g_window = nullptr;
int g_wndWidth = 800;
int g_wndHeight = 600;

//Graphics
VkInstance g_instance;

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

void queryExtensions(std::vector<VkExtensionProperties> &extensions)
{
    extensions.clear();

    uint32_t extCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
    if(extCount == 0)
        return;

    extensions.resize(extCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, extensions.data());

    DBGLOG("Extensions available:\n");
    for(const VkExtensionProperties &ext : extensions)
        DBGLOG("\t%s v%u", ext.extensionName, ext.specVersion);
}

int createInstance(const char *appName)
{
    std::vector<VkExtensionProperties> available_extensions;
    queryExtensions(available_extensions);

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName;
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "My Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    const char **extensions;
    uint32_t numExtensions;
    extensions = glfwGetRequiredInstanceExtensions(&numExtensions);

    //Ensure all the extensions required are supported
    for(uint32_t i = 0; i < numExtensions; ++i)
    {
        bool ext_found = false;
        for(const VkExtensionProperties &ext : available_extensions)
        {
            if(strcmp(ext.extensionName, extensions[i]) == 0)
            {
                ext_found = true;
                break;
            }
        }
        if(!ext_found)
        {
            DBGLOG("Missing required extension: %s", extensions[i]);
            return EXIT_FAILURE;
        }
    }

    createInfo.enabledExtensionCount = numExtensions;
    createInfo.ppEnabledExtensionNames = extensions;

    createInfo.enabledLayerCount = 0; //We'll enable this later

    VkResult res = vkCreateInstance(&createInfo, nullptr, &g_instance);
    if(res != VK_SUCCESS)
        DBGLOG("Vulkan instance creation failed");

    return res == VK_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
}

int initVulkan(int argc, const char *argv[])
{
    //Strip out path from file name
    int i = strlen(argv[0]) - 1;
    for( ; i > 0; --i)
        if(argv[0][i] == '/')
            break;

    //Create window
    int ret = initWindow(argv[0] + i);
    if(ret != EXIT_SUCCESS)
        return ret;

    ret = createInstance(argv[0] + i);
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
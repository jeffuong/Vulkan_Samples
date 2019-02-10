#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>
#include <cstdlib>

#include "../common/debug.h"


struct QueueFamilies
{
    uint32_t graphicsQueue; 
    bool hasGraphicsQueue = false;;
    //Add other queue families in the future
};

//Resources
//Window
GLFWwindow *g_window = nullptr;
int g_wndWidth = 800;
int g_wndHeight = 600;

//Graphics
VkInstance g_instance;
VkDebugUtilsMessengerEXT g_debugMessenger;
VkPhysicalDevice g_physicalDevice = VK_NULL_HANDLE;

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

const char *stdValidationLayers = "VK_LAYER_LUNARG_standard_validation";

bool checkValidationLayerSupport()
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    if(layerCount == 0)
    {
        DBGLOG("No validation layers installed.");
        return false;
    }

    std::vector<VkLayerProperties> available_layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, available_layers.data());

    bool foundStdValidationLayer = false;
    DBGLOG("Available validation layers:")
    for(const VkLayerProperties &layer : available_layers)
    {
        DBGLOG("\t%s v%u s%u", layer.layerName, layer.implementationVersion, layer.specVersion);
        DBGLOG("\t\t%s", layer.description);
        if(strcmp(layer.layerName, stdValidationLayers) == 0)
            foundStdValidationLayer = true;
    }

    return foundStdValidationLayer;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) 
{
    #define VALIDATION_MSG "validation layer:"
    switch (messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            //Only print in debug mode
            DBGLOG(VALIDATION_MSG " %s", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            printf(VALIDATION_MSG " %s\n", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            printf("\x1B[33m" VALIDATION_MSG " %s\n \x1B[0m", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            printf("\x1B[31m" VALIDATION_MSG " %s\n \x1B[0m", pCallbackData->pMessage);
            break;
        default:
            //Normal print, in case none of the above cases were hit
            printf(VALIDATION_MSG " %s\n", pCallbackData->pMessage);
            break;
    }

    return VK_FALSE;
}

void getRequiredExtensions(std::vector<const char *> &extensions)
{
    uint32_t numExtensions;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&numExtensions);

    extensions.clear();
    extensions.reserve(numExtensions + 1);
    for(uint32_t i = 0; i < numExtensions; ++i)
        extensions.push_back(glfw_extensions[i]);

#ifdef _DEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
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

    std::vector<const char *> required_extensions;
    getRequiredExtensions(required_extensions);

    //Ensure all the extensions required are supported
    for(const char *required_ext : required_extensions)
    {
        bool ext_found = false;
        for(const VkExtensionProperties &ext : available_extensions)
        {
            if(strcmp(ext.extensionName, required_ext) == 0)
            {
                ext_found = true;
                break;
            }
        }
        if(!ext_found)
        {
            DBGLOG("Missing required extension: %s", required_ext);
            return EXIT_FAILURE;
        }
    }

    createInfo.enabledExtensionCount = required_extensions.size();
    createInfo.ppEnabledExtensionNames = required_extensions.data();

    //Enable validation layers, if possible, in debug builds
#ifdef _DEBUG
    if(checkValidationLayerSupport())
    {
        DBGLOG("Enabled standard LunarG validation layer");
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = &stdValidationLayers;
    }
#else
    createInfo.enabledLayerCount = 0; 
#endif

    VkResult res = vkCreateInstance(&createInfo, nullptr, &g_instance);
    if(res != VK_SUCCESS)
        DBGLOG("Vulkan instance creation failed");

    return res == VK_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
}

void setupDebugCallback()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = 
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;

#define CREATE_DEBUGUTILSMESSENGER_NAME "vkCreateDebugUtilsMessengerEXT"
    PFN_vkCreateDebugUtilsMessengerEXT pCreateDebugUtilsMessenger = 
        (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(g_instance, CREATE_DEBUGUTILSMESSENGER_NAME);

    if(pCreateDebugUtilsMessenger)
    {
        VkResult res = pCreateDebugUtilsMessenger(g_instance, &createInfo, nullptr, &g_debugMessenger);
        if(res != VK_SUCCESS)
        {
            DBGLOG("Failed to create debug messenger");
        }
        else
        {
            DBGLOG("Debug messenger created");
        }
    }
    else
    {
        DBGLOG("Failed tp load debug messenger creation function");
    }
}

void releaseDebugCallback()
{
    #define DESTROY_DEBUGUTILSMESSENGER_NAME "vkDestroyDebugUtilsMessengerEXT"
    PFN_vkDestroyDebugUtilsMessengerEXT pDestroyDebugUtils = 
        (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(g_instance, DESTROY_DEBUGUTILSMESSENGER_NAME);

    if(pDestroyDebugUtils)
    {
        pDestroyDebugUtils(g_instance, g_debugMessenger, nullptr);
        DBGLOG("Debug messenger destroyed");
    }
    else
        DBGLOG("Debug messenger destroy function not found");
}

int findQueueFamilies(VkPhysicalDevice device, QueueFamilies *queueFamilies)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    if(queueFamilyCount == 0)
    {
        DBGLOG("No queue families found");
        return EXIT_FAILURE;
    }

    std::vector<VkQueueFamilyProperties> props(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, props.data());

    //Take the first graphics queue
    uint32_t f = 0;
    for(const VkQueueFamilyProperties &fam : props)
    {
        if(fam.queueCount > 0 && fam.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queueFamilies->graphicsQueue = f;
            queueFamilies->hasGraphicsQueue = true;
            break;
        }
    }

    if(queueFamilies->hasGraphicsQueue)
        return EXIT_SUCCESS;

    DBGLOG("No graphics queue supported on this device");
    return EXIT_FAILURE;
}

int evalDeviceScore(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    int score = 0;

    //Ensure that this device supports a graphics queue
    QueueFamilies queueFamilies;
    //Score 0 if it doesn't as this is a mandatory requirement
    if(findQueueFamilies(device, &queueFamilies) != EXIT_SUCCESS)
        return 0;

    //Favor discrete GPUs
    if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 1000;

    //We like big textures
    score += props.limits.maxImageDimension2D;

    //Support for optional shader types is useful
    if(features.tessellationShader)
        score += 100;
    if(features.geometryShader)
        score += 10;

    DBGLOG("\t%s\tscore: %d", props.deviceName, score);
    return score;
}

int pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(g_instance, &deviceCount, nullptr);
    if(deviceCount == 0)
    {
        DBGLOG("No physical device found.");
        return EXIT_FAILURE;
    }

    std::vector<VkPhysicalDevice> deviceList(deviceCount);
    vkEnumeratePhysicalDevices(g_instance, &deviceCount, deviceList.data());

    //Sort devices by their scores
    std::map<int, VkPhysicalDevice> sortedDevices;

    DBGLOG("Physical device scores:");
    for(const VkPhysicalDevice &dev : deviceList)
    {
        sortedDevices[evalDeviceScore(dev)] = dev;
    }

    if(sortedDevices.rbegin()->first > 0)
    {
        g_physicalDevice = sortedDevices.rbegin()->second;
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(g_physicalDevice, &props);
        DBGLOG("Selected device: %s", props.deviceName);
        return EXIT_SUCCESS;
    }
    DBGLOG("No suitable physical device found.");
    return EXIT_FAILURE;
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

#ifdef _DEBUG
    setupDebugCallback();
#endif

    ret = pickPhysicalDevice();
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

#ifdef _DEBUG
    releaseDebugCallback();
#endif

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
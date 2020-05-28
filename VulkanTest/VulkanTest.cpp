#include <iostream>
#include <vector>
//#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#if (_MANAGED == 1) || (_M_CEE == 1)
#define Println(x) System::Console::WriteLine(x)
#define Print(x) System::Console::Write(x)
#else
#define Println(x) std::cout << x << std::endl
#define Print(x) std::cout << x
#endif

#ifdef _DEBUG

#define ASSERT_VULKAN(val) \
    if (val != VK_SUCCESS) \
    { \
        __debugbreak(); \
    }

#define VK_CALL(function) \
    ASSERT_VULKAN(function)

#else

#define ASSERT_VULKAN(x) x
#define VK_CALL(f) f

#endif

VkInstance instance;
VkSurfaceKHR surface;
VkDevice device;
GLFWwindow* window;

void PrintDeviceProperties(VkPhysicalDevice& device)
{

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    std::cout << "Name: " << properties.deviceName << std::endl;
    uint32_t apiVersion = properties.apiVersion;
    std::cout << "API Version: " <<
        VK_VERSION_MAJOR(apiVersion) << '.' <<
        VK_VERSION_MINOR(apiVersion) << '.' <<
        VK_VERSION_PATCH(apiVersion) << std::endl;

    std::cout << "Driver Version: " << properties.driverVersion << std::endl;
    std::cout << "Vendor ID: " << properties.vendorID << std::endl;
    std::cout << "Device ID: " << properties.deviceID << std::endl;
    std::cout << "Device Type: " << properties.deviceType << std::endl;
    std::cout << "Discrete Queue Priorities: " << properties.limits.discreteQueuePriorities << std::endl;

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(device, &memProps);

    uint32_t numberOfQueueFamilies = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &numberOfQueueFamilies, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProps;
    queueFamilyProps.resize(numberOfQueueFamilies);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &numberOfQueueFamilies, queueFamilyProps.data());

    std::cout << std::endl;
    std::cout << "Number of Queue families: " << numberOfQueueFamilies << std::endl;

    for (int i = 0; i < numberOfQueueFamilies; i++)
    {
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << "Queue Family #" << i << std::endl;
        std::cout << "VK_QUEUE_GRAPHICS_BIT " <<
            ((queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) << std::endl;
        std::cout << "VK_QUEUE_COMPUTE_BIT " <<
            ((queueFamilyProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) << std::endl;
        std::cout << "VK_QUEUE_TRANSFER_BIT " <<
            ((queueFamilyProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) << std::endl;
        std::cout << "VK_QUEUE_SPARSE_BINDING_BIT " <<
            ((queueFamilyProps[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) != 0) << std::endl;
        std::cout << "Queue Count: " << queueFamilyProps[i].queueCount << std::endl;
        std::cout << "Timestamp Valid Bits: " << queueFamilyProps[i].timestampValidBits << std::endl;
        uint32_t width = queueFamilyProps[i].minImageTransferGranularity.width;
        uint32_t height = queueFamilyProps[i].minImageTransferGranularity.height;
        uint32_t depth = queueFamilyProps[i].minImageTransferGranularity.depth;
        std::cout << "Min Image Transfer Granularity: " << width << ", " << height << ", " << depth << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
    }

    VkSurfaceCapabilitiesKHR surfaceCaps;
    VK_CALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &surfaceCaps));
    std::cout << "---------------------------------------------------" << std::endl;
    std::cout << "Physical Device Surface Capabilities:" << std::endl;
    std::cout << "Min Image Count: " << surfaceCaps.minImageCount << std::endl;
    std::cout << "Max Image Count: " << surfaceCaps.maxImageCount << std::endl;
    std::cout << "Current Extent Width: " << surfaceCaps.currentExtent.width << std::endl;
    std::cout << "Current Extent Height: " << surfaceCaps.currentExtent.height << std::endl;
    std::cout << "Min Image Extent Width: " << surfaceCaps.minImageExtent.width << std::endl;
    std::cout << "Min Image Extent Height: " << surfaceCaps.minImageExtent.height << std::endl;
    std::cout << "Max Image Extent Width: " << surfaceCaps.maxImageExtent.width << std::endl;
    std::cout << "Max Image Extent Height: " << surfaceCaps.maxImageExtent.height << std::endl;
    std::cout << "Max Image Array Layers: " << surfaceCaps.maxImageArrayLayers << std::endl;
    std::cout << "Supported Transforms: " << surfaceCaps.supportedTransforms << std::endl;
    std::cout << "Current Transform: " << surfaceCaps.currentTransform << std::endl;
    std::cout << "Supported Composite Alpha: " << surfaceCaps.supportedCompositeAlpha << std::endl;
    std::cout << "Supported Usage Flags: " << surfaceCaps.supportedUsageFlags << std::endl;
    std::cout << ---------------------------------------------------" << std::endl;
}

void InitGlfw()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(400, 300, "Vulkan Test", nullptr, nullptr);
}

void InitVulkan()
{
    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "Vulkan Test";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.pEngineName = "Vulkan Test Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    uint32_t numLayers = 0;
    vkEnumerateInstanceLayerProperties(&numLayers, nullptr);
    std::vector<VkLayerProperties> layers;
    layers.resize(numLayers);

    vkEnumerateInstanceLayerProperties(&numLayers, layers.data());

    std::cout << "Number of Instance Layers: " << numLayers << std::endl;
    for (int i = 0; i < numLayers; i++) {
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << "Layer #" << i << std::endl;
        std::cout << "Layer Name: " << layers[i].layerName << std::endl;
        std::cout << "Layer Spec Version: " << layers[i].specVersion << std::endl;
        std::cout << "Layer Impl Version: " << layers[i].implementationVersion << std::endl;
        std::cout << "Layer Description: " << layers[i].description << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
    }

    uint32_t numExt = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &numExt, nullptr);
    std::vector<VkExtensionProperties> extensions;
    extensions.resize(numExt);
    vkEnumerateInstanceExtensionProperties(nullptr, &numExt, extensions.data());
    for (int i = 0; i < numExt; i++) {
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << "Extension #" << i << std::endl;
        std::cout << "Extension Name: " << extensions[i].extensionName << std::endl;
        std::cout << "Extension Spec Version: " << extensions[i].specVersion << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
    }

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_LUNARG_standard_validation"
    };

    uint32_t numberOfGlfwExt = 0;
    const char** glfwExt = glfwGetRequiredInstanceExtensions(&numberOfGlfwExt);
     
    VkInstanceCreateInfo instanceInfo;
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = validationLayers.size();
    instanceInfo.ppEnabledLayerNames = validationLayers.data();
    instanceInfo.enabledExtensionCount = numberOfGlfwExt;
    instanceInfo.ppEnabledExtensionNames = glfwExt;

    VK_CALL(vkCreateInstance(&instanceInfo, nullptr, &instance));

    VK_CALL(glfwCreateWindowSurface(instance, window, nullptr, &surface));

    uint32_t physDeviceCount = 0;
    VK_CALL(vkEnumeratePhysicalDevices(instance, &physDeviceCount, nullptr));
    std::vector<VkPhysicalDevice> physicalDevices;
    physicalDevices.resize(physDeviceCount);
    VK_CALL(vkEnumeratePhysicalDevices(instance, &physDeviceCount, physicalDevices.data()));

    for (size_t i = 0; i < physDeviceCount; i++) {
        PrintDeviceProperties(physicalDevices[i]);
    }

    float queuePriorities[] = {
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };

    VkDeviceQueueCreateInfo deviceQueueCreateInfo;
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.pNext = nullptr;
    deviceQueueCreateInfo.flags = 0;
    deviceQueueCreateInfo.queueFamilyIndex = 0; // TODO : check the queue families for the most suitable one
    deviceQueueCreateInfo.queueCount = 16; // TODO : check if the count is valid / use 4 if doesn't work
    deviceQueueCreateInfo.pQueuePriorities = queuePriorities;

    VkPhysicalDeviceFeatures usedFeatures = {};

    VkDeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.enabledLayerCount = validationLayers.size();
    deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    deviceCreateInfo.enabledExtensionCount = 0;
    deviceCreateInfo.ppEnabledExtensionNames = nullptr;
    deviceCreateInfo.pEnabledFeatures = &usedFeatures;

    VK_CALL(vkCreateDevice(physicalDevices[0], &deviceCreateInfo, nullptr, &device)); // TODO : pick the best device instead of the first device

    VkQueue queue;
    vkGetDeviceQueue(device, 0, 0, &queue);

}

void GameLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void ShutdownVulkan()
{

    VK_CALL(vkDeviceWaitIdle(device));

    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr); // der physikalische device gehoert der Instance

}

void ShutdownGlfw()
{
    glfwDestroyWindow(window);
}

int main()
{
    InitGlfw();
    InitVulkan();
    GameLoop();
    ShutdownVulkan();
    ShutdownGlfw();

    return 0;
}

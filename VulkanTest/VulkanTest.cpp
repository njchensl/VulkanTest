#pragma managed

#include <fstream>
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

#define VK_ASSERT(val) \
    if (val != VK_SUCCESS) \
    { \
        std::cerr << "Vulkan assertion failed in " << __FILE__ << " at line " << __LINE__ << std::endl; \
        __debugbreak(); \
    }

#define ASSERT(val) \
    if (val != VK_SUCCESS) \
    { \
        std::cerr << "Assertion failed in " << __FILE__ << " at line " << __LINE__ << std::endl; \
        __debugbreak(); \
    }

#define VK_CALL(function) \
    VK_ASSERT(function)

#else

#define ASSERT_VULKAN(x) x
#define VK_CALL(f) f

#endif

#if defined(VK_NULL_HANDLE) && VK_NULL_HANDLE == 0
#undef VK_NULL_HANDLE
#define VK_NULL_HANDLE nullptr
#else
#error Incorrect null handle definition
#endif

#ifdef interface
#undef interface
#endif

typedef uint8_t byte;
typedef char sbyte;
typedef uint32_t uint;


VkInstance instance;
VkSurfaceKHR surface;
VkDevice device;
VkSwapchainKHR swapchain;
std::vector<VkImageView> imgViews;
GLFWwindow* window;

VkShaderModule vsModule;
VkShaderModule fsModule;

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 720;

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
    std::cout << "---------------------------------------------------" << std::endl;

    uint32_t numFormats = 0;
    VK_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &numFormats, nullptr));
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    surfaceFormats.resize(numFormats);
    VK_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &numFormats, surfaceFormats.data()));
    std::cout << "---------------------------------------------------" << std::endl;
    std::cout << "Number of Formats: " << numFormats << std::endl;
    for (int i = 0; i < numFormats; i++)
    {
        std::cout << "Format #" << i << ": " << surfaceFormats[i].format << std::endl;
        std::cout << "Color Space #" << i << ": " << surfaceFormats[i].colorSpace << std::endl;
    }
    std::cout << "---------------------------------------------------" << std::endl;
}

std::vector<byte> ReadFileBin(const std::string& filepath)
{
    std::ifstream fs(filepath, std::ios::binary | std::ios::ate);
    if (fs)
    {
        size_t fileSize = fs.tellg();
        std::vector<byte> fileBuffer(fileSize);
        fs.seekg(0);
        fs.read((sbyte*)fileBuffer.data(), fileSize);
        return fileBuffer;
    }
    throw std::runtime_error("File error");
}

void InitGlfw()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Test", nullptr, nullptr);
}

void CreateShaderModule(const std::vector<byte>& code, VkShaderModule* shaderModule)
{
    VkShaderModuleCreateInfo shaderCreateInfo;
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderCreateInfo.pNext = nullptr;
    shaderCreateInfo.flags = 0;
    shaderCreateInfo.codeSize = code.size();
    shaderCreateInfo.pCode = (uint32_t*)code.data();

    VK_CALL(vkCreateShaderModule(device, &shaderCreateInfo, nullptr, shaderModule));
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
    for (uint i = 0; i < numLayers; i++)
    {
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
    for (uint i = 0; i < numExt; i++)
    {
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

    for (size_t i = 0; i < physDeviceCount; i++)
    {
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

    const std::vector<const char*> deviceExt = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.enabledLayerCount = validationLayers.size();
    deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    deviceCreateInfo.enabledExtensionCount = deviceExt.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExt.data();
    deviceCreateInfo.pEnabledFeatures = &usedFeatures;

    VK_CALL(vkCreateDevice(physicalDevices[0], &deviceCreateInfo, nullptr, &device));
    // TODO : pick the best device instead of the first device

    VkQueue queue;
    vkGetDeviceQueue(device, 0, 0, &queue);

    VkBool32 surfaceSupport = VK_FALSE;
    VK_CALL(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[0], 0, surface, &surfaceSupport));

    //ASSERT_VULKAN(surfaceSupport);

    VkSwapchainCreateInfoKHR scCreateInfo;
    scCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    scCreateInfo.pNext = nullptr;
    scCreateInfo.flags = 0;
    scCreateInfo.surface = surface;
    scCreateInfo.minImageCount = 3; // TODO : check if valid
    scCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM; // TODO civ
    scCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; // TODO civ
    scCreateInfo.imageExtent = {WIDTH, HEIGHT};
    scCreateInfo.imageArrayLayers = {1};
    scCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    scCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // TODO civ
    scCreateInfo.queueFamilyIndexCount = 0;
    scCreateInfo.pQueueFamilyIndices = nullptr;
    scCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    scCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    scCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; // TODO civ
    scCreateInfo.clipped = VK_TRUE; // set to false if we need to access those pixels in the shaders
    scCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    VK_CALL(vkCreateSwapchainKHR(device, &scCreateInfo, nullptr, &swapchain));

    uint32_t numImgs = 0;
    VK_CALL(vkGetSwapchainImagesKHR(device, swapchain, &numImgs, nullptr));
    std::vector<VkImage> swapchainImgs;
    swapchainImgs.resize(numImgs);
    VK_CALL(vkGetSwapchainImagesKHR(device, swapchain, &numImgs, swapchainImgs.data()));

    imgViews.resize(numImgs);
    for (uint i = 0; i < numImgs; i++)
    {
        VkImageViewCreateInfo imgViewCreateInfo;
        imgViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imgViewCreateInfo.pNext = nullptr;
        imgViewCreateInfo.flags = 0;
        imgViewCreateInfo.image = swapchainImgs[i];
        imgViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imgViewCreateInfo.format = VK_FORMAT_B8G8R8A8_UNORM; // TODO civ
        imgViewCreateInfo.components = {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY
        };
        imgViewCreateInfo.subresourceRange = {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0, 1, 0, 1
        };

        VK_CALL(vkCreateImageView(device, &imgViewCreateInfo, nullptr, &imgViews[i]));
    }

    const std::vector<byte> vs = ReadFileBin("vert.spv");
    const std::vector<byte> fs = ReadFileBin("frag.spv");

    CreateShaderModule(vs, &vsModule);
    CreateShaderModule(fs, &fsModule);

    VkPipelineShaderStageCreateInfo vsStageCreateInfo;
    vsStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vsStageCreateInfo.pNext = nullptr;
    vsStageCreateInfo.flags = 0;
    vsStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vsStageCreateInfo.module = vsModule;
    vsStageCreateInfo.pName = "main";
    vsStageCreateInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fsStageCreateInfo;
    fsStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fsStageCreateInfo.pNext = nullptr;
    fsStageCreateInfo.flags = 0;
    fsStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fsStageCreateInfo.module = fsModule;
    fsStageCreateInfo.pName = "main";
    fsStageCreateInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] = {vsStageCreateInfo, fsStageCreateInfo};
    

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

    for (VkImageView& view : imgViews)
    {
        vkDestroyImageView(device, view, nullptr);
    }
    vkDestroyShaderModule(device, vsModule, nullptr);
    vkDestroyShaderModule(device, fsModule, nullptr);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
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

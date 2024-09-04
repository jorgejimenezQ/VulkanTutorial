#include "DrawingTrianglesExample.h"

// Constants for window dimensions
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// Validation layers for debugging Vulkan
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

// Enable validation layers based on build type (debug or release)
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// Function to create a debug messenger for logging debug information in Vulkan
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

// Function to destroy the debug messenger
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

// Constructor for DrawingTrianglesExample
DrawingTrianglesExample::DrawingTrianglesExample()
{
}

// Destructor for DrawingTrianglesExample
DrawingTrianglesExample::~DrawingTrianglesExample()
{
}

// Main application class for rendering a triangle using Vulkan
class HelloTriangleApplication {
public:
    // Main function to run the application
    void run() {
        initWindow();  // Initialize the GLFW window
        initVulkan();  // Initialize Vulkan objects
        mainLoop();    // Enter the rendering loop
        cleanup();     // Cleanup and free resources
    }

private:
    GLFWwindow* window;   // Pointer to the GLFW window

    VkInstance instance;  // Vulkan instance
    VkDebugUtilsMessengerEXT debugMessenger;  // Debug messenger for validation layers
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;  // Handle to the physical GPU
    VkDevice device;      // Logical device to interface with the GPU
    VkQueue graphicsQueue; // Queue for graphics commands

    // Structure to hold queue family indices
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        bool isComplete() {
            return graphicsFamily.has_value();
        }
    };

    // Initialize the GLFW window
    void initWindow() {
        glfwInit();  // Initialize GLFW

        // Configure GLFW to not use OpenGL and disable window resizing
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        // Create the window
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    // Initialize Vulkan by creating the instance and picking devices
    void initVulkan() {
        createInstance();  // Create Vulkan instance
        setupDebugMessenger();  // Set up the debug messenger for logging
        pickPhysicalDevice();   // Select the physical GPU
        createLogicalDevice();  // Create a logical device from the physical one
    }

    // Main loop to keep the window open and handle events
    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();  // Poll for window events
        }
    }

    // Cleanup Vulkan and GLFW resources
    void cleanup() {
        vkDestroyDevice(device, nullptr);  // Destroy the logical device

        // Destroy the debug messenger if validation layers are enabled
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroyInstance(instance, nullptr);  // Destroy the Vulkan instance
        glfwDestroyWindow(window);  // Destroy the GLFW window
        glfwTerminate();  // Terminate GLFW
    }

    // Create a logical device for interfacing with the physical GPU
    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);  // Find the queue family supporting graphics commands

        VkDeviceQueueCreateInfo queueCreateInfo{};  // Structure to create a queue
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;

        // Set queue priority to maximum
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures{};  // Device features to enable

        VkDeviceCreateInfo createInfo{};  // Structure to create the logical device
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = 0;
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        // Create the logical device and check for success
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);  // Get the graphics queue from the device
    }

    // Pick a suitable physical GPU for rendering
    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);  // Get the number of available physical devices

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());  // Get all available devices

        // Iterate through the devices and check if they are suitable
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;  // Select the first suitable device
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    // Check if a device is suitable for our needs
    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);  // Check if the device supports the necessary queue families

        return indices.isComplete();
    }

    // Find queue families that support graphics commands on the physical device
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);  // Get the number of queue families
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());  // Get properties of all queue families

        int i = 0;
        // Iterate through the queue families to find one that supports graphics commands
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }
            i++;
        }
        return indices;
    }

    // Create the Vulkan instance
    void createInstance() {
        // Check if validation layers are available
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        // Application information structure
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";  // Name of the application
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);  // Application version
        appInfo.pEngineName = "No Engine";  // Engine name (not using any)
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);  // Engine version
        appInfo.apiVersion = VK_API_VERSION_1_0;  // Vulkan API version

        // Instance creation information
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();  // Get required extensions for GLFW
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            // Populate the debug messenger create info
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        // Create the Vulkan instance and check for success
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    // Fill in the debug messenger create info structure
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;  // Callback function for validation layer messages
    }

    // Set up the debug messenger for validation layers
    void setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    // Get the required Vulkan extensions for GLFW
    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        // Add debug extension if validation layers are enabled
        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    // Check if the requested validation layers are available
    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);  // Get the number of available validation layers

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());  // Get the properties of all available layers

        // Check if all requested validation layers are available
        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    // Callback function for the debug messenger to log validation messages
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
};

// Function to run the DrawingTrianglesExample application
void DrawingTrianglesExample::run() {
    HelloTriangleApplication app;

    try {
        app.run();  // Run the application
    }
    catch (const std::exception& e) {
        // Rethrow any exception that occurs
        throw e;
    }
}

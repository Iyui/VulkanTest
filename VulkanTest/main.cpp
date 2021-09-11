
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <algorithm>
//vkGetInstanceProcAddr函数如果不能被加载，那么代理函数
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

//指定是否启用指定的校验层
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

//使用结构体来存储查询得到的交换链细节信息
struct SwapChainSupportDetails {
     VkSurfaceCapabilitiesKHR capabilities;
     std :: vector<VkSurfaceFormatKHR> formats;
     std :: vector<VkPresentModeKHR> presentModes;
};

class HelloTriangleApplication {
	GLFWwindow* window;
    VkDevice device;

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkSurfaceKHR surface;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    std :: vector<VkImageView> swapChainImageViews;


    const std :: vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };


	struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
	};


#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    //

private:
    VkInstance instance;
    VkDebugUtilsMessengerEXT callback;//存储回调函数信息，然后将它提交给 Vulkan 完成回调函数的设置
    VkDebugUtilsMessengerEXT debugMessenger;

public:
	void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);//显式阻止GLFW自动创建OPENGL上下文
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);//禁止窗口大小改变
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);//创建窗口句柄
    }

    void initVulkan() {
        createInstance();
        setupDebugCallback();
        CreateSurfaceWindow();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createGraphicsPipeline();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
             glfwPollEvents();
        }
    }

    void cleanup() {
        for(auto imageView : swapChainImageViews) {
             vkDestroyImageView(device, imageView, nullptr); 
        }

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();

        //vkDestroyDevice(device, nullptr);

    }

    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount,devices.data());
        if (deviceCount == 0)
            throw std::runtime_error("failed to find gpu with vulkan support");  
        for(const auto& device:devices)
        {
            if (isDeviceSuitable(device))
            {
                physicalDevice = device;
                break;
            }
        }
		if (physicalDevice == VK_NULL_HANDLE)
			throw std::runtime_error("failed to find a suitable GPU");
	}

    //将所需的扩展保存在一个集合中，然后枚举所有可用的扩展，将集合中的扩展剔除，最后，如果这个集合中的元素为 0，说明我们所需的扩展全部都被满足。
	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &
			extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(
			extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &
			extensionCount, availableExtensions.data());
		std::set<std::string > requiredExtensions(
			deviceExtensions.begin(), deviceExtensions.end());
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension .
				extensionName);
		}
		return requiredExtensions.empty();
	}

	bool isDeviceSuitable(VkPhysicalDevice device)
	{
		/*VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader;*/
		QueueFamilyIndices indices = findQueueFamilies(device);
		bool extensionsSupported = checkDeviceExtensionSupport(
			device);
        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport =
                querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty()
                && !swapChainSupport.presentModes.empty();
        }
		return indices.isComplete() && extensionsSupported &&
            swapChainAdequate;
	}

    void createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    //获取拓展支持信息
    void GetExtensionsInfo()
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        std::cout << "available extensions:\n";

        for (const auto& extension : extensions) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
    }

    //请求所有可用的校验层
    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());//获取所有可用校验层的列表

        //检查是否所有 validationLayers 列表中的校验层都可以在 availableLayers 列表中
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

    //根据是否启用校验层，返回所需的扩展列表
    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    //第一个参数指定消息级别：诊断信息 资源创建之类的信息 警告信息 不合法和可能造成奔溃的操作信息
    //第二个参数可以是以下这些值
    //1、发生了一些与规范和性能无关的事件
    //2、出现了违反规范的情况或发生了一个可能的错误
    //3、进行了可能影响 Vulkan 性能的行为
    //pCallbackData 参数是一个指向 VkDebugUtilsMessengerCallbackDataEXT 结构体的指针，这一结构体包含了下面这些非常重要的成员
    //1、pMessage：一个以 null 结尾的包含调试信息的字符串
    //2、pObjects：存储有和消息相关的 Vulkan 对象句柄的数组
    //3、objectCount：数组中的对象个数
    //pUserData 是一个指向了我们设置回调函数时，传递的数据的指针。
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr; // Optional
    }



    void setupDebugCallback() {
        if(!enableValidationLayers) return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug messenger!");
        }   
    }

 

	//返回满足需求的队列族的索引
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            //来检查物理设备是否具有呈现能力
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
	}

    //逻辑设备创建
	void createLogicalDevice()
	{
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        //创建所有使用的队列族
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast <uint32_t>(
            deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

    void CreateSurfaceWindow()
    {
        //VkWin32SurfaceCreateInfoKHR createInfo = {};
        //createInfo.sType =
        //    VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        //createInfo.hwnd = glfwGetWin32Window(window);//获取 GLFW 窗口对象的 Windows 平台窗口句柄
        //createInfo.hinstance = GetModuleHandle(nullptr);//获取当前进程的实例句柄

        //auto CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)
        //    vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
        //if (!CreateWin32SurfaceKHR || CreateWin32SurfaceKHR(instance, &
        //    createInfo, nullptr, &surface) != VK_SUCCESS) {
        //    throw std::runtime_error(" failed to create window surface!");

        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
	}


    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice
        device) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &
            details.capabilities);//查询基础表面信息

        //首先查询格式数量，然后分配数组空间查询具体信息
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &
            formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &
                formatCount, details.formats.data());
        }

        //查询支持的呈现模式
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &
            presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device,
                surface, &presentModeCount, details.presentModes.data());
            return details;
        }
	}

	//选择合适的表面格式
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<
		VkSurfaceFormatKHR>& availableFormats) {
		if (availableFormats.size() == 1 && availableFormats[0].format
			== VK_FORMAT_UNDEFINED) {
			return { VK_FORMAT_B8G8R8A8_UNORM,
			 VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}
        //如果 Vulkan 返回了一个格式列表，那么我们检查这个列表，看下我们想要设定的格式是否存在于这个列表中
        for(const auto & availableFormat : availableFormats) {
             if(availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM
                && availableFormat.colorSpace ==
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                 return availableFormat;
            }   
		}
        //如果不能在列表中找到我们想要的格式，我们可以对列表中存在的格式进行打分，选择分数最高的那个作为我们使用的格式
        return availableFormats[0];
    }

    //查找最佳的可用呈现模式
    VkPresentModeKHR chooseSwapPresentMode(const std :: vector<
        VkPresentModeKHR> availablePresentModes) {
        VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
        //检查三倍缓冲是否可用
        for(const auto & availablePresentMode :
        availablePresentModes) {
             if(availablePresentMode ==
                VK_PRESENT_MODE_MAILBOX_KHR)
                 {
                 return availablePresentMode;
                 }else if(availablePresentMode ==
                     VK_PRESENT_MODE_IMMEDIATE_KHR) {
                  bestMode = availablePresentMode;  
             }  
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    //交换范围
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR&
        capabilities) {
        if(capabilities.currentExtent.width != (std::numeric_limits<uint32_t >::max)()) {
             return capabilities.currentExtent;
        } else{
      VkExtent2D actualExtent = {WIDTH, HEIGHT};
      actualExtent.width = (std :: max)(capabilities.minImageExtent.width , 
          (std :: min)(capabilities.maxImageExtent.width , 
              actualExtent.width));
      actualExtent.height = (std :: max)(capabilities.minImageExtent.height ,
          (std :: min)(capabilities.maxImageExtent.height ,
     actualExtent.height));
     return actualExtent;
      }    
    }

	//创建交换链
	void createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
	}

    void createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());//分配足够的数组空间来存储图像视图
		//遍历所有交换链图像，创建图像视图：
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;//用于指定图像被看作是一维纹理、二维纹理、三维纹理还是立方体贴图
			createInfo.format = swapChainImageFormat;
			/*
			* components 成员变量用于进行图像颜色通道的映射。比如，对于单色
            纹理，我们可以将所有颜色通道映射到红色通道。我们也可以直接将颜色
            通道的值映射为常数 0 或 1。在这里，我们只使用默认的映射：
			*/
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            //调用 vkCreateImageView 函数创建图像视图
            if(vkCreateImageView(device, &createInfo, nullptr, &
                swapChainImageViews[i]) != VK_SUCCESS) {
                throw std :: runtime_error("failed to create image views !");
            }
        }
    }

    //管线

    void createGraphicsPipeline() {
        
    }


    //

};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
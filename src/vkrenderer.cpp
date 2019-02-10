#include "vkrenderer.h"

#include <stdio.h>
#include "camera.h"
#include "window.h"
#include "macro.h"
#include "array.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vk_mem_alloc.h"

#define CheckResult(expr) \
{ \
    VkResult res = (expr); \
    Assert(res == VK_SUCCESS); \
}

enum QueueType
{
    QT_Graphics = 0,
    QT_Compute,
    QT_Present,
    QT_Transfer,
    QT_Count
};

struct SwapchainSupport
{
    VkSurfaceCapabilitiesKHR        capabilities;
    TempArray<VkSurfaceFormatKHR>   formats;
    TempArray<VkPresentModeKHR>     modes;
};

namespace VkRenderer
{
    static Array<const char*>               ms_layerNames;
    static Array<const char*>               ms_instExtNames;
    static Array<const char*>               ms_devExtNames;

    static VkInstance                       ms_inst;
    static VkPhysicalDevice                 ms_pdev;
    static VkDevice                         ms_dev;

    static VkQueue                          ms_queues[QT_Count];
    static int32_t                          ms_qfams[QT_Count];

    static VkPhysicalDeviceMemoryProperties ms_memprops;
    static VmaAllocator                     ms_allocator;

    static FixedArray<VkImage, 4>           ms_swapImages;
    static VkSwapchainKHR                   ms_swapchain;
    static VkSurfaceKHR                     ms_surface;
    static VkSurfaceCapabilitiesKHR         ms_surfaceCaps;
    static VkSurfaceFormatKHR               ms_surfaceFormat;
    static VkExtent2D                       ms_surfaceExtent;
    static VkPresentModeKHR                 ms_presentMode;

    static const VkSurfaceFormatKHR         preferredSwapFormats[] = 
    {
        { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
    };
    static const VkPresentModeKHR           preferredSwapModes[] = 
    {
        VK_PRESENT_MODE_MAILBOX_KHR,        // single-entry queue that gets overwritten; useful for triple buffering
        VK_PRESENT_MODE_FIFO_RELAXED_KHR,   // multi-entry queue, waits for blank if queue not empty
        VK_PRESENT_MODE_FIFO_KHR,           // multi-entry queue read per-blank
        VK_PRESENT_MODE_IMMEDIATE_KHR,      // no queue, no vertical blank sync
    };

    DVK_ONLY(
        static VkDebugUtilsMessengerEXT     ms_dbgMsger;

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT             messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void*                                       pUserData) 
        {
            printf("validation layer: %s\n", pCallbackData->pMessage);
            Assert(!(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT));
            return VK_FALSE;
        });

    void Init()
    {
        SelectExtensions();
        CreateInstance();
        CreateSurface();
        ChoosePhysicalDevice();
        CreateDevice();
        CreateAllocator();
        CreateSwapchain();
    }
    void SelectExtensions()
    {
        {
            uint32_t count = 0;
            const char** glfwExts = glfwGetRequiredInstanceExtensions(&count);
            for(uint32_t i = 0; i < count; ++i)
            {
                ms_instExtNames.grow() = glfwExts[i];
            }
        }
        DVK_ONLY({
            ms_layerNames.grow() = "VK_LAYER_LUNARG_standard_validation";
            ms_instExtNames.grow() = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        });
        ms_devExtNames.grow() = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    }
    void CreateInstance()
    {
        VkApplicationInfo appInfo;
        MemZero(appInfo);
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "RL1";
        appInfo.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo instInfo;
        MemZero(instInfo);
        instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instInfo.pApplicationInfo = &appInfo;

        instInfo.enabledExtensionCount = ms_instExtNames.count();
        instInfo.ppEnabledExtensionNames = ms_instExtNames.begin();
        instInfo.enabledLayerCount = ms_layerNames.count();
        instInfo.ppEnabledLayerNames = ms_layerNames.begin();
        CheckResult(vkCreateInstance(&instInfo, 0, &ms_inst));

        DVK_ONLY({
            VkDebugUtilsMessengerCreateInfoEXT dminfo;
            MemZero(dminfo);
            dminfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            dminfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            dminfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            dminfo.pfnUserCallback = DebugCallback;

            auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                ms_inst, 
                "vkCreateDebugUtilsMessengerEXT");
            Assert(func != nullptr);
            CheckResult(func(ms_inst, &dminfo, 0, &ms_dbgMsger));
        });
    }
    int32_t RatePhysicalDevice(VkPhysicalDevice pdev)
    {
        int32_t score = 0;

        VkPhysicalDeviceProperties props;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(pdev, &props);
        vkGetPhysicalDeviceFeatures(pdev, &features);

        score += props.limits.maxImageDimension2D;
        score += props.limits.maxPushConstantsSize;
        score += props.limits.maxUniformBufferRange;
        score += props.limits.maxStorageBufferRange;

        if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            score += 1000;
            score *= 2;
        }

        return score;
    }
    int32_t FindQueueFamily(
        VkPhysicalDevice                    pdev,
        TempArray<VkQueueFamilyProperties>& fams,
        VkQueueFlags                        flags,
        bool                                present)
    {
        for(int32_t i = 0; i < fams.count(); ++i)
        {
            if(!fams[i].queueCount)
            {
                continue;
            }
            if((flags & fams[i].queueFlags) != flags)
            {
                continue;
            }
            if(present)
            {
                VkBool32 hasPres = false;
                CheckResult(vkGetPhysicalDeviceSurfaceSupportKHR(
                    pdev, i, ms_surface, &hasPres));
                if(!hasPres)
                {
                    continue;
                }
            }
            return i;
        }

        return -1;
    }
    bool DevExtensionSupport(
        VkPhysicalDevice                    pdev, 
        TempArray<VkExtensionProperties>&   props)
    {
        uint32_t count = 0;
        CheckResult(vkEnumerateDeviceExtensionProperties(
            pdev, nullptr, &count, nullptr));
        props.resize(count);
        CheckResult(vkEnumerateDeviceExtensionProperties(
            pdev, nullptr, &count, props.begin()));
        for(const char* extName : ms_devExtNames)
        {
            bool found = false;
            for(const VkExtensionProperties& prop : props)
            {
                if(strcmp(extName, prop.extensionName) == 0)
                {
                    found = true;
                    break;
                }
            }
            if(!found)
            {
                return false;
            }
        }
        return true;
    }
    void GetQFamProps(VkPhysicalDevice pdev, TempArray<VkQueueFamilyProperties>& fams)
    {
        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pdev, &count, 0);
        fams.resize(count);
        vkGetPhysicalDeviceQueueFamilyProperties(pdev, &count, fams.begin());
    }
    void GetSwapchainSupport(VkPhysicalDevice pdev, SwapchainSupport& support)
    {
        support.formats.clear();
        support.modes.clear();

        CheckResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            pdev, ms_surface, &support.capabilities));

        {
            uint32_t count = 0;
            CheckResult(vkGetPhysicalDeviceSurfaceFormatsKHR(
                pdev, ms_surface, &count, nullptr));
            support.formats.resize(count);
            CheckResult(vkGetPhysicalDeviceSurfaceFormatsKHR(
                pdev, ms_surface, &count, support.formats.begin()));
        }
        {
            uint32_t count = 0;
            CheckResult(vkGetPhysicalDeviceSurfacePresentModesKHR(
                pdev, ms_surface, &count, nullptr));
            support.modes.resize(count);
            CheckResult(vkGetPhysicalDeviceSurfacePresentModesKHR(
                pdev, ms_surface, &count, support.modes.begin()));
        }
    }
    VkSurfaceFormatKHR ChooseSwapFormat(const SwapchainSupport& support)
    {
        const auto& formats = support.formats;
        if(formats.count() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
        {
            return preferredSwapFormats[0];
        }
        for(const auto& pref : preferredSwapFormats)
        {
            for(const auto& format : formats)
            {
                if( pref.format == format.format &&
                    pref.colorSpace == format.colorSpace)
                {
                    return format;
                }
            }
        }
        return formats[0];
    }
    VkPresentModeKHR ChoosePresentMode(const SwapchainSupport& support)
    {
        for(VkPresentModeKHR pref : preferredSwapModes)
        {
            for(VkPresentModeKHR mode : support.modes)
            {
                if(mode == pref)
                {
                    return mode;
                }
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }
    VkExtent2D ChooseSwapExtent(const SwapchainSupport& support)
    {
        VkExtent2D current = support.capabilities.currentExtent;
        if(current.width == 0xFFFFFFFF || current.height == 0xFFFFFFFF)
        {
            VkExtent2D maxExtent = support.capabilities.maxImageExtent;
            VkExtent2D minExtent = support.capabilities.minImageExtent;
            int32_t winWidth, winHeight;
            Window::GetSize(Window::GetActive(), winWidth, winHeight);
            current.width = Clamp((uint32_t)winWidth, minExtent.width, maxExtent.width);
            current.height = Clamp((uint32_t)winHeight, minExtent.height, maxExtent.height);
        }
        return current;
    }
    int32_t RateSwapchainSupport(const SwapchainSupport& support)
    {
        int32_t score = 0;
        int32_t maxImages = support.capabilities.maxImageCount;
        if(maxImages == 0)
        {
            maxImages = 10;
        }
        if(maxImages < 2)
        {
            return 0;
        }
        if(maxImages > 2)
        {
            score += 1000;
        }

        int32_t winWidth, winHeight;
        Window::GetSize(Window::GetActive(), winWidth, winHeight);
        VkExtent2D maxExtent = support.capabilities.maxImageExtent;
        VkExtent2D minExtent = support.capabilities.minImageExtent;
        if(maxExtent.width < winWidth || maxExtent.height < winHeight)
        {
            score -= 1000;
        }
        if(minExtent.width > winWidth || minExtent.height > winHeight)
        {
            score -= 1000;
        }

        if(support.formats.empty() || support.modes.empty())
        {
            return 0;
        }
        VkSurfaceFormatKHR format = ChooseSwapFormat(support);
        VkPresentModeKHR mode = ChoosePresentMode(support);
        for(int32_t i = 0; i < NELEM(preferredSwapFormats); ++i)
        {
            if(memcmp(&format, &preferredSwapFormats[i], sizeof(format)) == 0)
            {
                score += 1000 / (i + 1);
                break;
            }
        }
        for(int32_t i = 0; i < NELEM(preferredSwapModes); ++i)
        {
            if(mode == preferredSwapModes[i])
            {
                score += 1000 / (i + 1);
                break;
            }
        }

        return score;
    }
    void ChoosePhysicalDevice()
    {
        TempArray<VkPhysicalDevice>         pdevs;
        TempArray<VkQueueFamilyProperties>  fams;
        TempArray<VkExtensionProperties>    extProps;
        TempArray<int32_t>                  scores;
        SwapchainSupport                    scSupport;

        {
            uint32_t count = 0;
            CheckResult(vkEnumeratePhysicalDevices(ms_inst, &count, 0));
            pdevs.resize(count);
            scores.reserve(count);
            CheckResult(vkEnumeratePhysicalDevices(ms_inst, &count, pdevs.begin()));
        }

        for(VkPhysicalDevice pdev : pdevs)
        {
            int32_t score = RatePhysicalDevice(pdev);
            if(score < 1)
            {
                continue;
            }

            if(!DevExtensionSupport(pdev, extProps))
            {
                continue;
            }

            GetQFamProps(pdev, fams);
            int32_t presFam = FindQueueFamily(pdev, fams, 0, true);
            if(presFam < 0)
            {
                continue;
            }
            int32_t gfxFam = FindQueueFamily(pdev, fams, VK_QUEUE_GRAPHICS_BIT, false);
            if(gfxFam < 0)
            {
                continue;
            }
            int32_t compFam = FindQueueFamily(pdev, fams, VK_QUEUE_COMPUTE_BIT, false);
            if(compFam < 0)
            {
                continue;
            }
            int32_t xferFam = FindQueueFamily(pdev, fams, VK_QUEUE_TRANSFER_BIT, false);
            if(xferFam < 0)
            {
                continue;
            }

            GetSwapchainSupport(pdev, scSupport);
            int32_t scScore = RateSwapchainSupport(scSupport);
            if(scScore < 1)
            {
                continue;
            }

            score += scScore;

            scores.append() = score;
        }

        Assert(!scores.empty());
        ms_pdev = pdevs[scores.largest()];

        GetQFamProps(ms_pdev, fams);
        ms_qfams[QT_Graphics] = FindQueueFamily(ms_pdev, fams, VK_QUEUE_GRAPHICS_BIT, false);
        ms_qfams[QT_Compute]  = FindQueueFamily(ms_pdev, fams, VK_QUEUE_COMPUTE_BIT, false);
        ms_qfams[QT_Transfer] = FindQueueFamily(ms_pdev, fams, VK_QUEUE_TRANSFER_BIT, false);
        ms_qfams[QT_Present]  = FindQueueFamily(ms_pdev, fams, 0, true);
    }
    void CreateDevice()
    {
        VkDeviceQueueCreateInfo qcis[QT_Count];
        MemZero(qcis);
        float priority = 1.0f;
        for(int32_t i = 0; i < QT_Count; ++i)
        {
            qcis[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            qcis[i].queueFamilyIndex = ms_qfams[i];
            qcis[i].queueCount = 1;
            qcis[i].pQueuePriorities = &priority;
        }

        VkPhysicalDeviceFeatures pdevFeats;
        MemZero(pdevFeats);

        VkDeviceCreateInfo dci;
        MemZero(dci);
        dci.pQueueCreateInfos = qcis;
        dci.queueCreateInfoCount = NELEM(qcis);
        dci.pEnabledFeatures = &pdevFeats;
        dci.enabledExtensionCount = ms_devExtNames.count();
        dci.ppEnabledExtensionNames = ms_devExtNames.begin();
        dci.enabledLayerCount = ms_layerNames.count();
        dci.ppEnabledLayerNames = ms_layerNames.begin();
        CheckResult(vkCreateDevice(ms_pdev, &dci, 0, &ms_dev));

        for(int32_t i = 0; i < QT_Count; ++i)
        {
            vkGetDeviceQueue(ms_dev, ms_qfams[i], 0, &ms_queues[i]);
        }
    }
    void CreateAllocator()
    {
        VmaAllocatorCreateInfo allocatorInfo;
        MemZero(allocatorInfo);
        allocatorInfo.physicalDevice = ms_pdev;
        allocatorInfo.device = ms_dev;
        vmaCreateAllocator(&allocatorInfo, &ms_allocator);
    }
    void CreateSurface()
    {
        CheckResult(glfwCreateWindowSurface(
            ms_inst, 
            Window::GetActive(), 
            nullptr, 
            &ms_surface));
    }
    void CreateSwapchain()
    {
        SwapchainSupport scSupport;
        GetSwapchainSupport(ms_pdev, scSupport);
        ms_surfaceCaps      = scSupport.capabilities;
        ms_surfaceFormat    = ChooseSwapFormat(scSupport);
        ms_surfaceExtent    = ChooseSwapExtent(scSupport);
        ms_presentMode      = ChoosePresentMode(scSupport);
        uint32_t imgCount   = scSupport.capabilities.minImageCount + 1;
        if(scSupport.capabilities.maxImageCount != 0)
        {
            Assert(imgCount <= scSupport.capabilities.maxImageCount);
        }
        Assert(ms_surfaceExtent.width != 0);
        Assert(ms_surfaceExtent.height != 0);

        VkSwapchainCreateInfoKHR sci;
        MemZero(sci);
        sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        sci.surface             = ms_surface;
        sci.minImageCount       = imgCount;
        sci.imageFormat         = ms_surfaceFormat.format;
        sci.imageColorSpace     = ms_surfaceFormat.colorSpace;
        sci.imageExtent         = ms_surfaceExtent;
        sci.imageArrayLayers    = 1;
        sci.imageUsage          = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        sci.preTransform        = scSupport.capabilities.currentTransform;
        sci.compositeAlpha      = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        sci.presentMode         = ms_presentMode;
        sci.clipped             = VK_TRUE;
        sci.oldSwapchain        = VK_NULL_HANDLE;

        uint32_t qfams[] = { ms_qfams[QT_Graphics], ms_qfams[QT_Present] };
        if(qfams[0] == qfams[1])
        {
            sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
        else
        {
            sci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            sci.pQueueFamilyIndices = qfams;
            sci.queueFamilyIndexCount = NELEM(qfams);
        }

        CheckResult(vkCreateSwapchainKHR(ms_dev, &sci, nullptr, &ms_swapchain));

        CheckResult(vkGetSwapchainImagesKHR(
            ms_dev, ms_swapchain, &imgCount, nullptr));
        ms_swapImages.resize(imgCount);
        CheckResult(vkGetSwapchainImagesKHR(
            ms_dev, ms_swapchain, &imgCount, ms_swapImages.begin()));
    }
    void Shutdown()
    {
        vkDestroySwapchainKHR(ms_dev, ms_swapchain, 0);
        vkDestroyDevice(ms_dev, 0);
        vkDestroySurfaceKHR(ms_inst, ms_surface, 0);
        DVK_ONLY({
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                ms_inst, "vkDestroyDebugUtilsMessengerEXT");
            Assert(func != nullptr);
            func(ms_inst, ms_dbgMsger, 0);
        });
        vkDestroyInstance(ms_inst, 0);
    }
    void Begin()
    {
        
    }
    void End()
    {
        
    }
    void SetViewport(const vec4& viewport)
    {
        
    }
    void DrawBackground()
    {
        
    }
    void DrawTextured(
        Renderer::Buffer      verts,
        Renderer::Buffer      inds,
        Renderer::Texture     mat,
        Renderer::Texture     norm,
        const Textured::VSUniform& vsuni,
        const Textured::FSUniform& fsuni)
    {
        
    }
    void DrawFlat(
        Renderer::Buffer      verts,
        Renderer::Buffer      inds,
        const Flat::VSUniform& vsuni,
        const Flat::FSUniform& fsuni)
    {
        
    }
    Renderer::Buffer CreateBuffer(const Renderer::BufferDesc& desc)
    {
        return Renderer::Buffer();
    }
    void DestroyBuffer(Renderer::Buffer buffer)
    {

    }
    Renderer::Texture CreateTexture(const Renderer::TextureDesc& desc)
    {
        return Renderer::Texture();
    }
    void DestroyTexture(Renderer::Texture texture)
    {
        
    }
};

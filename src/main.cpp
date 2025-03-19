// Main File, implemented using an example for vulcan includen in the Dear ImGui
// repository

#include "character.h"
#include "dice.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "json.hpp" // TODO Character Save Files

#include <array>
#include <chrono>
#include <stdio.h> // printf, fprintf
#include <stdlib.h> // abort
#include <thread>
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Volk headers
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
#define VOLK_IMPLEMENTATION
#include <volk.h>
#endif

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to
// maximize ease of testing and compatibility with old VS compilers. To link
// with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project
// should not be affected, as you are likely to link with a newer binary of GLFW
// that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) &&                                 \
    !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

//#define APP_USE_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

// Data
static VkAllocationCallbacks *g_Allocator = nullptr;
static VkInstance g_Instance = VK_NULL_HANDLE;
static VkPhysicalDevice g_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice g_Device = VK_NULL_HANDLE;
static uint32_t g_QueueFamily = (uint32_t)-1;
static VkQueue g_Queue = VK_NULL_HANDLE;
static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
static VkPipelineCache g_PipelineCache = VK_NULL_HANDLE;
static VkDescriptorPool g_DescriptorPool = VK_NULL_HANDLE;

static ImGui_ImplVulkanH_Window g_MainWindowData;
static uint32_t g_MinImageCount = 2;
static bool g_SwapChainRebuild = false;

// Frame Rate Data
const int TARGET_FPS = 60;
const int FRAME_TIME_MS = 1000 / TARGET_FPS; // Time per frame in milliseconds

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}
static void check_vk_result(VkResult err) {
    if (err == VK_SUCCESS)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

#ifdef APP_USE_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
             uint64_t object, size_t location, int32_t messageCode,
             const char *pLayerPrefix, const char *pMessage, void *pUserData) {
    (void)flags;
    (void)object;
    (void)location;
    (void)messageCode;
    (void)pUserData;
    (void)pLayerPrefix; // Unused arguments
    fprintf(stderr,
            "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n",
            objectType, pMessage);
    return VK_FALSE;
}
#endif // APP_USE_VULKAN_DEBUG_REPORT

static bool
IsExtensionAvailable(const ImVector<VkExtensionProperties> &properties,
                     const char *extension) {
    for (const VkExtensionProperties &p : properties)
        if (strcmp(p.extensionName, extension) == 0)
            return true;
    return false;
}

static void SetupVulkan(ImVector<const char *> instance_extensions) {
    VkResult err;
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
    volkInitialize();
#endif

    // Create Vulkan Instance
    {
        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        // Enumerate available extensions
        uint32_t properties_count;
        ImVector<VkExtensionProperties> properties;
        vkEnumerateInstanceExtensionProperties(nullptr, &properties_count,
                                               nullptr);
        properties.resize(properties_count);
        err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count,
                                                     properties.Data);
        check_vk_result(err);

        // Enable required extensions
        if (IsExtensionAvailable(
                properties,
                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
            instance_extensions.push_back(
                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
        if (IsExtensionAvailable(
                properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
            instance_extensions.push_back(
                VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            create_info.flags |=
                VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }
#endif

        // Enabling validation layers
#ifdef APP_USE_VULKAN_DEBUG_REPORT
        const char *layers[] = {"VK_LAYER_KHRONOS_validation"};
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = layers;
        instance_extensions.push_back("VK_EXT_debug_report");
#endif

        // Create Vulkan Instance
        create_info.enabledExtensionCount = (uint32_t)instance_extensions.Size;
        create_info.ppEnabledExtensionNames = instance_extensions.Data;
        err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
        check_vk_result(err);
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
        volkLoadInstance(g_Instance);
#endif

        // Setup the debug report callback
#ifdef APP_USE_VULKAN_DEBUG_REPORT
        auto f_vkCreateDebugReportCallbackEXT =
            (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
                g_Instance, "vkCreateDebugReportCallbackEXT");
        IM_ASSERT(f_vkCreateDebugReportCallbackEXT != nullptr);
        VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
        debug_report_ci.sType =
            VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                                VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debug_report_ci.pfnCallback = debug_report;
        debug_report_ci.pUserData = nullptr;
        err = f_vkCreateDebugReportCallbackEXT(g_Instance, &debug_report_ci,
                                               g_Allocator, &g_DebugReport);
        check_vk_result(err);
#endif
    }

    // Select Physical Device (GPU)
    g_PhysicalDevice = ImGui_ImplVulkanH_SelectPhysicalDevice(g_Instance);
    IM_ASSERT(g_PhysicalDevice != VK_NULL_HANDLE);

    // Select graphics queue family
    g_QueueFamily = ImGui_ImplVulkanH_SelectQueueFamilyIndex(g_PhysicalDevice);
    IM_ASSERT(g_QueueFamily != (uint32_t)-1);

    // Create Logical Device (with 1 queue)
    {
        ImVector<const char *> device_extensions;
        device_extensions.push_back("VK_KHR_swapchain");

        // Enumerate physical device extension
        uint32_t properties_count;
        ImVector<VkExtensionProperties> properties;
        vkEnumerateDeviceExtensionProperties(g_PhysicalDevice, nullptr,
                                             &properties_count, nullptr);
        properties.resize(properties_count);
        vkEnumerateDeviceExtensionProperties(
            g_PhysicalDevice, nullptr, &properties_count, properties.Data);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
        if (IsExtensionAvailable(properties,
                                 VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
            device_extensions.push_back(
                VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

        const float queue_priority[] = {1.0f};
        VkDeviceQueueCreateInfo queue_info[1] = {};
        queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[0].queueFamilyIndex = g_QueueFamily;
        queue_info[0].queueCount = 1;
        queue_info[0].pQueuePriorities = queue_priority;
        VkDeviceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount =
            sizeof(queue_info) / sizeof(queue_info[0]);
        create_info.pQueueCreateInfos = queue_info;
        create_info.enabledExtensionCount = (uint32_t)device_extensions.Size;
        create_info.ppEnabledExtensionNames = device_extensions.Data;
        err = vkCreateDevice(g_PhysicalDevice, &create_info, g_Allocator,
                             &g_Device);
        check_vk_result(err);
        vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);
    }

    // Create Descriptor Pool
    // If you wish to load e.g. additional textures you may need to alter pools
    // sizes and maxSets.
    {
        VkDescriptorPoolSize pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
             IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE},
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 0;
        for (VkDescriptorPoolSize &pool_size : pool_sizes)
            pool_info.maxSets += pool_size.descriptorCount;
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        err = vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator,
                                     &g_DescriptorPool);
        check_vk_result(err);
    }
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used
// by the demo. Your real engine/app may not use them.
static void SetupVulkanWindow(ImGui_ImplVulkanH_Window *wd,
                              VkSurfaceKHR surface, int width, int height) {
    wd->Surface = surface;

    // Check for WSI support
    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_QueueFamily,
                                         wd->Surface, &res);
    if (res != VK_TRUE) {
        fprintf(stderr, "Error no WSI support on physical device 0\n");
        exit(-1);
    }

    // Select Surface Format
    const VkFormat requestSurfaceImageFormat[] = {
        VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
    const VkColorSpaceKHR requestSurfaceColorSpace =
        VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
        g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat,
        (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat),
        requestSurfaceColorSpace);

    // Select Present Mode
#ifdef APP_USE_UNLIMITED_FRAME_RATE
    VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_MAILBOX_KHR,
                                        VK_PRESENT_MODE_IMMEDIATE_KHR,
                                        VK_PRESENT_MODE_FIFO_KHR};
#else
    VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
#endif
    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
        g_PhysicalDevice, wd->Surface, &present_modes[0],
        IM_ARRAYSIZE(present_modes));
    // printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    IM_ASSERT(g_MinImageCount >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(
        g_Instance, g_PhysicalDevice, g_Device, wd, g_QueueFamily, g_Allocator,
        width, height, g_MinImageCount);
}

static void CleanupVulkan() {
    vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);

#ifdef APP_USE_VULKAN_DEBUG_REPORT
    // Remove the debug report callback
    auto f_vkDestroyDebugReportCallbackEXT =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
            g_Instance, "vkDestroyDebugReportCallbackEXT");
    f_vkDestroyDebugReportCallbackEXT(g_Instance, g_DebugReport, g_Allocator);
#endif // APP_USE_VULKAN_DEBUG_REPORT

    vkDestroyDevice(g_Device, g_Allocator);
    vkDestroyInstance(g_Instance, g_Allocator);
}

static void CleanupVulkanWindow() {
    ImGui_ImplVulkanH_DestroyWindow(g_Instance, g_Device, &g_MainWindowData,
                                    g_Allocator);
}

static void FrameRender(ImGui_ImplVulkanH_Window *wd, ImDrawData *draw_data) {
    VkSemaphore image_acquired_semaphore =
        wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore =
        wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkResult err = vkAcquireNextImageKHR(g_Device, wd->Swapchain, UINT64_MAX,
                                         image_acquired_semaphore,
                                         VK_NULL_HANDLE, &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        g_SwapChainRebuild = true;
    if (err == VK_ERROR_OUT_OF_DATE_KHR)
        return;
    if (err != VK_SUBOPTIMAL_KHR)
        check_vk_result(err);

    ImGui_ImplVulkanH_Frame *fd = &wd->Frames[wd->FrameIndex];
    {
        err = vkWaitForFences(
            g_Device, 1, &fd->Fence, VK_TRUE,
            UINT64_MAX); // wait indefinitely instead of periodically checking
        check_vk_result(err);

        err = vkResetFences(g_Device, 1, &fd->Fence);
        check_vk_result(err);
    }
    {
        err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        check_vk_result(err);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info,
                             VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        check_vk_result(err);
        err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
        check_vk_result(err);
    }
}

static void FramePresent(ImGui_ImplVulkanH_Window *wd) {
    if (g_SwapChainRebuild)
        return;
    VkSemaphore render_complete_semaphore =
        wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(g_Queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        g_SwapChainRebuild = true;
    if (err == VK_ERROR_OUT_OF_DATE_KHR)
        return;
    if (err != VK_SUBOPTIMAL_KHR)
        check_vk_result(err);
    wd->SemaphoreIndex =
        (wd->SemaphoreIndex + 1) %
        wd->SemaphoreCount; // Now we can use the next set of semaphores
}

// Main code
int main(int, char **) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Create window with Vulkan context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window = glfwCreateWindow(
        1280, 720, "Down and Dirty DnD Machine v0.0.2", nullptr, nullptr);
    if (!glfwVulkanSupported()) {
        printf("GLFW: Vulkan Not Supported\n");
        return 1;
    }

    ImVector<const char *> extensions;
    uint32_t extensions_count = 0;
    const char **glfw_extensions =
        glfwGetRequiredInstanceExtensions(&extensions_count);
    for (uint32_t i = 0; i < extensions_count; i++)
        extensions.push_back(glfw_extensions[i]);
    SetupVulkan(extensions);

    // Create Window Surface
    VkSurfaceKHR surface;
    VkResult err =
        glfwCreateWindowSurface(g_Instance, window, g_Allocator, &surface);
    check_vk_result(err);

    // Create Framebuffers
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    ImGui_ImplVulkanH_Window *wd = &g_MainWindowData;
    SetupVulkanWindow(wd, surface, w, h);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    // init_info.ApiVersion = VK_API_VERSION_1_3;              
    // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default
    // to header version.
    init_info.Instance = g_Instance;
    init_info.PhysicalDevice = g_PhysicalDevice;
    init_info.Device = g_Device;
    init_info.QueueFamily = g_QueueFamily;
    init_info.Queue = g_Queue;
    init_info.PipelineCache = g_PipelineCache;
    init_info.DescriptorPool = g_DescriptorPool;
    init_info.RenderPass = wd->RenderPass;
    init_info.Subpass = 0;
    init_info.MinImageCount = g_MinImageCount;
    init_info.ImageCount = wd->ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = g_Allocator;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can
    // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
    // them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
    // need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr.
    // Please handle those errors in your application (e.g. use an assertion, or
    // display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and
    // stored into a texture when calling
    // ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame
    // below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use
    // Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string
    // literal you need to write a double backslash \\ !
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("imgui/misc/fonts/Roboto-Medium.ttf", 16.0f);
    io.Fonts->AddFontFromFileTTF("imgui/misc/fonts/Cousine-Regular.ttf", 15.0f);
    // ImFont* font =
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
    // nullptr, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = false;
    bool show_dice_box = false;
    bool show_settings = false;
    bool show_character = true;
    ImVec4 clear_color = ImVec4(0.132f, 0.257f, 0.151f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Set Clock For FPS limiter
        auto frameStart = std::chrono::high_resolution_clock::now();

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
        // tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data
        // to your main application, or clear/overwrite your copy of the mouse
        // data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
        // data to your main application, or clear/overwrite your copy of the
        // keyboard data. Generally you may always pass all inputs to dear
        // imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Resize swap chain?
        int fb_width, fb_height;
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        if (fb_width > 0 && fb_height > 0 &&
            (g_SwapChainRebuild || g_MainWindowData.Width != fb_width ||
             g_MainWindowData.Height != fb_height)) {
            ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
            ImGui_ImplVulkanH_CreateOrResizeWindow(
                g_Instance, g_PhysicalDevice, g_Device, &g_MainWindowData,
                g_QueueFamily, g_Allocator, fb_width, fb_height,
                g_MinImageCount);
            g_MainWindowData.FrameIndex = 0;
            g_SwapChainRebuild = false;
        }
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        /* Window for Opening Other Windows */
        if (true) {
            ImGui::Begin("Tools");

            if (ImGui::Button("Dice Box"))
                show_dice_box = true;
            if (ImGui::Button("Customization & Debug"))
                show_settings = true;
            if (ImGui::Button("Characters"))
                show_character = true;

            ImGui::End();
        }

        static Character myCharacter;

        /* ----- Window for Character Stats ----- */
        if (show_character) // TODO savefiles, race, and class in JSON
        {
            ImGui::Begin("Character(s)", &show_character);

            if (ImGui::BeginTabBar("MyTabBar")) {
                if (ImGui::BeginTabItem(myCharacter.getName().c_str())) {
                    ImGui::Text("Name: ");
                    ImGui::SameLine();
                    ImGui::Text("%s", myCharacter.getName().c_str());
                    ImGui::Text("Race: ");
                    ImGui::SameLine();
                    ImGui::Text("%s", myCharacter.getRace().c_str());

                    // Display list of classes and their respective levels
                    ImGui::Text("Class:");
                    for (const auto &[className, level] :
                         myCharacter.getClassLevels()) {
                        ImGui::SameLine();
                        ImGui::Text("%s(%d) ", className.c_str(), level);
                    }

                    /* Ability Table */
                    if (ImGui::BeginTable("AbilityScoresTable", 7,
                                          ImGuiTableFlags_Borders |
                                              ImGuiTableFlags_RowBg)) {
                        // Column headers (Ability Names)
                        ImGui::TableSetupColumn("##");
                        const char *abilityNames[] = {"STR", "DEX", "CON",
                                                      "INT", "WIS", "CHR"};
                        for (const char *name : abilityNames) {
                            ImGui::TableSetupColumn(name);
                        }
                        ImGui::TableHeadersRow(); // Render headers

                        // Display ability scores
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Score:");
                        for (int i = 0; i < 6; ++i) {
                            int score = 0;
                            switch (i) {
                            case 0:
                                score = myCharacter.getStr();
                                break;
                            case 1:
                                score = myCharacter.getDex();
                                break;
                            case 2:
                                score = myCharacter.getCon();
                                break;
                            case 3:
                                score = myCharacter.getInt();
                                break;
                            case 4:
                                score = myCharacter.getWis();
                                break;
                            case 5:
                                score = myCharacter.getChr();
                                break;
                            }
                            ImGui::TableSetColumnIndex(i + 1);
                            ImGui::Text("%d", score);
                        }

                        // Display ability score modifiers
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Mod:");
                        for (int i = 0; i < 6; ++i) {
                            int mod = 0;
                            switch (i) {
                            case 0:
                                mod = myCharacter.getAbilityMod(
                                    myCharacter.getStr());
                                break;
                            case 1:
                                mod = myCharacter.getAbilityMod(
                                    myCharacter.getDex());
                                break;
                            case 2:
                                mod = myCharacter.getAbilityMod(
                                    myCharacter.getCon());
                                break;
                            case 3:
                                mod = myCharacter.getAbilityMod(
                                    myCharacter.getInt());
                                break;
                            case 4:
                                mod = myCharacter.getAbilityMod(
                                    myCharacter.getWis());
                                break;
                            case 5:
                                mod = myCharacter.getAbilityMod(
                                    myCharacter.getChr());
                                break;
                            }
                            ImGui::TableSetColumnIndex(i + 1);
                            ImGui::Text("%+d", mod);
                        }

                        ImGui::EndTable();

                        ImGui::Spacing();

                        std::map<std::string, bool> saves =
                            myCharacter.getSavingThrows();
                        std::map<std::string, bool> proficiencies =
                            myCharacter.getProficiencies();

                        /* Table for Saving Throws */
                        if (ImGui::BeginTable("SavingThrows", 3,
                                              ImGuiTableFlags_Borders |
                                                  ImGuiTableFlags_RowBg)) {
                            ImGui::TableSetupColumn(
                                "Saving Throws:",
                                ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableSetupColumn(
                                "Proficient?",
                                ImGuiTableColumnFlags_WidthFixed);
                            ImGui::TableSetupColumn("Bonus:");
                            ImGui::TableHeadersRow();

                            for (auto &[save, isProficient] : saves) {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::Text("%s", save.c_str());

                                ImGui::TableSetColumnIndex(1);
                                ImGui::Checkbox(("##save_" + save).c_str(),
                                                myCharacter.getSave(save));

                                int bonus = myCharacter.getAbilityMod(
                                    myCharacter.getAbility(save));
                                if (isProficient)
                                    bonus += myCharacter.getProfBonus();
                                ImGui::TableSetColumnIndex(2);
                                ImGui::Text("%+d", bonus);
                            }
                            ImGui::EndTable();
                        }

                        ImGui::Spacing();

                        /* Table for Proficiencies */
                        if (ImGui::BeginTable("Proficiencies", 3,
                                              ImGuiTableFlags_Borders |
                                                  ImGuiTableFlags_RowBg)) {
                            ImGui::TableSetupColumn(
                                "Skills:", ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableSetupColumn(
                                "Proficient?",
                                ImGuiTableColumnFlags_WidthFixed);
                            ImGui::TableSetupColumn("Bonus:");
                            ImGui::TableHeadersRow();

                            for (auto &[skill, isProficient] : proficiencies) {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::Text("%s", skill.c_str());

                                ImGui::TableSetColumnIndex(1);
                                ImGui::Checkbox(("##prof_" + skill).c_str(),
                                                myCharacter.getProf(skill));

                                int bonus = 0;

                                map<string, string> table =
                                    myCharacter.getSkillToAbility();

                                auto it = table.find(skill);

                                if (it != table.end()) {
                                    string abilityName =
                                        it->second; // e.g., "dexterity",
                                                    // "wisdom"

                                    // Get the ability score and calculate the
                                    // modifier
                                    int abilityScore =
                                        myCharacter.getAbility(abilityName);

                                    // Add ability modifier
                                    bonus +=
                                        myCharacter.getAbilityMod(abilityScore);
                                }
                                if (isProficient)
                                    bonus += myCharacter.getProfBonus();

                                ImGui::TableSetColumnIndex(2);
                                ImGui::Text("%+d", bonus);
                            }
                            ImGui::EndTable();
                        }
                    }

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Tab 2")) {
                    ImGui::Text("This is the second tab.");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Tab 3")) {
                    ImGui::Text("This is the third tab.");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::End();
        }

        /* ----- Window for Dice Rolls ----- */
        if (show_dice_box) {
            static std::array<int, 8> dice = {
                0, 0, 0, 0, 0, 0, 0, 0}; // Number of dice to roll
                                         // {d100,d20,d12,d10,d8,d6,d4,custom}
            static int customDie = 0;
            static int totalRoll = 0;

            static map<int, vector<int>> roll_results = {
                // Stores results of individual rolls for each die type
                {100, {}}, {20, {}}, {12, {}}, {10, {}},
                {8, {}},   {6, {}},  {4, {}},  {0, {}} // (0 = custom)
            };

            ImGui::Begin("Dice Box", &show_dice_box);

            vector<std::pair<int, const char *>> diceTypes = {
                {100, "D-100s:"}, {20, "D-20s:"}, {12, "D-12s:"},
                {10, "D-10s:"},   {8, "D-8s:"},   {6, "D-6s:"},
                {4, "D-4s:"}};

            ImGui::Text("D-100s:"); // D100 increment/decrement
            ImGui::SameLine();
            ImGui::Text("%d", dice[0]);
            ImGui::SameLine();
            if (ImGui::Button("-##d100")) {
                if (dice[0] > 0)
                    dice[0]--;
                else
                    dice[0] = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("+##d100"))
                dice[0]++;

            ImGui::Text("D-20s: "); // D20 increment/decrement
            ImGui::SameLine();
            ImGui::Text("%d", dice[1]);
            ImGui::SameLine();
            if (ImGui::Button("-##d20")) {
                if (dice[1] > 0)
                    dice[1]--;
                else
                    dice[1] = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("+##d20"))
                dice[1]++;

            ImGui::Text("D-12s: "); // D12 increment/decrement
            ImGui::SameLine();
            ImGui::Text("%d", dice[2]);
            ImGui::SameLine();
            if (ImGui::Button("-##d12")) {
                if (dice[2] > 0)
                    dice[2]--;
                else
                    dice[2] = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("+##d12"))
                dice[2]++;

            ImGui::Text("D-10s: "); // D10 increment/decrement
            ImGui::SameLine();
            ImGui::Text("%d", dice[3]);
            ImGui::SameLine();
            if (ImGui::Button("-##d10")) {
                if (dice[3] > 0)
                    dice[3]--;
                else
                    dice[3] = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("+##d10"))
                dice[3]++;

            ImGui::Text("D-8s:  "); // D8 increment/decrement
            ImGui::SameLine();
            ImGui::Text("%d", dice[4]);
            ImGui::SameLine();
            if (ImGui::Button("-##d8")) {
                if (dice[4] > 0)
                    dice[4]--;
                else
                    dice[4] = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("+##d8"))
                dice[4]++;

            ImGui::Text("D-6s:  "); // D6 increment/decrement
            ImGui::SameLine();
            ImGui::Text("%d", dice[5]);
            ImGui::SameLine();
            if (ImGui::Button("-##d6")) {
                if (dice[5] > 0)
                    dice[5]--;
                else
                    dice[5] = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("+##d6"))
                dice[5]++;

            ImGui::Text("D-4s:  "); // D4 increment/decrement
            ImGui::SameLine();
            ImGui::Text("%d", dice[6]);
            ImGui::SameLine();
            if (ImGui::Button("-##d4")) {
                if (dice[6] > 0)
                    dice[6]--;
                else
                    dice[6] = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("+##d4"))
                dice[6]++;

            ImGui::Text("D-Custom"); // Custom Die increment/decrement
            ImGui::SameLine();
            ImGui::SetNextItemWidth(50);
            ImGui::InputInt(":##NoStepButtons", &customDie, 0, 0,
                            ImGuiInputTextFlags_None);
            ImGui::SameLine();
            ImGui::Text("%d", dice[7]);
            ImGui::SameLine();
            if (ImGui::Button("-##dC")) {
                if (dice[7] > 0)
                    dice[7]--;
                else
                    dice[7] = 0;
            }
            ImGui::SameLine();
            if (ImGui::Button("+##dC"))
                dice[7]++;

            // Roll button
            if (ImGui::Button("Roll")) {
                totalRoll = 0; // Reset total roll
                for (auto &[dieType, rolls] : roll_results) {
                    rolls.clear(); // Clear previous results
                }

                // Roll each dice type
                for (size_t i = 0; i < diceTypes.size(); ++i) {
                    int dieType = diceTypes[i].first;
                    for (int j = 0; j < dice[i]; j++) {
                        int roll = 0;
                        switch (dieType) {
                        case 100:
                            roll = d_100();
                            break;
                        case 20:
                            roll = d_20();
                            break;
                        case 12:
                            roll = d_12();
                            break;
                        case 10:
                            roll = d_10();
                            break;
                        case 8:
                            roll = d_8();
                            break;
                        case 6:
                            roll = d_6();
                            break;
                        case 4:
                            roll = d_4();
                            break;
                        }
                        roll_results[dieType].push_back(roll);
                        totalRoll += roll;
                    }
                }

                // Roll custom dice
                for (int j = 0; j < dice[7]; j++) {
                    int roll = d_custom(customDie);
                    roll_results[0].push_back(roll);
                    totalRoll += roll;
                }
            }

            ImGui::SameLine();
            ImGui::Text("Total = %d", totalRoll);

            // Display roll results
            if (!roll_results.empty()) {
                ImGui::Text("Roll Results:");
                ImGui::BeginChild("RollResults", ImVec2(0, 200), true);

                for (const auto &[dieType, rolls] : roll_results) {
                    if (!rolls.empty()) {
                        if (dieType == 0) {
                            ImGui::Text("D-%d:", customDie);
                        } else {
                            ImGui::Text("D-%d:", dieType);
                        }

                        for (size_t i = 0; i < rolls.size(); i++) {
                            ImGui::SameLine();
                            ImGui::Text("%d ", rolls[i]);
                        }
                        // ImGui::NewLine(); // Separate dice types
                    }
                }

                ImGui::EndChild();
            }

            ImGui::End();
        }

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        /* ----- Settings & Debug ----- */
        if (show_settings) {

            ImGui::Begin("Customization & Debug", &show_settings);

            if (ImGui::Button("ImGui Demo Window"))
                show_demo_window = true;

            ImGui::ColorEdit3(
                "clear color",
                (float *)&clear_color); // Edit 3 floats representing a
                                        // color for the background

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        1000.0f / io.Framerate,
                        io.Framerate); // Display framerate
            ImGui::End();
        }

        /* Rendering */
        ImGui::Render();
        ImDrawData *draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f ||
                                   draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized) {
            wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
            wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
            wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
            wd->ClearValue.color.float32[3] = clear_color.w;
            FrameRender(wd, draw_data);
            FramePresent(wd);
        }

        /* ----- Frame Rate Limiter ----- */
        auto frameEnd = std::chrono::high_resolution_clock::now();
        auto elapsedTime =
            std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd -
                                                                  frameStart)
                .count();

        if (elapsedTime < FRAME_TIME_MS) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(FRAME_TIME_MS - elapsedTime));
        }
    }

    // Cleanup
    err = vkDeviceWaitIdle(g_Device);
    check_vk_result(err);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    CleanupVulkanWindow();
    CleanupVulkan();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

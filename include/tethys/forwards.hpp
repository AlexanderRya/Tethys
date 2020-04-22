#ifndef TETHYS_FORWARDS_HPP
#define TETHYS_FORWARDS_HPP

struct GLFWwindow;

namespace vk {
    class Instance;
    class DebugUtilsMessengerEXT;
    class SurfaceKHR;
    class DescriptorPool;
    class CommandPool;
    class CommandBuffer;
    class RenderPass;
    class Framebuffer;
    class Fence;
    class Semaphore;
    class Sampler;
    class Pipeline;
} // namespace vk

namespace tethys {
    struct Mesh;
    struct Vertex;
    struct Texture;
    struct PipelineLayout;
    struct PointLight;
    struct DirectionalLight;
    struct RenderData;
} // namespace tethys

namespace tethys::api {
    struct Context;
    struct Device;
    struct Swapchain;
    struct StaticBuffer;
    class DescriptorSet;
    struct Image;
    template <typename>
    class Buffer;
    template <typename>
    class SingleBuffer;
} // namespace tethys::api

namespace tethys::window {
    struct Window;
} // namespace tethys::window

#endif //TETHYS_FORWARDS_HPP

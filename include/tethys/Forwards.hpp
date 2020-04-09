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
} // namespace vk

namespace tethys {
    struct Mesh;
    struct Vertex;
} // namespace tethys

namespace tethys::api {
    struct Context;
    struct Device;
    struct Swapchain;
    struct Buffer;
    class DescriptorSet;
    struct Image;
    class MappedBuffer;
    class SingleMappedBuffer;
    struct Pipeline;
    struct PipelineLayout;
} // namespace tethys::api

namespace tethys::window {
    struct Window;
} // namespace tethys::window

#endif //TETHYS_FORWARDS_HPP

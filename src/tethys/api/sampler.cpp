#include <tethys/api/sampler.hpp>
#include <tethys/api/context.hpp>

namespace tethys::api {
    vk::Sampler make_default_sampler() {
        vk::SamplerCreateInfo info{}; {
            info.magFilter = vk::Filter::eLinear;
            info.minFilter = vk::Filter::eLinear;
            info.addressModeU = vk::SamplerAddressMode::eRepeat;
            info.addressModeV = vk::SamplerAddressMode::eRepeat;
            info.addressModeW = vk::SamplerAddressMode::eRepeat;
            info.borderColor = vk::BorderColor::eIntOpaqueBlack;
            info.anisotropyEnable = true;
            info.maxAnisotropy = 16;
            info.compareEnable = false;
            info.compareOp = {};
            info.mipmapMode = {};
            info.unnormalizedCoordinates = false;
        }

        return ctx.device.logical.createSampler(info, nullptr, ctx.dispatcher);
    }
} // namespace tethys::api
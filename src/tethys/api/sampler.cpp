#include <tethys/api/sampler.hpp>
#include <tethys/api/context.hpp>

namespace tethys::api {
    static vk::Sampler default_sampler{};
    static vk::Sampler depth_sampler{};

    [[nodiscard]] static vk::Sampler make_default_sampler() {
        vk::SamplerCreateInfo info{}; {
            info.magFilter = vk::Filter::eLinear;
            info.minFilter = vk::Filter::eLinear;
            info.addressModeU = vk::SamplerAddressMode::eRepeat;
            info.addressModeV = vk::SamplerAddressMode::eRepeat;
            info.addressModeW = vk::SamplerAddressMode::eRepeat;
            info.anisotropyEnable = true;
            info.maxAnisotropy = 16;
            info.borderColor = vk::BorderColor::eFloatOpaqueWhite;
            info.unnormalizedCoordinates = false;
            info.compareEnable = false;
            info.compareOp = vk::CompareOp::eAlways;
            info.mipmapMode = vk::SamplerMipmapMode::eLinear;
            info.minLod = 0;
            info.maxLod = 1;
            info.mipLodBias = 0;
        }

        return context.device.logical.createSampler(info, nullptr, context.dispatcher);
    }

    [[nodiscard]] static vk::Sampler make_depth_sampler() {
        vk::SamplerCreateInfo info{}; {
            info.magFilter = vk::Filter::eLinear;
            info.minFilter = vk::Filter::eLinear;
            info.addressModeU = vk::SamplerAddressMode::eClampToBorder;
            info.addressModeV = vk::SamplerAddressMode::eClampToBorder;
            info.addressModeW = vk::SamplerAddressMode::eClampToBorder;
            info.anisotropyEnable = false;
            info.maxAnisotropy = 1;
            info.borderColor = vk::BorderColor::eFloatOpaqueWhite;
            info.unnormalizedCoordinates = false;
            info.compareEnable = false;
            info.compareOp = vk::CompareOp::eAlways;
            info.mipmapMode = vk::SamplerMipmapMode::eLinear;
            info.minLod = 0;
            info.maxLod = 1;
            info.mipLodBias = 0;
        }

        return context.device.logical.createSampler(info, nullptr, context.dispatcher);
    }

    void make_samplers() {
        default_sampler = make_default_sampler();
        depth_sampler = make_depth_sampler();
    }

    vk::Sampler sampler_from_type(const SamplerType& type) {
        switch (type) {
            case SamplerType::eDefault:
                return default_sampler;
            case SamplerType::eDepth:
                return depth_sampler;
            default:
                return nullptr;
        }
    }
} // namespace tethys::api
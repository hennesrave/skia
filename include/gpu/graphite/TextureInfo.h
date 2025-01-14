/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_TextureInfo_DEFINED
#define skgpu_graphite_TextureInfo_DEFINED

#include "include/core/SkString.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/private/base/SkAPI.h"
#include "include/private/base/SkAnySubclass.h"

#ifdef SK_DAWN
#include "include/private/gpu/graphite/DawnTypesPriv.h"
#endif

#ifdef SK_VULKAN
#include "include/private/gpu/graphite/VulkanGraphiteTypesPriv.h"
#endif

struct SkISize;

namespace skgpu::graphite {

class TextureInfoData;

#if defined(SK_METAL) && !defined(SK_DISABLE_LEGACY_TEXTURE_INFO_FUNCS)
struct MtlTextureInfo;
#endif

class SK_API TextureInfo {
public:
    TextureInfo();
#ifdef SK_DAWN
    TextureInfo(const DawnTextureInfo& dawnInfo);
#endif

#if defined(SK_METAL) && !defined(SK_DISABLE_LEGACY_TEXTURE_INFO_FUNCS)
    TextureInfo(const MtlTextureInfo& mtlInfo);
#endif

#ifdef SK_VULKAN
    TextureInfo(const VulkanTextureInfo& vkInfo);
#endif

    ~TextureInfo();
    TextureInfo(const TextureInfo&);
    TextureInfo& operator=(const TextureInfo&);

    bool operator==(const TextureInfo&) const;
    bool operator!=(const TextureInfo& that) const { return !(*this == that); }

    bool isValid() const { return fValid; }
    BackendApi backend() const { return fBackend; }

    uint32_t numSamples() const { return fSampleCount; }
    Mipmapped mipmapped() const { return fMipmapped; }
    Protected isProtected() const { return fProtected; }
    SkTextureCompressionType compressionType() const;

#ifdef SK_DAWN
    bool getDawnTextureInfo(DawnTextureInfo* info) const;
#endif

#ifdef SK_VULKAN
    bool getVulkanTextureInfo(VulkanTextureInfo* info) const {
        if (!this->isValid() || fBackend != BackendApi::kVulkan) {
            return false;
        }
        *info = VulkanTextureSpecToTextureInfo(fVkSpec, fSampleCount, fMipmapped);
        return true;
    }
#endif

    bool isCompatible(const TextureInfo& that) const;
    // Return a string containing the full description of this TextureInfo.
    SkString toString() const;
    // Return a string containing only the info relevant for its use as a RenderPass attachment.
    SkString toRPAttachmentString() const;

private:
    friend class TextureInfoData;
    friend class TextureInfoPriv;

    // Size determined by looking at the TextureInfoData subclasses, then guessing-and-checking.
    // Compiler will complain if this is too small - in that case, just increase the number.
    inline constexpr static size_t kMaxSubclassSize = 40;
    using AnyTextureInfoData = SkAnySubclass<TextureInfoData, kMaxSubclassSize>;

    template <typename SomeTextureInfoData>
    TextureInfo(BackendApi backend,
                uint32_t sampleCount,
                skgpu::Mipmapped mipped,
                skgpu::Protected isProtected,
                const SomeTextureInfoData& textureInfoData)
            : fBackend(backend)
            , fValid(true)
            , fSampleCount(sampleCount)
            , fMipmapped(mipped)
            , fProtected(isProtected) {
        fTextureInfoData.emplace<SomeTextureInfoData>(textureInfoData);
    }

    friend size_t ComputeSize(SkISize dimensions, const TextureInfo&);  // for bytesPerPixel

    size_t bytesPerPixel() const;

#ifdef SK_DAWN
    friend class DawnCaps;
    friend class DawnCommandBuffer;
    friend class DawnComputePipeline;
    friend class DawnGraphicsPipeline;
    friend class DawnResourceProvider;
    friend class DawnTexture;
    const DawnTextureSpec& dawnTextureSpec() const {
        SkASSERT(fValid && fBackend == BackendApi::kDawn);
        return fDawnSpec;
    }
#endif

#ifdef SK_VULKAN
    friend class VulkanCaps;
    friend class VulkanResourceProvider;
    friend class VulkanTexture;
    // For querying texture for YCbCr information when generating a PaintParamsKey
    friend class PaintParamsKey;

    const VulkanTextureSpec& vulkanTextureSpec() const {
        SkASSERT(fValid && fBackend == BackendApi::kVulkan);
        return fVkSpec;
    }
#endif

    BackendApi fBackend = BackendApi::kMock;
    bool fValid = false;

    uint32_t fSampleCount = 1;
    Mipmapped fMipmapped = Mipmapped::kNo;
    Protected fProtected = Protected::kNo;

    AnyTextureInfoData fTextureInfoData;

    union {
#ifdef SK_DAWN
        DawnTextureSpec fDawnSpec;
#endif
#ifdef SK_VULKAN
        VulkanTextureSpec fVkSpec;
#endif
        void* fEnsureUnionNonEmpty;
    };
};

}  // namespace skgpu::graphite

#endif  //skgpu_graphite_TextureInfo_DEFINED

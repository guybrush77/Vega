#pragma once

#include "etna/buffer.hpp"
#include "etna/command.hpp"
#include "etna/device.hpp"
#include "etna/image.hpp"
#include "etna/queue.hpp"

#include <map>
#include <string>

class TextureLoader {
  public:
    TextureLoader(etna::Device device, etna::Queue transfer_queue);

    TextureLoader(const TextureLoader&) = delete;
    TextureLoader& operator=(const TextureLoader&) = delete;

    TextureLoader(TextureLoader&&) = default;
    TextureLoader& operator=(TextureLoader&&) = default;

    void LoadAsync(const std::string& base, const std::string& file);

    void UploadAsync();

    void CleanAfterUpload();

    auto GetImage(const std::string& image) -> etna::ImageView2D;

    auto GetDefaultImage() -> etna::ImageView2D;

  private:
    struct StageBuffer final {
        etna::UniqueBuffer buffer;
        uint32_t           width;
        uint32_t           height;
    };

    struct ImageRecord final {
        etna::UniqueImage2D     image;
        etna::UniqueImageView2D view;
    };

    etna::Device              m_device;
    etna::Queue               m_transfer_queue;
    etna::UniqueCommandPool   m_command_pool;
    etna::UniqueCommandBuffer m_command_buffer;

    std::map<std::string, StageBuffer> m_host_buffers;
    std::map<std::string, ImageRecord> m_gpu_images;
};

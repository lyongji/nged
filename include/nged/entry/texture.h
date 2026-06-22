#pragma once

#include <imgui.h>

#include <stdint.h>
#include <memory>

namespace nged {

class Texture;
using TexturePtr = std::shared_ptr<Texture>;

class TextureResource;

class Texture final
{
  TextureResource* resource_ = nullptr;
  Texture(Texture const&) = delete;
public:
  Texture(TextureResource* resource): resource_(resource) {}
  ~Texture() { release(); }
  void release();
  ImTextureID id() const;
};

enum class AddressMode
{
  Repeat, Clamp, Border
};

enum class FilterMode
{
  Nearest, Linear
};

// upload r8g8b8a8 texture
TexturePtr uploadTexture(
  uint8_t const* data,
  int width,
  int height,
  AddressMode address = AddressMode::Repeat,
  FilterMode filter = FilterMode::Linear
);

}

// NGED Raylib backend: texture upload

#include "texture.h"
#include "raylib.h"
#include "rlgl.h"

namespace nged {

class TextureResource
{
public:
    Texture2D texture_;
};

ImTextureID Texture::id() const
{
    return (ImTextureID)(intptr_t)(resource_->texture_.id);
}

void Texture::release()
{
    if (resource_)
    {
        UnloadTexture(resource_->texture_);
        delete resource_;
        resource_ = nullptr;
    }
}

TexturePtr uploadTexture(
    uint8_t const* data, int width, int height,
    AddressMode address, FilterMode filter)
{
    TextureResource* resource = new TextureResource();

    // Upload pixel data to an Image, then to GPU texture
    Image image = {0};
    image.data    = (void*)data;
    image.width   = width;
    image.height  = height;
    image.mipmaps = 1;
    image.format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    resource->texture_ = LoadTextureFromImage(image);

    int addressFlag = RL_TEXTURE_WRAP_REPEAT;
    int filterFlag  = RL_TEXTURE_FILTER_LINEAR;
    if (address == AddressMode::Clamp)
        addressFlag = RL_TEXTURE_WRAP_CLAMP;
    else if (address == AddressMode::Border)
        addressFlag = RL_TEXTURE_WRAP_CLAMP; // ponytail: raylib has no RL_TEXTURE_WRAP_BORDER; clamp is closest
    if (filter == FilterMode::Nearest)
        filterFlag = RL_TEXTURE_FILTER_NEAREST;

    SetTextureWrap(resource->texture_, addressFlag);
    SetTextureFilter(resource->texture_, filterFlag);

    return std::make_shared<Texture>(resource);
}

} // namespace nged

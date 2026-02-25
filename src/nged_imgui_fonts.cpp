#include <nged/style.h>
#include <nged/res/fa_icondef.h>

#include "res/fa_solid.hpp"
#include "res/roboto_medium.hpp"
#include "res/sourcecodepro.hpp"

#include <imgui.h>
#include <fstream>
#include <vector>

namespace nged {
namespace detail {

static std::vector<unsigned char> readFile(const char* path)
{
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file) return {};
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);
  std::vector<unsigned char> buffer(size);
  if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) return {};
  return buffer;
}

void reloadImGuiFonts(ImFont* &sansSerif, ImFont* &mono, ImFont* &icon)
{
  auto* atlas = ImGui::GetIO().Fonts;
  auto const& style = UIStyle::instance();

  atlas->Clear();

  ImFontConfig fontConfig;
  fontConfig.OversampleH = style.fontOversampleH;
  fontConfig.OversampleV = style.fontOversampleV;
  fontConfig.PixelSnapH = style.fontPixelSnapH;

  // Load base font (from file or embedded Roboto Medium)
  if (!style.fontSansSerifPath.empty()) {
    auto fontData = readFile(style.fontSansSerifPath.c_str());
    if (!fontData.empty()) {
      sansSerif = atlas->AddFontFromMemoryTTF(fontData.data(), fontData.size(), style.normalFontSize, &fontConfig);
    }
  }
  if (!sansSerif) {
    fontConfig.FontDataOwnedByAtlas = false;
    sansSerif = atlas->AddFontFromMemoryCompressedTTF(
      roboto_medium_compressed_data, roboto_medium_compressed_size, style.normalFontSize, &fontConfig);
  }
  IM_ASSERT(sansSerif != nullptr && "Failed to load base font");

  if (style.fontMergeEnabled) {
    // Merge Source Code Pro for monospace (from file or embedded)
    ImFontConfig monoConfig;
    monoConfig.MergeMode = true;
    monoConfig.OversampleH = style.fontOversampleH;
    monoConfig.OversampleV = style.fontOversampleV;
    monoConfig.PixelSnapH = style.fontPixelSnapH;

    if (!style.fontMonoPath.empty()) {
      auto fontData = readFile(style.fontMonoPath.c_str());
      if (!fontData.empty()) {
        mono = atlas->AddFontFromMemoryTTF(fontData.data(), fontData.size(), style.normalFontSize, &monoConfig);
      }
    }
    if (!mono) {
      monoConfig.FontDataOwnedByAtlas = false;
      mono = atlas->AddFontFromMemoryCompressedTTF(
        sourcecodepro_compressed_data, sourcecodepro_compressed_size, style.normalFontSize, &monoConfig);
    }
    IM_ASSERT(mono != nullptr && "Failed to merge monospace font");

    // Merge FontAwesome icons into base font (from file or embedded)
    ImFontConfig iconConfig;
    iconConfig.MergeMode = true;
    iconConfig.GlyphMinAdvanceX = style.normalFontSize;
    iconConfig.OversampleH = style.fontOversampleH;
    iconConfig.OversampleV = style.fontOversampleV;
    iconConfig.PixelSnapH = style.fontPixelSnapH;

    if (!style.fontIconPath.empty()) {
      auto fontData = readFile(style.fontIconPath.c_str());
      if (!fontData.empty()) {
        icon = atlas->AddFontFromMemoryTTF(fontData.data(), fontData.size(), style.normalFontSize, &iconConfig);
      }
    }
    if (!icon) {
      iconConfig.FontDataOwnedByAtlas = false;
      icon = atlas->AddFontFromMemoryCompressedTTF(
        FontAwesomeSolid_compressed_data, FontAwesomeSolid_compressed_size, style.normalFontSize, &iconConfig);
    }
    IM_ASSERT(icon != nullptr && "Failed to merge icon font");
  }
}

} // namespace detail
} // namespace nged

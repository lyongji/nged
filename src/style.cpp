#include <nged/style.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

namespace nged {

using Json = nlohmann::json;

UIStyle& UIStyle::instance()
{
  static UIStyle theInstance_;
  return theInstance_;
}

void UIStyle::save()
{
  try {
    Json j;
    j["windowBackgroundColor"]    = windowBackgroundColor;
    j["nodeDefaultColor"]         = nodeDefaultColor;
    j["nodeStrokeWidth"]          = nodeStrokeWidth;
    j["nodeStrokeColor"]          = nodeStrokeColor;
    j["nodePinRadius"]            = nodePinRadius;
    j["linkStrokeWidth"]          = linkStrokeWidth;
    j["linkDefaultColor"]         = linkDefaultColor;
    j["linkSelectedWidth"]        = linkSelectedWidth;
    j["linkSelectedColor"]        = linkSelectedColor;
    j["arrowDefaultColor"]        = arrowDefaultColor;
    j["arrowSelectedColor"]       = arrowSelectedColor;
    j["nodeLabelColor"]           = nodeLabelColor;
    j["selectionBoxBackground"]   = selectionBoxBackground;
    j["deselectionBoxBackground"] = deselectionBoxBackground;
    j["routerRadius"]             = routerRadius;
    j["commentBoxMargin"]         = {commentBoxMargin.x, commentBoxMargin.y};
    j["commentColor"]             = commentColor;
    j["commentBackground"]        = commentBackground;
    j["groupBoxBackground"]       = groupBoxBackground;
    j["bigFontSize"]              = bigFontSize;
    j["normalFontSize"]           = normalFontSize;
    j["smallFontSize"]            = smallFontSize;
    j["commandPaletteWidthRatio"] = commandPaletteWidthRatio;
    j["groupboxHeaderHeight"]     = groupboxHeaderHeight;

    // Font paths
    j["fontSansSerifPath"]        = fontSansSerifPath;
    j["fontMonoPath"]             = fontMonoPath;
    j["fontIconPath"]             = fontIconPath;

    // Font rendering settings
    j["fontOversampleH"]          = fontOversampleH;
    j["fontOversampleV"]          = fontOversampleV;
    j["fontPixelSnapH"]           = fontPixelSnapH;
    j["fontMergeEnabled"]         = fontMergeEnabled;

    // Save to user config directory (cross-platform)
    const char* home = getenv("HOME");
    if (!home) {
      home = getenv("USERPROFILE"); // Windows fallback
    }
    if (!home) {
      return; // No valid home directory found
    }

    std::filesystem::path configDir = std::filesystem::path(home) / ".config" / "nged";
    std::filesystem::create_directories(configDir);
    std::ofstream(configDir / "style.json") << j.dump(2);
  } catch (...) {
    // Silently fail on save errors to avoid disrupting the app
  }
}

void UIStyle::load()
{
  try {
    // Cross-platform config path (HOME -> USERPROFILE fallback)
    const char* home = getenv("HOME");
    if (!home) {
      home = getenv("USERPROFILE"); // Windows fallback
    }
    if (!home) {
      return; // No valid home directory found
    }

    std::filesystem::path configPath = std::filesystem::path(home) / ".config" / "nged" / "style.json";
    if (!std::filesystem::exists(configPath))
      return;

    std::ifstream f(configPath);
    Json j = Json::parse(f);

    if (j.contains("windowBackgroundColor")) windowBackgroundColor    = j["windowBackgroundColor"];
    if (j.contains("nodeDefaultColor"))      nodeDefaultColor         = j["nodeDefaultColor"];
    if (j.contains("nodeStrokeWidth"))       nodeStrokeWidth          = j["nodeStrokeWidth"];
    if (j.contains("nodeStrokeColor"))       nodeStrokeColor          = j["nodeStrokeColor"];
    if (j.contains("nodePinRadius"))         nodePinRadius            = j["nodePinRadius"];
    if (j.contains("linkStrokeWidth"))       linkStrokeWidth          = j["linkStrokeWidth"];
    if (j.contains("linkDefaultColor"))      linkDefaultColor         = j["linkDefaultColor"];
    if (j.contains("linkSelectedWidth"))     linkSelectedWidth        = j["linkSelectedWidth"];
    if (j.contains("linkSelectedColor"))     linkSelectedColor        = j["linkSelectedColor"];
    if (j.contains("arrowDefaultColor"))     arrowDefaultColor        = j["arrowDefaultColor"];
    if (j.contains("arrowSelectedColor"))    arrowSelectedColor       = j["arrowSelectedColor"];
    if (j.contains("nodeLabelColor"))        nodeLabelColor           = j["nodeLabelColor"];
    if (j.contains("selectionBoxBackground")) selectionBoxBackground  = j["selectionBoxBackground"];
    if (j.contains("deselectionBoxBackground")) deselectionBoxBackground = j["deselectionBoxBackground"];
    if (j.contains("routerRadius"))          routerRadius             = j["routerRadius"];
    if (j.contains("commentColor"))          commentColor             = j["commentColor"];
    if (j.contains("commentBackground"))     commentBackground        = j["commentBackground"];
    if (j.contains("groupBoxBackground"))    groupBoxBackground       = j["groupBoxBackground"];
    if (j.contains("bigFontSize"))           bigFontSize              = j["bigFontSize"];
    if (j.contains("normalFontSize"))        normalFontSize           = j["normalFontSize"];
    if (j.contains("smallFontSize"))         smallFontSize            = j["smallFontSize"];
    if (j.contains("commandPaletteWidthRatio")) commandPaletteWidthRatio = j["commandPaletteWidthRatio"];
    if (j.contains("groupboxHeaderHeight"))  groupboxHeaderHeight     = j["groupboxHeaderHeight"];

    // Load font paths
    if (j.contains("fontSansSerifPath"))     fontSansSerifPath        = j["fontSansSerifPath"];
    if (j.contains("fontMonoPath"))          fontMonoPath             = j["fontMonoPath"];
    if (j.contains("fontIconPath"))          fontIconPath             = j["fontIconPath"];

    // Load font rendering settings
    if (j.contains("fontOversampleH"))       fontOversampleH          = j["fontOversampleH"];
    if (j.contains("fontOversampleV"))       fontOversampleV          = j["fontOversampleV"];
    if (j.contains("fontPixelSnapH"))        fontPixelSnapH           = j["fontPixelSnapH"];
    if (j.contains("fontMergeEnabled"))      fontMergeEnabled         = j["fontMergeEnabled"];

    if (j.contains("commentBoxMargin") && j["commentBoxMargin"].is_array() && j["commentBoxMargin"].size() >= 2) {
      commentBoxMargin.x = j["commentBoxMargin"][0];
      commentBoxMargin.y = j["commentBoxMargin"][1];
    }
  } catch (...) {
    // Silently fail on load errors - use defaults
  }
}

} // namespace nged


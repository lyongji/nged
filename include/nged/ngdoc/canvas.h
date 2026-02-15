#pragma once

#include "types.h"

namespace nged {

// Canvas {{{
class Canvas
{
public:
  enum class TextAlign
  {
    Left,
    Center,
    Right
  };
  enum class TextVerticalAlign
  {
    Top,
    Center,
    Bottom
  };
  enum class FontFamily
  {
    Serif,
    SansSerif,
    Mono,
    Icon
  };
  enum class FontStyle
  {
    Regular,
    Italic,
    Strong
  };
  enum class FontSize
  {
    Normal,
    Small,
    Large
  };
  enum class Layer : int
  { // z-order
    Lower = 0,
    Low,
    Standard,
    High,
    Higher,

    Count
  };

  struct ShapeStyle
  {
    bool     filled;
    uint32_t fillColor; // RGBA
    float    strokeWidth;
    uint32_t strokeColor; // RGBA
  };
  static constexpr ShapeStyle defaultShapeStyle = {true, 0xff0000ff, 0.f, 0xffffffff};
  struct TextStyle
  {
    TextAlign         align;
    TextVerticalAlign valign;
    FontFamily        font;
    FontStyle         style;
    FontSize          size;
    uint32_t          color;
  };

  class Image
  {
  public:
    virtual ~Image() = default;
  };
  using ImagePtr = std::shared_ptr<Image>;

  static constexpr TextStyle defaultTextStyle = {
    TextAlign::Left,
    TextVerticalAlign::Center,
    FontFamily::SansSerif,
    FontStyle::Regular,
    FontSize::Normal,
    0xffffffff};

protected:
  // states
  Vec2  viewPos_        = {0, 0};
  Vec2  viewSize_       = {800, 600};
  float viewScale_      = 1.0f;
  Mat3  canvasToScreen_ = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
  Mat3  screenToCanvas_ = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
  Layer layer_          = Layer::Standard;

  // display options
  bool displayTypeHint_ = false;

  Vector<Layer> layerStack_ = {};

  virtual void updateMatrix()
  {
    canvasToScreen_ = Mat3::fromSRT(Vec2(viewScale_, viewScale_), 1.f, -viewPos_) *
                      Mat3::fromRTS(Vec2(1, 1), 0, viewSize_ * 0.5f);
    screenToCanvas_ = canvasToScreen_.inverse();
  }

public:
  Vec2  viewSize() const { return viewSize_; }
  Vec2  viewPos() const { return viewPos_; }
  float viewScale() const { return viewScale_; }
  Mat3  canvasToScreen() const { return canvasToScreen_; }
  Mat3  screenToCanvas() const { return screenToCanvas_; }
  bool  displayTypeHint() const { return displayTypeHint_; }
  void  setDisplayTypeHint(bool b) { displayTypeHint_ = b; }

  void setViewSize(Vec2 size) { viewSize_ = size; }
  void setViewPos(Vec2 pos)
  {
    viewPos_ = pos;
    updateMatrix();
  }
  void setViewScale(float scale)
  {
    viewScale_ = scale;
    updateMatrix();
  }
  void pushLayer(Layer layer)
  {
    layerStack_.push_back(layer_);
    setCurrentLayer(layer);
  }
  void popLayer()
  {
    assert(layerStack_.size() > 0);
    setCurrentLayer(layerStack_.back());
    layerStack_.pop_back();
  }
  static float floatFontSize(FontSize enumsize);

  virtual ~Canvas() = default;
  virtual AABB viewport() const
  {
    return AABB(screenToCanvas_.transformPoint({0, 0}), screenToCanvas_.transformPoint(viewSize_));
  }
  virtual Vec2 measureTextSize(StringView text, TextStyle const& style = defaultTextStyle)
    const                                                                                     = 0;
  virtual void setCurrentLayer(Layer layer)                                                   = 0;
  virtual void drawLine(Vec2 a, Vec2 b, uint32_t color = 0x000000ff, float width = 1.f) const = 0;
  virtual void drawRect(
    Vec2       topleft,
    Vec2       bottomright,
    float      cornerradius = 0,
    ShapeStyle style        = defaultShapeStyle) const = 0;
  virtual void drawCircle(
    Vec2       center,
    float      radius,
    int        nsegments = 0,
    ShapeStyle style     = defaultShapeStyle) const = 0;
  virtual void drawPoly(
    Vec2 const* pts,
    sint        numpt,
    bool        closed = true,
    ShapeStyle  style  = defaultShapeStyle) const = 0;
  virtual void drawText(Vec2 pos, StringView text, TextStyle const& style = defaultTextStyle)
    const = 0;
  virtual void drawTextUntransformed(
    Vec2             pos,
    StringView       text,
    TextStyle const& style = defaultTextStyle,
    float            scale = 1.f) const = 0;

  // `data` assumed to be 32-bit RGBA, with 8-bit per channel, and `width` x `height` in size
  static ImagePtr createImage(uint8_t const* data, int width, int height);
  // this function is implemented in the-canvas-you-are-going-to-use, e.g. ImGuiCanvas
  // TODO: maybe we should put it inside a polymorphic Resource class, so that
  //       we could support more than one type of Canvas at once

  // draws a rect at (pmin to pmax) with given image
  virtual void drawImage(ImagePtr image, Vec2 pmin, Vec2 pmax, Vec2 uvmin={0,0}, Vec2 uvmax={1,1}) const = 0;
};
// }}} Canvas

} // namespace nged

// ===================================================================
// NGED Demo: 最小节点图编辑器示例
// 展示自定义节点、子图、命令的基本用法
// ===================================================================

// NGED 核心头文件
#include <nged/nged.h>              // 编辑器、视图、命令管理器
#include <nged/nged_imgui.h>        // ImGui 后端实现（停靠布局、交互状态）
#include <nged/style.h>             // UI 样式（颜色、字体、尺寸）
#include <nged/entry/entry.h>       // App 基类 + 窗口入口

// 日志
#include <spdlog/spdlog.h>
#ifdef _WIN32
#include <spdlog/sinks/wincolor_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#else
#include <spdlog/sinks/ansicolor_sink.h>
#endif

#include <nlohmann/json.hpp>       // 序列化/反序列化

#include <imgui.h>                 // Dear ImGui
#include <misc/cpp/imgui_stdlib.h> // InputText 支持 std::string

#include <chrono>                  // 计算帧时间

// ===================================================================
// 1. 自定义节点：DummyNode
//    演示如何继承 Node，定义引脚数量、类型检查、自定义绘制
// ===================================================================
class DummyNode : public nged::Node
{
  int numInput=1;                     // 输入引脚数量
  int numOutput=1;                    // 输出引脚数量
  nged::Canvas::ImagePtr icon=nullptr; // 节点图标

public:
  DummyNode(int numInput, int numOutput, nged::Graph* parent, std::string const& type, std::string const& name)
    : nged::Node(parent, type, name)   // 调用基类构造
    , numInput(numInput)
    , numOutput(numOutput)
  {
    // 生成一个随机彩色方块作为图标
    constexpr int res = 64;
    uint8_t pixels[res * res * 4] = {0};
    for (int y = 0; y < 64; y++)
      for (int x = 0; x < 64; x++)
      {
        const int index = (y * res + x) * 4;
        pixels[index + 0] = x * 4;                                    // R 通道
        pixels[index + 1] = y * 4;                                    // G 通道
        pixels[index + 2] = rand() & 0xff;                            // B 通道（随机）
        float d = gmath::distance(nged::Vec2(x,y), nged::Vec2(res / 2, res / 2));
        pixels[index + 3] = uint8_t(gmath::clamp((res / 2 - 1 - d) / 4.f, 0.f, 1.f) * 255.f); // A 通道
      }
    icon = nged::Canvas::createImage(pixels, res, res);               // 上传到 GPU
  }

  // — 引脚定义 —
  nged::sint numMaxInputs() const override { return numInput; }       // 最大输入数（<0 表示不限制）
  nged::sint numOutputs() const override { return numOutput; }        // 输出引脚数

  // — 类型检查：是否接受来自 srcNode 的连接 —
  bool acceptInput(nged::sint port, nged::Node const* srcNode, nged::sint srcPort) const override
  {
    // 演示："picky" 节点不接受另一个 "picky" 节点的连接
    if (srcNode->type() == "picky" && type() == "picky")
      return false;
    return true;
  }

  // — 自定义绘制：先画图标，再调用默认节点绘制 —
  void draw(nged::Canvas* canvas, nged::GraphItemState state) const override
  {
    auto left = nged::Vec2(aabb().min.x, pos().y);
    canvas->drawImage(icon, left-nged::Vec2(40,16), left-nged::Vec2(8,-16)); // 图标在节点左侧
    nged::Node::draw(canvas, state);                                          // 默认头部+引脚
  }
};

// ===================================================================
// 2. 子图节点：SubGraphNode
//    继承 DummyNode，内部包含一个独立的 Graph（子图）
//    双击节点即可进入子图编辑
// ===================================================================
class SubGraphNode : public DummyNode
{
  nged::GraphPtr subgraph_; // 子图对象

public:
  SubGraphNode(nged::Graph* parent):
    DummyNode(1,1,parent,"subgraph","subgraph") // 1 入 1 出，类型名 "subgraph"
  {
    subgraph_ = std::make_shared<nged::Graph>(parent->docRoot(), parent, "subgraph");
  }

  // — 返回子图，编辑器通过这个接口识别子图节点 —
  virtual nged::Graph* asGraph() override { return subgraph_.get(); }
  virtual nged::Graph const* asGraph() const override { return subgraph_.get(); }

  // — 重命名时同步更新子图名称，确保面包屑路径正确 —
  virtual bool rename(nged::String const& desired, nged::String& accepted) override {
    if (DummyNode::rename(desired, accepted)) {      // 先调用基类改名
      subgraph_->rename(accepted);                    // 同步 Graph 名称
      return true;
    }
    return false;
  }

  // — 序列化/反序列化：同时保存/加载子图内容 —
  virtual bool serialize(nged::Json& json) const override {
    return DummyNode::serialize(json) && subgraph_->serialize(json);
  }
  virtual bool deserialize(nged::Json const& json) override {
    return DummyNode::deserialize(json) && subgraph_->deserialize(json);
  }
};

// ===================================================================
// 3. 节点类型定义表
//    每个条目对应一种可创建的节点类型
// ===================================================================
struct DummyNodeDef
{
  std::string type;     // 类型标识
  int numinput, numoutput; // 引脚数量（-1 表示不限）
};

static DummyNodeDef defs[] = {
  { "exec", 4, 1 },      // 执行节点（4 入 1 出）
  { "null", 1, 1 },      // 直通节点
  { "merge", -1, 1 },    // 合并节点（不限输入，1 输出）
  { "split", 1, 2 },     // 分支节点（1 入 2 出）
  { "picky", 3, 2 },     // 挑剔节点（演示类型检查）
  { "out", 1, 0 },       // 输出节点（1 入 0 出）
  { "in", 0, 1 }         // 输入节点（0 入 1 出）
};

// ===================================================================
// 4. 节点工厂：MyNodeFactory
//    编辑器通过工厂创建节点和列出可用类型
// ===================================================================
class MyNodeFactory: public nged::NodeFactory 
{
  // — 创建根图（文档的最外层图）—
  nged::GraphPtr createRootGraph(nged::NodeGraphDoc* root) const override
  {
    return std::make_shared<nged::Graph>(root, nullptr, "root");
  }

  // — 根据类型名创建节点 —
  nged::NodePtr createNode(nged::Graph* parent, std::string_view type) const override
  {
    std::string typestr(type);
    if (type=="subgraph")
      return std::make_shared<SubGraphNode>(parent);                  // 子图节点
    for (auto const& d: defs)
      if (d.type == type)
        return std::make_shared<DummyNode>(d.numinput, d.numoutput, parent, typestr, typestr);
    return std::make_shared<DummyNode>(4, 1, parent, typestr, typestr); // 未知类型默认
  }

  // — 列出可创建的节点类型（供右键菜单和 Ctrl+P 使用）—
  //   context: 用户自定义上下文；callback: 回调函数
  //   参数: (context, 分类名, 类型标识, 显示名称)
  void listNodeTypes(
      nged::Graph* graph,
      void* context,
      void(*ret)(
        void* context,
        nged::StringView category,
        nged::StringView type,
        nged::StringView name)) const override
  {
    ret(context, "subgraph", "subgraph", "subgraph");                 // 子图分类
    for (auto const& d: defs)
      ret(context, "demo", d.type, d.type);                           // demo 分类
  }
};

// ===================================================================
// 5. 应用入口：DemoApp
//    继承 nged::App，定义初始化、帧更新、退出逻辑
// ===================================================================
class DemoApp: public nged::App
{
  nged::EditorPtr editor = nullptr; // 编辑器实例

  void init()
  {
    // — 日志设置 —
#ifdef _WIN32
    spdlog::set_default_logger(std::make_shared<spdlog::logger>("", std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>()));
    spdlog::default_logger()->sinks().emplace_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
#else
    spdlog::set_default_logger(std::make_shared<spdlog::logger>("", std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>()));
#endif
    spdlog::set_level(spdlog::level::trace);

    App::init(); // 基类初始化：设置暗色主题 + 禁用 ini 文件

    // — 创建编辑器并配置 —
    editor = nged::newImGuiNodeGraphEditor();                            // ImGui 实现的编辑器
    editor->setItemFactory(nged::addImGuiItems(nged::defaultGraphItemFactory())); // 图元工厂（注释框、路由等）
    editor->setViewFactory(nged::defaultViewFactory());                  // 视图工厂（网络视图、检查器等）
    editor->setNodeFactory(std::make_shared<MyNodeFactory>());           // 节点工厂
    editor->initCommands();                                              // 注册内置命令（保存、撤销等）
    nged::addImGuiInteractions();                                        // 注册交互状态（拖拽、框选等）

    // — 加载字体（先加载内置字体，再合并中文 CJK 字体）—
    nged::ImGuiResource::reloadFonts();
    {
      ImFontConfig cjkConfig;
      cjkConfig.MergeMode = true;                                        // 合并模式：追加到已有字体
      ImGui::GetIO().Fonts->AddFontFromFileTTF(
        "/usr/share/fonts/TTF/MapleMono-CN-Regular.ttf", 0.0f, &cjkConfig); // 1.92 动态字体：size=0
    }

    // — 创建新文档并添加默认视图（网络视图 + 检查器 + 消息视图）—
    auto doc = editor->createNewDocAndDefaultViews();
    doc->root()->createNode("in");                                       // 根图上放置一个输入节点
  }

  char const* title() { return "Demo"; }                                // 窗口标题

  bool agreeToQuit()
  {
    return editor->agreeToQuit();                                        // 委托编辑器检查未保存
  }

  void update()
  {
    static auto prev = std::chrono::system_clock::now();
    auto now = std::chrono::system_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - prev); // 增量时间

    ImGui::PushFont(nged::ImGuiResource::instance().sansSerifFont);     // 使用无衬线字体
    editor->update(dt.count() / 1000.f);                                 // 更新编辑器逻辑
    editor->draw();                                                      // 绘制所有视图
    ImGui::PopFont();

    prev = now;
  }

  void quit() {}                                                         // 退出时清理
};

// ===================================================================
// 6. main：启动应用程序
// ===================================================================
int main()
{
  nged::startApp(new DemoApp()); // 创建 App 并进入主循环
  return 0;
}

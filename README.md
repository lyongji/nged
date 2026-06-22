# NGED — 节点图编辑器

基于 C++17、Dear ImGui (docking) 和 Raylib 的跨平台节点图编辑器。

## 快速开始

```bash
# 编译并运行
xmake && ./build/linux/x86_64/release/demo
```

## 需求

* [xmake](https://xmake.io/)
* C++17 编译器（MSVC 2019+ / Clang 14+ / GCC 11+）

---

## 使用指南

### 基本操作

| 操作 | 方式 |
|------|------|
| 创建节点 | 右键画布 → 选择节点类型；或 `Ctrl+P` 输名称 |
| 连线 | 从节点输出引脚拖到目标输入引脚 |
| 断开连线 | 选中连线按 `Delete`；或按 `Y` + 划过连线 |
| 移动节点 | 左键拖拽 |
| 框选 | 左键拖拽空白区域 |
| 追加选择 | `Shift` + 点击 / 框选 |
| 缩放 | 滚轮 |
| 平移 | 中键拖拽 / `Alt` + 左键拖拽 |
| 聚焦 | `F`（聚焦选中）或 `A`（聚焦全部） |
| 复制粘贴 | `Ctrl+C/V` |

### 节点操作

| 操作 | 快捷键 |
|------|--------|
| 全选 | `Ctrl+A` |
| 删除选中 | `Delete` |
| 重命名 | `F2`（需选中单个节点） |
| 垂直对齐 | `Shift+\` |
| 水平对齐 | `Shift+-` |
| 切换绕过标志 | 右键节点 → Bypass |
| 回到父图 | `U` |

### 文件操作

| 操作 | 快捷键 |
|------|--------|
| 新建 | `Ctrl+N` |
| 打开 | `Ctrl+O` |
| 保存 | `Ctrl+S` |
| 另存为 | `Ctrl+Shift+S` |
| 撤销 | `Ctrl+Z` |
| 重做 | `Ctrl+R` |
| 关闭视图 | `Ctrl+W` |

### 视图

| 操作 | 快捷键 |
|------|--------|
| 命令面板 | `Ctrl+P` |
| 网络视图 | `Shift+Alt+W` |
| 检查器 | 右键菜单 → Inspector |
| 消息视图 | 右键菜单 → Messages |
| 类型提示切换 | `Alt+T` |

### 导航

| 操作 | 键 |
|------|---|
| 跳到上游节点 | `K` |
| 跳到下游节点 | `J` |
| 跳到左兄弟 | `H` |
| 跳到右兄弟 | `L` |

---

## 自定义节点图

最小示例参见 `examples/demo/main.cpp`：

```cpp
// 1. 定义节点工厂
class MyNodeFactory : public nged::NodeFactory {
  nged::NodePtr createNode(nged::Graph* parent, std::string_view type) const override {
    return std::make_shared<MyNode>(parent, type);
  }
  void listNodeTypes(nged::Graph*, void* ctx,
    void(*cb)(void*, nged::StringView category, nged::StringView type, nged::StringView name))
    const override {
    cb(ctx, "分类", "my_node", "我的节点");
  }
};

// 2. 定义 App
class MyApp : public nged::App {
  void init() override {
    App::init();
    editor = nged::newImGuiNodeGraphEditor();
    editor->setNodeFactory(std::make_shared<MyNodeFactory>());
    editor->setViewFactory(nged::defaultViewFactory());
    editor->initCommands();
    nged::addImGuiInteractions();
    nged::ImGuiResource::reloadFonts();
    editor->createNewDocAndDefaultViews();
  }
  void update() override {
    editor->update(deltaTime);
    editor->draw();
  }
  char const* title() override { return "My Editor"; }
};

// 3. 启动
int main() {
  nged::startApp(new MyApp());
}
```

---

## 依赖

| 库 | 来源 | 许可 |
|----|------|------|
| Dear ImGui (docking) | vendored | MIT |
| Raylib 6.0 | xmake-repo | zlib |
| rlImGui | vendored | zlib |
| spdlog, fmt, nlohmann_json, miniz, doctest | xmake-repo | MIT |

---

## 更多信息

详见 `agent.md`（架构、组件说明、扩展点）。

# ImGuiNodeGraphEditor

`ImGuiNodeGraphEditor` 是 `NodeGraphEditor` 的具体实现，使用 Dear ImGui 进行渲染和输入处理。

## 头文件

`#include "nged/nged_imgui.h"`

## 核心功能

- **停靠布局**：基于 ImGui 停靠系统（DockBuilder）的默认布局。
- **视图管理**：管理 ImGui 窗口中的 `NetworkView`、`InspectorView`、`MessageView`。
- **命令面板**：实现模糊搜索的命令面板（`Ctrl+P`）。
- **字体管理**：通过 `ImGuiResource` 加载和管理字体。

## 视图实现

| 视图类 | 功能 |
|--------|------|
| `ImGuiNetworkView` | 网络视图：画布渲染、交互状态（平移、缩放、拖拽、连线等） |
| `ImGuiInspectorView` | 检查器视图：属性编辑面板 |
| `ImGuiMessageView` | 消息视图：日志/通知/输出面板 |
| `ImGuiHelpView` | 帮助视图：关于信息、开源许可、命令列表 |

## 布局描述

默认布局使用简单的文本描述格式：

```
hsplit:            // 水平分割
  vsplit:7         // 左侧垂直分割，占 70%
    network:5      // 网络视图占 50%
    inspector:3:hide_tab_bar  // 检查器占 30%，隐藏标签栏
  message:3:hide_tab_bar     // 消息视图占 30%，隐藏标签栏
```

可调用 `editor->setDefaultLayoutDesc(desc)` 自定义布局。

## ImGuiResource

单例，管理 ImGui 字体资源：

- `sansSerifFont`：内置 Roboto Medium 或自定义文件。
- `monoFont`：内置 Source Code Pro 或自定义文件。
- `iconFont`：内置 FontAwesome Solid 或自定义文件。

```cpp
auto& res = ImGuiResource::instance();
ImGui::PushFont(res.sansSerifFont);
// ... 绘制内容 ...
ImGui::PopFont();
```

## 创建编辑器

```cpp
auto editor = nged::newImGuiNodeGraphEditor();
editor->setNodeFactory(std::make_shared<MyNodeFactory>());
editor->setViewFactory(nged::defaultViewFactory());
editor->setItemFactory(nged::addImGuiItems(nged::defaultGraphItemFactory()));
editor->initCommands();
nged::addImGuiInteractions();
```

# NGED — 节点图编辑器

## 概览

NGED 是一个**跨平台 C++17 节点图编辑器库**，基于 Raylib + Dear ImGui（docking 分支）。提供独立、可定制的 UI，用于构建可视化节点编辑器。

项目自带 Demo：
- **[demo](examples/demo/)** — 最小 C++ 示例
- **[ngs7](examples/ngs7/)** — 可视化 Lisp（s7 Scheme）节点图

---

## 项目结构

```
.
├── include/nged/          # 公共头文件
│   ├── nged.h             # 编辑器、视图、命令、交互状态
│   ├── ngdoc.h            # 文档模型：Graph、Node、Link、Item、撤销/重做
│   ├── ngdoc/types.h      # 核心类型：ItemID、Vec2、Color、HashMap、UID
│   ├── ngdoc/canvas.h     # 抽象画布绘制 API
│   ├── ngdoc/msghub.h     # 日志/消息设施
│   ├── nged_imgui.h       # ImGui 编辑器后端
│   ├── event.h            # Signal / Request 事件系统
│   ├── gmath.h            # 2D 数学：Vec2、Mat3、AABB、Color
│   ├── style.h            # UIStyle 样式常量
│   ├── utils.h            # 工具函数
│   ├── entry/entry.h      # App 基类 + 窗口入口
│   └── entry/texture.h    # 纹理抽象
│
├── src/                   # 实现文件
│   ├── ngdoc.cpp          # Graph、Node、Link、CommentBox、GroupBox、Arrow、Router
│   ├── ngdraw.cpp         # 连线路径绘制/路由
│   ├── nged.cpp           # 编辑器核心、视图、命令
│   ├── nged_imgui.cpp     # ImGui 编辑器实现（停靠布局、交互、命令面板）
│   ├── nged_imgui_fonts.cpp
│   ├── style.cpp
│   ├── entry/             # Raylib 后端：
│   │   ├── entry.cpp      # App 基类
│   │   ├── raylib_main.cpp # 主循环（raylib + rlImGui）
│   │   └── raylib_texture.cpp # 纹理上传
│   └── res/               # 嵌入资源（字体、图标）
│
├── deps/                  # 第三方依赖
│   ├── imgui/             # Dear ImGui（docking 分支，vendored）
│   ├── rlimgui/           # rlImGui（vendored，对接 project imgui）
│   ├── s7/                # s7 Scheme 解释器
│   ├── boxer/             # 跨平台消息弹窗
│   ├── nativefiledialog-extended/  # 文件对话框
│   ├── stduuid/           # UUID 生成
│   ├── parallel_hashmap/  # 高性能哈希表
│   └── subprocess.h/      # 子进程（header-only）
│
├── examples/
│   ├── demo/main.cpp      # 最小 C++ 示例
│   └── ngs7/              # 可视化 Lisp Demo
│
├── tests/
│   ├── graph_tests.cpp
│   ├── utils_tests.cpp
│   └── test_event_system.cpp
│
├── doc/                   # Markdown 文档
├── xmake.lua              # 构建配置
└── agent.md               # 本文件
```

---

## 架构

```
┌─────────────────────────────────────────┐
│         应用层 (demo/ngs7)              │
│  NodeFactory  │  InteractionStates      │
├─────────────────────────────────────────┤
│           编辑器层 (nged)               │
│  NodeGraphEditor  │  GraphView          │
│  CommandManager   │  NetworkView        │
├─────────────────────────────────────────┤
│          文档层 (ngdoc)                 │
│  NodeGraphDoc  │  Graph  │  Node        │
│  Link │ Router │ CommentBox │ GroupBox  │
│  撤销/重做 (NodeGraphDocHistory)        │
├─────────────────────────────────────────┤
│           核心 / 原语                    │
│  ItemID │ Vec2 │ AABB │ Color │ Canvas  │
│  MessageHub │ TypeSystem │ Event        │
├─────────────────────────────────────────┤
│        后端 (raylib + rlImGui)          │
└─────────────────────────────────────────┘
```

### 核心设计原则

1. **数据与 UI 分离** — `ngdoc` 完全独立于 `nged`，无头模式无需 UI 即可运行。
2. **一个文档多个视图** — 同一个 `NodeGraphDoc` 可有多个 `GraphView`。
3. **交互状态栈** — `NetworkView` 按优先级排序的状态栈定义所有用户交互。
4. **工厂模式** — `NodeFactory`、`GraphItemFactory`、`ViewFactory` 均可扩展。
5. **命令系统** — `CommandManager` 支持快捷键 + 命令面板分发。
6. **Signal/Request 事件** — 组件间松耦合。

---

## 核心组件

### 数据模型 (`ngdoc`)

| 类 | 职责 |
|---|------|
| `NodeGraphDoc` | 根文档：图树、撤销历史、图元池、序列化 |
| `Graph` | 图元 + 连线容器，支持子图和 BFS 遍历 |
| `Node` | 节点：类型、名称、颜色、输入/输出引脚 |
| `TypedNode` | 类型检查节点（通过 `TypeSystem`） |
| `Link` | 输出引脚与输入引脚间的连线 |
| `Router` | 可视化路由点 |
| `CommentBox` / `GroupBox` / `Arrow` | 注释框、分组框、箭头 |
| `NodeGraphDocHistory` | 撤销/重做栈（压缩 JSON 快照） |
| `TypeSystem` | 类型注册、可转换性检查、颜色提示 |
| `MessageHub` | 线程安全日志 |
| `Canvas` | 抽象 2D 绘制 API |

### 编辑器 (`nged`)

| 类 | 职责 |
|---|------|
| `NodeGraphEditor` | 顶层编辑器：管理文档、视图、命令 |
| `GraphView` / `NetworkView` / `InspectorView` | 视图层级 |
| `CommandManager` | 命名命令 + 快捷键 |
| `GraphEventHub` | 编辑器事件总线 |

---

## 依赖

| 包 | 来源 |
|----|------|
| raylib | xmake-repo |
| spdlog, fmt, nlohmann_json, miniz, doctest | xmake-repo |
| imgui (docking) | vendored (`deps/imgui/`) |
| rlImGui | vendored (`deps/rlimgui/`) |
| nfd, boxer, s7, stduuid, parallel_hashmap | vendored |

---

## 构建

```bash
xmake                  # 构建全部
xmake -r demo          # 构建并运行 demo
xmake run tests        # 运行测试
```

---

## 扩展点

1. 继承 `NodeFactory` — 定义节点创建逻辑
2. 继承 `GraphItemFactory` — 添加自定义图元
3. 实现 `ViewFactory` — 注册自定义视图
4. 继承 `App` — 自定义 `init()` / `update()`
5. 通过 `Command` 子类添加命令和快捷键
6. 订阅 `GraphEventHub` 信号

最小示例见 `examples/demo/main.cpp`。

---

## 关键注意事项

- 代码使用 `{{{` / `}}}` 折叠标记，建议 `foldmethod=marker`
- `ItemID` 是 64 位打包结构体（32 位随机 + 32 位池索引），防悬空指针
- 撤销/重做通过 miniz 压缩的 JSON 快照实现
- 一个输入引脚最多一根连线（设计如此）
- 子图通过 `Node::asGraph()` 支持

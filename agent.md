# NGED — 节点图编辑器

## 概览

NGED 是一个**跨平台 C++17 节点图编辑器库**，附带 Python 绑定。提供独立、可定制、可脚本化的 UI，用于构建可视化节点编辑器（如着色器图、可视化编程、数据管线）。

项目自带两个可用 Demo：
- **[ngs7](examples/ngs7/)** — 可视化 Lisp（s7 Scheme）节点图
- **[pydemo](examples/pydemo/)** — 带实时求值的 Python 可视化脚本环境

---

## 项目结构

```
.
├── include/nged/          # 公共头文件
│   ├── nged.h             # 编辑器、视图、命令、交互状态
│   ├── ngdoc.h            # 文档模型：Graph、Node、Link、各种 Item、撤销/重做历史
│   ├── ngdoc/types.h      # 核心类型：ItemID、Vec2、Color、HashMap、UID 等
│   ├── ngdoc/canvas.h     # 抽象画布绘制 API
│   ├── ngdoc/msghub.h     # 日志/消息设施
│   ├── nged_imgui.h       # ImGui 编辑器后端（视图、交互状态、资源）
│   ├── ngpy.h             # Python 绑定（pybind11）— PyNode、PyGraph、PyApp 等
│   ├── pybind11_imgui.h   # pybind11 ↔ ImGui 桥接
│   ├── event.h            # Signal / Request 事件系统
│   ├── gmath.h            # 2D 数学：Vec2、Mat3、AABB、Color
│   ├── style.h            # UIStyle 样式常量
│   ├── utils.h            # 工具函数
│   ├── entry/entry.h      # 平台窗口 + 渲染器入口
│   └── entry/texture.h    # 纹理抽象
│
├── src/                   # 实现文件
│   ├── ngdoc.cpp          # Graph、Node、Link、CommentBox、GroupBox、Arrow、Router、undo/redo
│   ├── ngdraw.cpp         # 连线路径绘制/路由
│   ├── nged.cpp           # 编辑器核心、视图、命令
│   ├── nged_imgui.cpp     # ImGui 编辑器实现（停靠布局、交互、命令面板）
│   ├── nged_imgui_fonts.cpp
│   ├── ngpy.cpp           # Python 绑定实现
│   ├── pybind11_imgui.cpp
│   ├── style.cpp
│   ├── entry/             # 平台渲染后端：
│   │   ├── entry.cpp      # App 基类（消息循环、窗口创建）
│   │   ├── dx11_main.cpp  # DirectX 11（Windows 默认）
│   │   ├── dx12_main.cpp  # DirectX 12
│   │   ├── gl2_main.cpp   # OpenGL 2
│   │   ├── gl3_main.cpp   # OpenGL 3
│   │   ├── vulkan_main.cpp
│   │   ├── metal_main.mm  # Metal（macOS）
│   │   ├── raylib_main.cpp # Raylib + rlImGui（跨平台）
│   │   └── *_texture.cpp  # 各后端纹理实现
│   └── res/               # 嵌入资源（字体、图标）
│
├── deps/                  # 第三方依赖（git submodules）
│   ├── imgui/             # Dear ImGui（UI 工具包）
│   ├── sol2/              # Lua C++ 绑定（parmscript 使用）
│   ├── s7/                # s7 Scheme 解释器（ngs7 demo 使用）
│   ├── spdlog/            # 日志库
│   ├── fmt/               # 字符串格式化
│   ├── pybind11/          # Python 绑定
│   ├── parmscript/        # 节点参数脚本引擎
│   ├── nlohmann/          # JSON 解析
│   ├── stduuid/           # UUID 生成
│   ├── miniz/             # 压缩（撤销/重做历史）
│   ├── boxer/             # 跨平台消息弹窗
│   ├── nativefiledialog-extended/  # 文件对话框
│   ├── parallel_hashmap/  # 高性能哈希表（phmap）
│   ├── subprocess.h/      # 子进程执行
│   ├── doctest/           # C++ 测试框架
│   ├── lua/               # Lua 5.x（parmscript 使用）
│   └── catch/             # 测试支持（stduuid 使用）
│
├── examples/
│   ├── demo/main.cpp      # 最小 C++ 示例 — 构建你自己的编辑器
│   ├── typed_demo/main.cpp
│   ├── ngs7/              # 可视化 Lisp Demo（通过 s7 运行 Scheme）
│   └── pydemo/            # Python 脚本化 Demo（可视化 Python）
│       ├── main.py
│       ├── node.py         # 节点定义
│       ├── graph.py        # PyGraph 子类（带求值）
│       ├── nodelib.py      # 内置节点库
│       ├── evaluation.py   # 异步图求值
│       ├── dataview.py     # 数据表格视图
│       ├── document.py     # 自定义 PyNodeGraphDoc
│       └── edresponse.py   # 编辑器事件回调
│
├── nged/                  # Python 包（pip install）
│   ├── __init__.py        # 从 C++ 扩展重新导出
│   ├── msghub.py          # Python 端 MessageHub API
│   └── _version.py
│
├── tests/
│   ├── graph_tests.cpp    # Graph 模型 C++ 单元测试
│   ├── utils_tests.cpp
│   ├── test_event_system.cpp
│   └── pytest.py          # Python 集成测试
│
├── doc/                   # Markdown 文档
│   ├── core/              # Graph、Node、Link 等
│   ├── views/             # GraphView、NetworkView、InspectorView
│   ├── graphics/          # Canvas、UIStyle
│   ├── interaction/       # CommandManager
│   └── utils/             # MessageHub、TypeSystem
│
├── xmake.lua              # 构建配置（xmake）
├── CMakeLists.txt         # 构建配置（CMake，供 pip install 使用）
├── pyproject.toml         # Python 包配置（scikit-build-core）
├── features.txt           # 功能清单
└── patches/               # imgui metal、s7、sol2 补丁
```

---

## 架构

### 分层设计

```
┌─────────────────────────────────────────┐
│         应用层 (demo/ngs7/pydemo)        │
│  ┌─────────────┐  ┌──────────────────┐  │
│  │ NodeFactory  │  │ InteractionStates│  │
│  └─────────────┘  └──────────────────┘  │
├─────────────────────────────────────────┤
│           编辑器层 (nged)               │
│  NodeGraphEditor  │  GraphView          │
│  CommandManager   │  NetworkView        │
│  InspectorView    │  ViewFactory        │
├─────────────────────────────────────────┤
│          文档层 (ngdoc)                 │
│  NodeGraphDoc  │  Graph  │  Node        │
│  Link │ Router │ CommentBox │ GroupBox  │
│  撤销/重做 (NodeGraphDocHistory)        │
├─────────────────────────────────────────┤
│           核心 / 原语                    │
│  ItemID │ Vec2 │ AABB │ Color │ Canvas  │
│  MessageHub │ TypeSystem │ Event(Signal)│
├─────────────────────────────────────────┤
│        后端 (entry / ImGui)             │
│  DX11 │ DX12 │ OpenGL 2/3 │ Vulkan │ Metal │ Raylib│
└─────────────────────────────────────────┘
```

### 核心设计原则

1. **数据与 UI 分离** — `ngdoc`（数据模型）完全独立于 `nged`（编辑器 UI）。无头模式无需任何 UI 即可运行。

2. **一个文档多个视图** — 同一个 `NodeGraphDoc` 可以有多个 `GraphView`（网络视图、检查器、自定义视图）。视图通过信号观察文档。

3. **交互状态栈** — `NetworkView` 使用按优先级排序的状态栈。`SelectionState`、`LinkState`、`MoveState`、`CreateNodeState` 等状态定义所有用户交互。自定义行为通过添加/移除状态实现。

4. **工厂模式** — `NodeFactory` 创建节点，`GraphItemFactory` 创建图元（注释框、箭头、分组框），`ViewFactory` 创建视图。全部可由用户扩展。

5. **命令系统** — `CommandManager` 支持命名命令、快捷键、可选提示对话框、菜单中可见/隐藏。

6. **信号/请求事件** — `Signal<T...>`（发射即忘）和 `Request<T...>`（所有处理器必须同意）用于组件间松耦合。

---

## 核心组件

### 数据模型 (`ngdoc`)

| 类 | 职责 |
|---|------|
| `NodeGraphDoc` | 根文档：拥有图树、撤销历史、图元池、序列化 |
| `Graph` | 图元 + 连线的容器。支持子图。遍历（BFS 自顶向下/自底向上） |
| `Node` | 基础节点：类型、名称、颜色、输入/输出引脚、序列化 |
| `TypedNode` | 带类型检查引脚的节点（通过 `TypeSystem`） |
| `Link` | 一个输出引脚与一个输入引脚之间的连线 |
| `Router` | 可视化走线路由点（直通） |
| `CommentBox` | 自由文本注释框，可调整大小 |
| `GroupBox` | 包含其他图元的分组矩形 |
| `Arrow` | 装饰性箭头 |
| `GraphItemPool` | 图元内存池（索引 + 随机 ID 防悬空指针） |
| `NodeGraphDocHistory` | 完整撤销/重做栈（压缩 JSON 快照） |
| `TypeSystem` | 全局单例：注册类型、检查可转换性、提供颜色提示 |
| `MessageHub` | 线程安全日志，分类（Log/Notice/Output）和详细程度级别 |
| `Canvas` | 抽象 2D 绘制 API（线条、矩形、圆、文本、图片）— 由 ImGui 后端实现 |
| `NodePin` | 标识一个引脚：节点 ID + 端口索引 + 方向（In/Out） |
| `GraphTraverseResult` | BFS 图遍历结果，支持索引访问节点/输入/输出 |

### 编辑器 (`nged`)

| 类 | 职责 |
|---|------|
| `NodeGraphEditor` | 顶层编辑器：管理文档、视图、命令、剪贴板、工厂 |
| `GraphView` | 文档/图上的基础视图。有 kind、title、focus/hover 状态 |
| `NetworkView` | 主画布视图：交互状态、选择、缩放/平移、特效 |
| `InspectorView` | 关联到另一个视图的属性检查器 |
| `ViewFactory` | 按 kind 字符串创建视图（"network"、"inspector"、自定义） |
| `CommandManager` | 基于快捷键 + 命令面板的命令分发 |
| `AnimationState` | 平滑视图平移/缩放动画 |
| `GraphEventHub` | 编辑器级信号的中心事件总线 |

### ImGui 后端 (`nged_imgui`)

| 类/结构体 | 职责 |
|-----------|------|
| `ImGuiNodeGraphEditor` | 停靠布局、窗口管理、内置默认视图 |
| `ImGuiGraphView<T>` | CRTP 基类，将 `GraphView` 适配到 ImGui 窗口 |
| `ImGuiResource` | 字体图集（无衬线、等宽、图标字体） |
| `ImGuiCommentBox` | 带就地文本编辑的注释框 |
| 各种 `detail::*` | 交互状态：`LinkState`、`MoveState`、`SelectionState`、`CreateNodeState`、`CutLinkState`、`HandleView` 等 |

### 入口 / 平台

| 类 | 职责 |
|---|------|
| `App` | 应用基类：`init()`、`update()`、`draw()`、`quit()` |
| `startApp()` | 平台相关的消息循环 + 窗口创建 |

---

## 依赖关系图

```
nged ─────── nfd, imgui, boxer, ngdoc, entry
ngdoc ────── spdlog, miniz, fmt
ngpy ────── nged, entry, parmscript, lua, spdlog, pybind11
entry ────── imgui（+ 平台后端）
demo/ngs7 ── nged, entry
tests ────── ngdoc, spdlog, doctest
```

所有依赖均以 vendored 方式存放在 `deps/` 目录。

---

## 扩展点（如何构建你自己的编辑器）

1. **定义 NodeFactory** — 继承 `NodeFactory`，实现 `createRootGraph()`、`createNode()`、`listNodeTypes()`
2. **定义 GraphItemFactory** — 通过 `set()` 添加自定义图元（注释框、分组框、箭头）
3. **设置 ViewFactory** — 注册自定义视图（如数据表格、控制台）
4. **继承 App** — 实现 `init()`、`update()`、`title()` 等
5. **注册命令** — 通过 `Command` 子类添加快捷键
6. **订阅事件** — 连接到 `GraphEventHub` 的信号

最小 C++ 示例参见 `examples/demo/main.cpp`（~100 行）。
Python 示例参见 `examples/pydemo/main.py`。

---

## 构建

| 工具 | 用途 |
|------|------|
| **xmake** | 主 C++ 构建系统（`xmake` → 构建所有目标） |
| **CMake** | 备选构建方式（由 `pip install .` 通过 scikit-build-core 使用） |
| **pip** | Python 包安装 |

```bash
xmake                  # 构建全部
xmake run demo         # 运行 C++ demo
xmake pytest           # 构建 ngpy + 运行 Python 测试
pip install .          # 安装为 Python 包（含类型桩文件）
```

选择渲染后端：`xmake f --backend=dx11|dx12|gl3|gl2|vulkan|metal|raylib`

---

## Python 绑定

`ngpy` 共享库通过 pybind11 封装了整个编辑器框架：
- `PyNode` / `PyNodeFactory` — 在 Python 中定义节点
- `PyGraph` / `PyNodeGraphDoc` — 带 Python 过滤器的自定义文档
- `PyImGuiNodeGraphEditor` — 从 Python 访问完整编辑器
- `PyApp` — 从 Python 运行编辑器主循环
- `PyCommand` / `PyGraphView` — Python 中自定义命令和视图

`nged/` Python 包提供清晰的重新导出、`msghub` 日志器和 `ngpy.pyi` 类型桩文件。

---

## 关键注意事项

- **折叠标记**：代码使用 `{{{` / `}}}` 折叠标记；在 vim/nvim 中建议设置 `foldmethod=marker`。
- **Editorconfig** + **clang-format** 强制执行代码风格。
- **ItemID** 是 64 位打包结构体：32 位随机数 + 32 位池索引 — 防止悬空指针 bug。
- **撤销/重做**：通过 miniz 进行完整 JSON 快照压缩。每次提交存储完整图状态。
- **一个输入引脚最多一根连线**：设计如此；多输入逻辑请使用合并节点。
- **子图**：`Node::asGraph()` 返回嵌套的 `Graph`，支持递归的图中图。

---

## 按任务速查文件

| 任务 | 关键文件 |
|------|----------|
| 添加新节点类型 | `include/nged/ngdoc.h`（Node 类），示例：`examples/demo/main.cpp`、`examples/pydemo/node.py` |
| 自定义交互 | `include/nged/nged.h`（InteractionState），`include/nged/nged_imgui.h`（detail:: 状态） |
| 添加新视图 | `include/nged/nged.h`（GraphView），`include/nged/nged_imgui.h`（ImGuiGraphView CRTP） |
| 修改数据模型 | `include/nged/ngdoc.h`，`src/ngdoc.cpp` |
| 序列化格式 | `include/nged/ngdoc.h`（serialize/deserialize 方法），`src/ngdoc.cpp` |
| 图遍历 | `include/nged/ngdoc.h`（`Graph::traverse`、`GraphTraverseResult`） |
| Python 绑定 | `include/nged/ngpy.h`，`src/ngpy.cpp` |
| 绘制 / 渲染 | `include/nged/ngdoc/canvas.h`，`src/ngdraw.cpp` |
| 事件 / 信号 | `include/nged/event.h` |
| 构建配置 | `xmake.lua`，`CMakeLists.txt`，`pyproject.toml` |

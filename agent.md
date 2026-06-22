# NGED — 节点图编辑器

## 概览

NGED 是一个**跨平台 C++17 节点图编辑器库**，基于 Raylib + Dear ImGui（docking 分支）。

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
│   ├── entry/entry.h      # App 基类
│   └── entry/texture.h    # 纹理抽象
│
├── src/                   # 实现
│   ├── ngdoc.cpp          # Graph、Node、Link、CommentBox、GroupBox、Arrow、Router
│   ├── ngdraw.cpp         # 连线路径绘制/路由
│   ├── nged.cpp           # 编辑器核心、视图、命令
│   ├── nged_imgui.cpp     # ImGui 编辑器实现（停靠布局、交互、命令面板）
│   ├── nged_imgui_fonts.cpp # 字体加载
│   ├── style.cpp
│   ├── entry/
│   │   ├── entry.cpp      # App 基类（默认主题、颜色）
│   │   ├── raylib_main.cpp # 主循环（raylib + rlImGui）
│   │   └── raylib_texture.cpp # 纹理上传
│   └── res/               # 嵌入资源（字体、图标）
│
├── deps/                  # 第三方依赖
│   ├── imgui/             # Dear ImGui docking 分支（vendored）
│   ├── rlimgui/           # rlImGui（vendored，对接项目 imgui）
│   ├── s7/                # s7 Scheme 解释器
│   ├── boxer/             # 消息弹窗
│   ├── nativefiledialog-extended/  # 文件对话框
│   ├── stduuid/           # UUID
│   ├── parallel_hashmap/  # 哈希表
│   └── subprocess.h/      # 子进程（header-only）
│
├── examples/
│   ├── demo/main.cpp      # 最小示例
│   ├── typed_demo/main.cpp
│   └── ngs7/              # 可视化 Lisp
│
├── tests/                 # C++ 单元测试
├── doc/                   # 文档
├── xmake.lua              # 构建配置
└── README.md
```

---

## 架构

```
┌─────────────────────────────────────┐
│         应用层 (demo/ngs7)          │
│  NodeFactory  │  InteractionStates  │
├─────────────────────────────────────┤
│         编辑器层 (nged)             │
│  NodeGraphEditor │ GraphView        │
│  CommandManager  │ NetworkView      │
├─────────────────────────────────────┤
│         文档层 (ngdoc)              │
│  NodeGraphDoc │ Graph │ Node        │
│  Link │ Router │ CommentBox         │
│  撤销/重做                          │
├─────────────────────────────────────┤
│         核心 / 原语                  │
│  ItemID │ Vec2 │ Canvas │ Event     │
│  MessageHub │ TypeSystem            │
├─────────────────────────────────────┤
│      后端 (raylib + rlImGui)        │
└─────────────────────────────────────┘
```

---

## 核心组件

### 数据模型 (`ngdoc`)

| 类 | 职责 |
|---|------|
| `NodeGraphDoc` | 根文档：图树、撤销历史、序列化 |
| `Graph` | 图元容器，支持子图和 BFS 遍历 |
| `Node` / `TypedNode` | 节点：引脚、类型检查 |
| `Link` | 输出→输入连线 |
| `Router` / `CommentBox` / `GroupBox` / `Arrow` | 路由点、注释、分组、箭头 |
| `NodeGraphDocHistory` | 撤销/重做（压缩 JSON 快照） |
| `TypeSystem` | 全局类型注册、可转换性检查 |
| `MessageHub` | 线程安全日志 |
| `Canvas` | 抽象 2D 绘制 |

### 编辑器 (`nged`)

| 类 | 职责 |
|---|------|
| `NodeGraphEditor` | 顶层管理：文档、视图、命令 |
| `GraphView` / `NetworkView` / `InspectorView` | 视图层级 |
| `CommandManager` | 命名命令 + 快捷键 |
| `GraphEventHub` | 事件总线 |

---

## 依赖

| 包 | 来源 |
|----|------|
| raylib | xmake-repo |
| spdlog, fmt, nlohmann_json, miniz, doctest | xmake-repo |
| imgui (docking) | vendored |
| rlImGui | vendored |
| nfd, boxer, s7, stduuid, phmap, subprocess.h | vendored |

---

## 构建

```bash
xmake                    # 构建全部
xmake -r demo            # 编译并运行 demo
xmake run tests          # 运行测试
```

---

## 扩展点

1. 继承 `NodeFactory` — 定义节点创建
2. 实现 `GraphItemFactory` — 自定义图元
3. 实现 `ViewFactory` — 自定义视图
4. 继承 `App` — 自定义 `init()` / `update()`
5. `Command` 子类 — 命令和快捷键
6. 订阅 `GraphEventHub` 信号

最小示例：`examples/demo/main.cpp`

---

## 注意事项

- 代码使用 `{{{` / `}}}` 折叠标记，建议 `foldmethod=marker`
- `ItemID` 是 64 位打包结构体（32 随机 + 32 池索引），防悬空指针
- 撤销/重做通过 miniz 压缩 JSON 快照
- 一个输入引脚最多一根连线
- 子图：`Node::asGraph()` 返回嵌套 `Graph`
- CJK 输入：demo 在 `reloadFonts()` 后合并 MapleMono-CN（1.92 动态字体系统）

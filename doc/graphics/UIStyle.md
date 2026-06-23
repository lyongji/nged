# UIStyle

`UIStyle` 定义节点图编辑器的视觉外观。包含颜色、尺寸等样式属性。是一个单例。

## 头文件

`#include "nged/nged.h"`

## 使用

```cpp
auto& style = nged::UIStyle::instance();
style.nodeDefaultColor = 0xddddddff;
```

## 属性

### 节点

| 属性 | 默认值 | 说明 |
|------|--------|------|
| `nodeDefaultColor` | `0xddddddff` | 节点默认颜色 |
| `nodeStrokeWidth` | `0.f` | 节点边框宽度 |
| `nodeStrokeColor` | `0x000000ff` | 节点边框颜色 |
| `nodePinRadius` | `3.4f` | 引脚半径 |
| `nodeLabelColor` | `0xeeeeeeff` | 节点标签颜色 |

### 连线

| 属性 | 默认值 | 说明 |
|------|--------|------|
| `linkStrokeWidth` | `2.f` | 连线宽度 |
| `linkDefaultColor` | `0xddddddff` | 连线默认颜色 |
| `linkSelectedWidth` | `4.f` | 选中连线宽度 |
| `linkSelectedColor` | `0xffff00ff` | 选中连线颜色 |

### 注释与分组

| 属性 | 默认值 | 说明 |
|------|--------|------|
| `commentColor` | `0x4caf50ff` | 注释边框颜色 |
| `commentBackground` | `0x004d4066` | 注释背景颜色 |
| `commentBoxMargin` | `{8, 8}` | 注释内边距 |
| `groupBoxBackground` | `0x44444466` | 分组框背景 |

### 字体大小

| 属性 | 默认值 |
|------|--------|
| `bigFontSize` | `24` |
| `normalFontSize` | `18` |
| `smallFontSize` | `14` |

### 字体路径（空字符串使用内嵌字体）

| 属性 | 说明 |
|------|------|
| `fontSansSerifPath` | 无衬线字体文件路径 |
| `fontMonoPath` | 等宽字体文件路径 |
| `fontIconPath` | 图标字体文件路径 |
| `fontMergeEnabled` | 是否启用字体合并（`true`） |

### 其他

| 属性 | 默认值 | 说明 |
|------|--------|------|
| `windowBackgroundColor` | `0x333333ff` | 窗口背景 |
| `selectionBoxBackground` | `0x33691E88` | 框选颜色 |
| `deselectionBoxBackground` | `0x600D1E88` | 反选颜色 |
| `routerRadius` | `6.f` | 路由点半径 |
| `commandPaletteWidthRatio` | `0.75f` | 命令面板宽度占比 |
| `groupboxHeaderHeight` | `16.f` | 分组框标题高度 |

## 公开方法

- `static UIStyle& instance()`：获取单例实例。
- `void save()`：保存样式到文件。
- `void load()`：从文件加载样式。

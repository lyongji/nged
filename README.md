NGED: A **N**ode **G**raph **ED**itor
=====================================

A standalone, customizable, cross-platform node graph editor built with C++17, Dear ImGui (docking) and Raylib.

Core components: `ngdoc.h` (node graph document) and `nged.h` (node graph editor).

* [demo](examples/demo/) — minimal C++ example
* [visual s7](examples/ngs7/) — Lisp visualized as node graph

---

## Building

### Requirements

* [xmake](https://xmake.io/)
* C++17 compiler (MSVC 2019+, Clang 14+, GCC 11+)

```bash
xmake && ./build/linux/x86_64/release/demo
```

---

## Features

* Nodes & Links, Subgraphs, Routers
* Fuzzy search, command palette
* Groups, Comments, Arrows
* Undo / Redo, Copy / Paste
* Multiple views on one graph
* Type checking with color hints
* Headless mode (doc without UI)

---

## To Make Your Own NodeGraph

See [examples/demo/main.cpp](examples/demo/main.cpp). Define your `NodeFactory` and go.

```cpp
class MyNodeFactory : public nged::NodeFactory {
  nged::NodePtr createNode(nged::Graph* parent, std::string_view type) const override { ... }
  void listNodeTypes(...) const override { ... }
};
```

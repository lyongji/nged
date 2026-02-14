# NodeGraphEditor

The `NodeGraphEditor` class is the central controller for the node graph editor application. It manages views, documents, commands, and factories.

## Header

`#include "nged/nged.h"`

## Class Definition

```cpp
class NodeGraphEditor
{
public:
  using DocPtr  = std::shared_ptr<NodeGraphDoc>;
  using ViewPtr = std::shared_ptr<GraphView>;
  virtual ~NodeGraphEditor() { }
  // ...
};
```

## Key Responsibilities

-   **Document Management**: Creates, opens, saves, and closes documents (`NodeGraphDoc`).
-   **View Management**: Manages a set of views (`GraphView`) that display the graph or other information.
-   **Command Handling**: Integrates with `CommandManager` to handle user commands.
-   **Factory Management**: Holds references to `NodeFactory`, `GraphItemFactory`, and `ViewFactory`.
-   **Edit Response**: Uses `NodeGraphEditResponser` to hook into edit events.

## Public Methods

### Configuration

-   `void setFileExt(String ext)`: Sets the file extension for saved documents.
-   `void setItemFactory(GraphItemFactoryPtr factory)`: Sets the factory for creating graph items.
-   `void setViewFactory(ViewFactoryPtr factory)`: Sets the factory for creating views.
-   `void setNodeFactory(NodeFactoryPtr factory)`: Sets the factory for creating nodes.
-   `void setResponser(NodeGraphEditResponserPtr responser)`: Sets the responser for edit events.
-   `void setDocFactory(std::function<NodeGraphDocPtr(...)> factory)`: Sets the factory for creating documents.
-   `void setContextMenus(ContextMenuEntriesPtr menus)`: Sets the context menu entries.

### Document Operations

-   `virtual DocPtr createNewDocAndDefaultViews()`: Creates a new document and default views.
-   `virtual DocPtr openDoc(StringView path)`: Opens a document from a file.
-   `virtual bool loadDocInto(StringView path, DocPtr dest)`: Loads a document into an existing one.
-   `virtual bool saveDoc(DocPtr doc)`: Saves a document.
-   `virtual bool saveDocAs(DocPtr doc, StringView path)`: Saves a document to a specific path.
-   `bool closeView(ViewPtr view, bool confirmIfNotSaved)`: Closes a view, optionally prompting to save.
-   `bool agreeToQuit() const`: Checks if it's safe to quit (i.e., no unsaved changes).

### Graph Manipulation

These methods respect the `NodeGraphEditResponser` and handle undo/redo history where appropriate.

-   `NodePtr createNode(Graph* graph, StringView type)`: Creates a node of a specific type.
-   `ItemID addItem(Graph* graph, GraphItemPtr item)`: Adds an item to the graph.
-   `void confirmItemPlacements(Graph* graph, HashSet<ItemID> const& items)`: Confirms the placement of moved items.
-   `bool moveItems(Graph* graph, HashSet<ItemID> const& items, Vec2 delta)`: Moves items by a delta.
-   `void removeItems(Graph* graph, HashSet<ItemID> const& items, ...)`: Removes items from the graph.
-   `bool setLink(Graph* graph, NetworkView* fromView, ItemID sourceItem, sint sourcePort, ItemID destItem, sint destPort)`: Creates a link between two items.
-   `void removeLink(Graph* graph, ItemID destItem, sint destPort)`: Removes a link connected to a specific input port.

### Update Loop

-   `virtual void update(float dt)`: Updates the editor and its views.
-   `virtual void draw() = 0`: Draws the editor (pure virtual, to be implemented by UI backend).

## Accessors

-   `auto const& views() const`: Returns the list of active views.
-   `auto& commandManager()`: Returns the command manager.
-   `auto* responser() const`: Returns the current responser.

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
  
  GraphEventHub& events() { return eventHub_; }
  // ...
};
```

## Key Responsibilities

-   **Document Management**: Creates, opens, saves, and closes documents (`NodeGraphDoc`).
-   **View Management**: Manages a set of views (`GraphView`) that display the graph or other information.
-   **Command Handling**: Integrates with `CommandManager` to handle user commands.
-   **Factory Management**: Holds references to `NodeFactory`, `GraphItemFactory`, and `ViewFactory`.
-   **Event Handling**: Centralized event hub (`GraphEventHub`) for observing and intercepting graph operations.

## Event System (GraphEventHub)

The `NodeGraphEditor` exposes a `GraphEventHub` via `events()`. This hub contains `Signal`s (for notifications) and `Request`s (for intercepting/modifying operations).

### Signals (Notifications)

Listeners can subscribe to these signals to be notified when events occur.

-   `onItemAdded(Graph*, GraphItem*)`: Called after an item is added.
-   `onItemRemoved(GraphItem*)`: Called after an item is removed.
-   `onNodeRenamed(Graph*, Node*)`: Called after a node is renamed.
-   `onViewRemoved(GraphView*)`: Called when a view is closed.
-   `beforeViewUpdate(GraphView*)`, `afterViewUpdate(GraphView*)`: Called around view updates.
-   `beforeViewDraw(GraphView*)`, `afterViewDraw(GraphView*)`: Called around view drawing.
-   `onItemMoved(GraphItem*)`: Called when an item is moved.
-   `onItemModified(GraphItem*)`: Called when an item's internal state changes.
-   `onInspect(InspectorView*, GraphItem**, size_t)`: Called when items are inspected.
-   `afterPaste(Graph*, GraphItem**, size_t)`: Called after items are pasted.
-   `onItemClicked`, `onItemDoubleClicked`, `onItemHovered`: Interaction events.
-   `onSelectionChanged`: Called when selection changes.
-   `onLinkSet(Link*)`, `onLinkRemoved(Link*)`: Called when links are modified.

### Requests (Interception)

Requests allow listeners to approve, deny, or modify an operation before it happens. If any listener returns `false`, the operation is aborted.

-   `requestAddItem(Graph*, GraphItem* item, GraphItem** replacement)`: Can deny addition (return false) or provide a replacement item.
-   `requestRemoveItem(Graph*, GraphItem*)`: Can deny item removal.
-   `requestRenameNode(Graph*, Node*)`: Can deny node renaming.
-   `requestRemoveView(GraphView*)`: Can deny view removal.
-   `requestLinkSet(Graph*, InputConnection, OutputConnection)`: Can deny link creation.

### Usage Example

```cpp
// Listen for notifications
editor->events().onItemAdded.connect([](Graph* graph, GraphItem* item) {
    msghub::infof("Item added: {}", item->id());
});

// Intercept operations
editor->events().requestAddItem.connect([](Graph* graph, GraphItem* item, GraphItem** replacement) -> bool {
    if (item->asNode() && item->asNode()->type() == "forbidden") {
        return false; // Deny addition
    }
    return true; // Allow
});
```

## Public Methods

### Configuration

-   `void setFileExt(String ext)`: Sets the file extension for saved documents.
-   `void setItemFactory(GraphItemFactoryPtr factory)`: Sets the factory for creating graph items.
-   `void setViewFactory(ViewFactoryPtr factory)`: Sets the factory for creating views.
-   `void setNodeFactory(NodeFactoryPtr factory)`: Sets the factory for creating nodes.
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
-   `GraphEventHub& events()`: Returns the event hub.
# Node

The `Node` class is the base class for all nodes in the graph. It represents a functional unit with inputs and outputs.

## Header

`#include "nged/nged.h"`

## Class Definition

```cpp
class Node : public GraphItem, public Dyeable
{
protected:
  String   type_;
  String   name_;
  Color    color_;
  uint64_t flags_ = 0;
  // ...
};
```

## Key Responsibilities

-   **Data Model**: Defines the number of inputs/outputs and their properties.
-   **Visual Representation**: Handles drawing and layout of the node.
-   **Interaction**: Defines how the node interacts with links (accepting inputs).

## Public Methods

### Properties

-   `String const& type() const`: Returns the node type.
-   `String const& name() const`: Returns the node name.
-   `Color color() const`: Returns the node color.
-   `void setColor(Color c)`: Sets the node color.
-   `uint64_t flags() const`: Returns node flags.

### Configuration (Virtual)

-   `virtual sint numMaxInputs() const`: Returns maximum number of inputs (negative for unlimited).
-   `virtual sint numFixedInputs() const`: Returns number of fixed input pins.
-   `virtual bool isRequiredInput(sint port) const`: Checks if an input is mandatory.
-   `virtual sint numOutputs() const`: Returns number of outputs.
-   `virtual bool acceptInput(sint port, Node const* sourceNode, sint sourcePort) const`: Checks if a connection is valid.
-   `virtual void resize(float width, float height, ...)`: Resizes the node.

### UI & Layout (Virtual)

-   `virtual Vec2 inputPinPos(sint i) const`: Returns position of an input pin.
-   `virtual Vec2 outputPinPos(sint i) const`: Returns position of an output pin.
-   `virtual Color inputPinColor(sint i) const`: Returns color of an input pin.
-   `virtual Color outputPinColor(sint i) const`: Returns color of an output pin.
-   `virtual void draw(Canvas* canvas, GraphItemState state) const`: Draws the node.

### Helper

-   `bool getInput(sint inPort, NodePtr& nodeptr, sint& outPort) const`: Retrieves the connected source node and port.

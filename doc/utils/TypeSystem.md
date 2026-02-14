# TypeSystem

The `TypeSystem` class manages type definitions and conversions for typed nodes and connections. It ensures that only compatible ports can be connected.

## Header

`#include "nged/nged.h"`

## Class Definition

```cpp
class TypeSystem
{
  // ...
  static TypeSystem& instance();
};
```

## Key Responsibilities

-   **Type Registration**: Registers new types with optional base types and color hints.
-   **Type Conversion**: Defines which types can be converted to others (for connection compatibility).
-   **Color Hints**: Associates colors with types for visual feedback in the graph.

## Public Methods

### Registration

-   `TypeIndex registerType(StringView type, StringView baseType, Color hintColor)`: Registers a new type.
-   `void setConvertable(StringView from, StringView to, bool convertable)`: Sets whether one type can be converted to another.

### Query

-   `bool isConvertable(StringView from, StringView to) const`: Checks if a conversion is allowed.
-   `bool isType(StringView type) const`: Checks if a type is registered.
-   `TypeIndex typeIndex(StringView type) const`: Returns the index of a type.
-   `StringView typeName(TypeIndex index) const`: Returns the name of a type.
-   `StringView typeBaseType(TypeIndex index) const`: Returns the base type of a type.

### Visuals

-   `void setColorHint(TypeIndex index, Color hint)`: Sets the color hint for a type.
-   `Optional<Color> colorHint(TypeIndex index) const`: Returns the color hint for a type.

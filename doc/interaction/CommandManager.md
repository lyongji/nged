# CommandManager

The `CommandManager` class handles the registration and execution of application commands, including keyboard shortcuts and palette integration.

## Header

`#include "nged/nged.h"`

## Class Definition

```cpp
class CommandManager
{
protected:
  Vector<CommandPtr> commands_;
  // ...
};
```

## Key Responsibilities

-   **Command Registry**: Stores a list of available commands.
-   **Shortcut Handling**: Checks for keyboard shortcuts.
-   **Execution**: Dispatches commands and handles prompts.
-   **Prompting**: Displays input prompts for commands that require arguments.

## Public Methods

### Registration

-   `Command& add(Command*&& cmd)`: Registers a new command.
-   `bool remove(String const& name)`: Unregisters a command.
-   `auto const& commands() const`: Returns all registered commands.

### Execution

-   `void update(GraphView* view)`: Checks for active shortcuts and updates prompt state.
-   `void draw(NetworkView* view)`: Draws any active prompts or overlays.
-   `void openPalette()`: Opens the command palette (if implemented).
-   `void checkShortcut(GraphView* view)`: Checks if any shortcut is pressed.

### Helper Methods

-   `void prompt(CommandPtr cmd, GraphView* view)`: Initiates a prompt for the given command.
-   `void resetPrompt()`: Cancels the current prompt.

## Nested Classes

### Command
Abstract base class for a command.
-   `String name()`: Command name.
-   `String description()`: Command description.
-   `String view()`: View type filter (e.g., "network", "*").
-   `Shortcut shortcut()`: Keyboard shortcut.
-   `virtual void onConfirm(GraphView* view) = 0`: Action to perform when executed.

### SimpleCommand
A concrete implementation of `Command` using callbacks.
-   `SimpleCommand(String name, String desc, Callback callback, ...)`
-   `void onConfirm(GraphView* view) override`: Executes the callback.

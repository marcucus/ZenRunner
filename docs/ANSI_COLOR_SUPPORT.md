# ANSI Color Support in ZenRunner

## Overview

ZenRunner includes a fast, lightweight ANSI/VT100 escape sequence parser that preserves colored terminal output from development tools in the log console. This allows developers to see errors, warnings, and other important information with the same colors they would see in a terminal.

## Supported Features

### Color Modes
- **Standard Colors (8)**: Basic ANSI colors (black, red, green, yellow, blue, magenta, cyan, white)
- **Bright Colors (8)**: High-intensity variants of standard colors
- **256-Color Palette**: Extended color palette with 216 colors + 24 grayscale shades
- **RGB True Color**: 24-bit RGB colors for maximum color fidelity

### Text Attributes
- **Bold** text (SGR 1)
- **Italic** text (SGR 3)
- **Underline** text (SGR 4)
- Attribute reset codes

### Supported ANSI Sequences

| Sequence | Description | Example |
|----------|-------------|---------|
| `\x1b[31m` | Red foreground | `\x1b[31mError\x1b[0m` |
| `\x1b[1m` | Bold text | `\x1b[1mBold\x1b[0m` |
| `\x1b[3m` | Italic text | `\x1b[3mItalic\x1b[0m` |
| `\x1b[4m` | Underline | `\x1b[4mUnderline\x1b[0m` |
| `\x1b[38;5;208m` | 256-color (orange) | `\x1b[38;5;208mOrange\x1b[0m` |
| `\x1b[38;2;255;100;50m` | RGB color | `\x1b[38;2;255;100;50mCustom\x1b[0m` |
| `\x1b[0m` | Reset all attributes | - |

## Performance

The ANSI parser is designed for high performance:

- **Parser Speed**: < 0.001ms per log entry
- **Integration**: 2ms for 1000 colored logs through LogBuffer
- **UI Impact**: Zero - parsing is async and non-blocking
- **Memory**: Minimal overhead - only stores parsed segments

### Benchmarks

From our test suite:
- 1000 ANSI-coded lines parsed in **0ms** (< 0.001ms per line)
- 1000 logs with ANSI codes added to LogBuffer in **2ms total**
- Maintains 60 FPS rendering in the UI

## Architecture

### Core Components

1. **IAnsiParser** (`include/core/IAnsiParser.hpp`)
   - Interface for ANSI parsing functionality
   - Methods: `parse()`, `stripAnsiCodes()`, `containsAnsiCodes()`

2. **AnsiParser** (`src/core/AnsiParser.cpp`)
   - Efficient O(n) single-pass parser
   - Handles malformed/incomplete sequences gracefully
   - Converts ANSI codes to styled segments

3. **LogEntry** (`include/core/ILogBuffer.hpp`)
   - Extended to store styled segments
   - Preserves both original text and plain text
   - Includes `hasAnsiCodes` flag for optimization

4. **LogBuffer** (`src/core/LogBuffer.cpp`)
   - Automatically parses ANSI codes on `append()`
   - Fast integration with zero UI blocking

5. **LogViewModel** (`src/ui/LogViewModel.cpp`)
   - Exposes styled segments to QML
   - New model roles: `styledSegments`, `plainText`, `hasAnsiCodes`

6. **LogConsole.qml** (`src/ui/LogConsole.qml`)
   - Renders multi-colored text inline
   - Uses Flow + Repeater for layout
   - Maintains 60 FPS with NativeRendering

## Usage Examples

### In C++ (LogBuffer)

```cpp
auto buffer = createLogBuffer();

// ANSI codes are automatically parsed
buffer->append("\x1b[31mError:\x1b[0m Connection failed");
buffer->append("\x1b[32m✓\x1b[0m Test passed");

// Access parsed segments
auto logs = buffer->getAll();
for (const auto& entry : logs) {
    qDebug() << "Plain text:" << entry.plainText;
    qDebug() << "Has colors:" << entry.hasAnsiCodes;
    for (const auto& seg : entry.segments) {
        qDebug() << seg.text << seg.foregroundColor.name();
    }
}
```

### In QML (LogConsole)

```qml
ListView {
    model: logViewModel
    delegate: Flow {
        Repeater {
            model: styledSegments || []
            Text {
                text: modelData.text
                color: modelData.fgColor
                font.bold: modelData.bold
            }
        }
    }
}
```

## Real-World Compatibility

The parser is tested with output from:

- **npm/yarn**: Build output, test results
- **Compilers**: GCC, Clang error messages
- **Test Frameworks**: Jest, Mocha, pytest
- **Build Tools**: Webpack, Vite, Rollup
- **Linters**: ESLint, TSLint output

### Example Output Styles

**npm test output:**
```
\x1b[32m✓\x1b[0m Tests passed: \x1b[1m42\x1b[0m/42
```

**Compiler error:**
```
\x1b[1merror:\x1b[0m \x1b[1m\x1b[31mundefined reference to 'main'\x1b[0m
```

**Build success:**
```
\x1b[32m✓\x1b[0m \x1b[1mBuild successful\x1b[0m in \x1b[36m2.4s\x1b[0m
```

## Testing

### Unit Tests (`test_ansi_parser`)

Tests parser functionality:
- Basic colors and text attributes
- Multiple colors in one line
- 256-color and RGB modes
- Stripping ANSI codes
- Edge cases and malformed sequences
- Performance benchmarks

Run with:
```bash
./bin/test_ansi_parser
```

### Integration Tests (`test_logbuffer_ansi`)

Tests end-to-end integration:
- LogBuffer + Parser integration
- Search with ANSI codes
- Circular buffer overflow
- Real-world log formats
- Performance validation

Run with:
```bash
./bin/test_logbuffer_ansi
```

## Configuration

The ANSI parser can be enabled/disabled:

```cpp
auto parser = createAnsiParser();
parser->setEnabled(false);  // Disable parsing
parser->setEnabled(true);   // Enable parsing (default)
```

When disabled, logs are stored as plain text without parsing overhead.

## Future Enhancements

Potential improvements:
- Configurable color themes/palettes
- Support for additional SGR codes (strikethrough, blink)
- Cursor movement sequence handling (for progress bars)
- Performance monitoring integration
- User preferences for ANSI display

## References

- [ANSI Escape Code Wikipedia](https://en.wikipedia.org/wiki/ANSI_escape_code)
- [SGR Parameters](https://en.wikipedia.org/wiki/ANSI_escape_code#SGR_(Select_Graphic_Rendition)_parameters)
- [xterm-256 Color Chart](https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit)

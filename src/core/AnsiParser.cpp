#include "AnsiParser.h"
#include <QRegularExpression>
#include <QDebug>

namespace ZenRunner::Core {

/**
 * @brief Fast ANSI/VT100 escape sequence parser implementation
 * 
 * This parser efficiently interprets ANSI color codes and SGR (Select Graphic Rendition)
 * sequences commonly used in terminal output. It supports:
 * - Standard colors (30-37 foreground, 40-47 background)
 * - Bright colors (90-97 foreground, 100-107 background)
 * - 256-color mode (38;5;n and 48;5;n)
 * - RGB/true color mode (38;2;r;g;b and 48;2;r;g;b)
 * - Text attributes (bold, italic, underline)
 * - Reset sequences
 * 
 * The parser is designed for high performance with O(n) complexity
 * where n is the length of the input text.
 */
class AnsiParser : public IAnsiParser {
public:
    AnsiParser() : enabled_(true) {}
    ~AnsiParser() override = default;

    std::vector<StyledSegment> parse(const QString& text) const override {
        if (!enabled_ || text.isEmpty()) {
            // Return single unstyled segment if parsing disabled or text empty
            StyledSegment segment;
            segment.text = text;
            return {segment};
        }

        std::vector<StyledSegment> segments;
        StyledSegment currentSegment;
        
        // Current text attributes
        QColor fgColor = QColor(Qt::white);
        QColor bgColor = QColor(Qt::transparent);
        bool bold = false;
        bool italic = false;
        bool underline = false;

        int i = 0;
        const int len = text.length();
        
        while (i < len) {
            // Look for ESC character (ASCII 27 or '\x1b')
            if (text[i] == QChar(0x1B) && i + 1 < len && text[i + 1] == '[') {
                // Found ANSI escape sequence start
                
                // Save current segment if it has text
                if (!currentSegment.text.isEmpty()) {
                    currentSegment.foregroundColor = fgColor;
                    currentSegment.backgroundColor = bgColor;
                    currentSegment.bold = bold;
                    currentSegment.italic = italic;
                    currentSegment.underline = underline;
                    segments.push_back(currentSegment);
                    currentSegment = StyledSegment();
                }
                
                // Parse the escape sequence
                i += 2; // Skip ESC[
                int start = i;
                
                // Find the end of the sequence (letter or 'm')
                while (i < len && !text[i].isLetter()) {
                    i++;
                }
                
                if (i < len) {
                    QChar commandChar = text[i];
                    QString params = text.mid(start, i - start);
                    
                    // We primarily care about 'm' (SGR - Select Graphic Rendition)
                    if (commandChar == 'm') {
                        // Parse SGR parameters
                        parseSgrSequence(params, fgColor, bgColor, bold, italic, underline);
                    }
                    // Other sequences (like cursor movement) are ignored for log display
                    
                    i++; // Move past the command character
                }
            } else {
                // Regular character, add to current segment
                currentSegment.text.append(text[i]);
                i++;
            }
        }
        
        // Add final segment if it has content
        if (!currentSegment.text.isEmpty()) {
            currentSegment.foregroundColor = fgColor;
            currentSegment.backgroundColor = bgColor;
            currentSegment.bold = bold;
            currentSegment.italic = italic;
            currentSegment.underline = underline;
            segments.push_back(currentSegment);
        }
        
        // If no segments were created (only ANSI codes), return empty segment
        if (segments.empty()) {
            segments.push_back(StyledSegment());
        }
        
        return segments;
    }

    QString stripAnsiCodes(const QString& text) const override {
        if (text.isEmpty()) {
            return text;
        }

        QString result;
        result.reserve(text.length()); // Pre-allocate for efficiency
        
        int i = 0;
        const int len = text.length();
        
        while (i < len) {
            // Look for ESC character (ASCII 27 or '\x1b')
            if (text[i] == QChar(0x1B) && i + 1 < len && text[i + 1] == '[') {
                // Found ANSI escape sequence start, skip it
                i += 2;
                
                // Skip until we find a letter (command character)
                while (i < len && !text[i].isLetter()) {
                    i++;
                }
                
                if (i < len) {
                    i++; // Skip the command character
                }
            } else {
                // Regular character, add to result
                result.append(text[i]);
                i++;
            }
        }
        
        return result;
    }

    bool containsAnsiCodes(const QString& text) const override {
        // Quick check for ESC[ sequence
        return text.contains(QChar(0x1B)) && text.contains('[');
    }

    void setEnabled(bool enabled) override {
        enabled_ = enabled;
    }

    bool isEnabled() const override {
        return enabled_;
    }

private:
    bool enabled_;

    /**
     * @brief Parse SGR (Select Graphic Rendition) parameters
     * @param params Parameter string (e.g., "1;31;40")
     * @param fgColor Current foreground color (modified)
     * @param bgColor Current background color (modified)
     * @param bold Current bold state (modified)
     * @param italic Current italic state (modified)
     * @param underline Current underline state (modified)
     */
    void parseSgrSequence(const QString& params, 
                          QColor& fgColor, 
                          QColor& bgColor,
                          bool& bold,
                          bool& italic,
                          bool& underline) const {
        if (params.isEmpty() || params == "0") {
            // Reset all attributes
            fgColor = QColor(Qt::white);
            bgColor = QColor(Qt::transparent);
            bold = false;
            italic = false;
            underline = false;
            return;
        }

        // Split parameters by semicolon
        QStringList codes = params.split(';', Qt::SkipEmptyParts);
        
        for (int i = 0; i < codes.size(); ++i) {
            bool ok;
            int code = codes[i].toInt(&ok);
            
            if (!ok) continue;
            
            // Process each SGR code
            if (code == 0) {
                // Reset all
                fgColor = QColor(Qt::white);
                bgColor = QColor(Qt::transparent);
                bold = false;
                italic = false;
                underline = false;
            }
            else if (code == 1) {
                bold = true;
            }
            else if (code == 3) {
                italic = true;
            }
            else if (code == 4) {
                underline = true;
            }
            else if (code == 22) {
                bold = false;
            }
            else if (code == 23) {
                italic = false;
            }
            else if (code == 24) {
                underline = false;
            }
            else if (code >= 30 && code <= 37) {
                // Standard foreground colors
                fgColor = getStandardColor(code - 30);
            }
            else if (code == 38) {
                // Extended foreground color
                if (i + 2 < codes.size() && codes[i + 1] == "5") {
                    // 256-color mode: 38;5;n
                    int colorIndex = codes[i + 2].toInt(&ok);
                    if (ok) {
                        fgColor = get256Color(colorIndex);
                    }
                    i += 2;
                }
                else if (i + 4 < codes.size() && codes[i + 1] == "2") {
                    // RGB mode: 38;2;r;g;b
                    int r = codes[i + 2].toInt(&ok);
                    int g = codes[i + 3].toInt(&ok);
                    int b = codes[i + 4].toInt(&ok);
                    if (ok) {
                        fgColor = QColor(r, g, b);
                    }
                    i += 4;
                }
            }
            else if (code == 39) {
                // Default foreground color
                fgColor = QColor(Qt::white);
            }
            else if (code >= 40 && code <= 47) {
                // Standard background colors
                bgColor = getStandardColor(code - 40);
            }
            else if (code == 48) {
                // Extended background color
                if (i + 2 < codes.size() && codes[i + 1] == "5") {
                    // 256-color mode: 48;5;n
                    int colorIndex = codes[i + 2].toInt(&ok);
                    if (ok) {
                        bgColor = get256Color(colorIndex);
                    }
                    i += 2;
                }
                else if (i + 4 < codes.size() && codes[i + 1] == "2") {
                    // RGB mode: 48;2;r;g;b
                    int r = codes[i + 2].toInt(&ok);
                    int g = codes[i + 3].toInt(&ok);
                    int b = codes[i + 4].toInt(&ok);
                    if (ok) {
                        bgColor = QColor(r, g, b);
                    }
                    i += 4;
                }
            }
            else if (code == 49) {
                // Default background color
                bgColor = QColor(Qt::transparent);
            }
            else if (code >= 90 && code <= 97) {
                // Bright foreground colors
                fgColor = getBrightColor(code - 90);
            }
            else if (code >= 100 && code <= 107) {
                // Bright background colors
                bgColor = getBrightColor(code - 100);
            }
        }
    }

    /**
     * @brief Get standard ANSI color (0-7)
     */
    QColor getStandardColor(int index) const {
        static const QColor colors[8] = {
            QColor(0, 0, 0),         // Black
            QColor(205, 49, 49),     // Red
            QColor(13, 188, 121),    // Green
            QColor(229, 229, 16),    // Yellow
            QColor(36, 114, 200),    // Blue
            QColor(188, 63, 188),    // Magenta
            QColor(17, 168, 205),    // Cyan
            QColor(229, 229, 229)    // White
        };
        
        if (index >= 0 && index < 8) {
            return colors[index];
        }
        return QColor(Qt::white);
    }

    /**
     * @brief Get bright ANSI color (0-7)
     */
    QColor getBrightColor(int index) const {
        static const QColor colors[8] = {
            QColor(102, 102, 102),   // Bright Black (Gray)
            QColor(241, 76, 76),     // Bright Red
            QColor(35, 209, 139),    // Bright Green
            QColor(245, 245, 67),    // Bright Yellow
            QColor(59, 142, 234),    // Bright Blue
            QColor(214, 112, 214),   // Bright Magenta
            QColor(41, 184, 219),    // Bright Cyan
            QColor(255, 255, 255)    // Bright White
        };
        
        if (index >= 0 && index < 8) {
            return colors[index];
        }
        return QColor(Qt::white);
    }

    /**
     * @brief Get 256-color palette color
     */
    QColor get256Color(int index) const {
        // Standard colors (0-15)
        if (index < 8) {
            return getStandardColor(index);
        }
        if (index >= 8 && index < 16) {
            return getBrightColor(index - 8);
        }
        
        // 216-color cube (16-231): 6x6x6 RGB cube
        if (index >= 16 && index < 232) {
            int cubeIndex = index - 16;
            int r = (cubeIndex / 36) * 51;
            int g = ((cubeIndex % 36) / 6) * 51;
            int b = (cubeIndex % 6) * 51;
            return QColor(r, g, b);
        }
        
        // Grayscale (232-255): 24 shades of gray
        if (index >= 232 && index < 256) {
            int gray = 8 + (index - 232) * 10;
            return QColor(gray, gray, gray);
        }
        
        return QColor(Qt::white);
    }
};

// Factory function to create AnsiParser instances
std::unique_ptr<IAnsiParser> createAnsiParser() {
    return std::make_unique<AnsiParser>();
}

} // namespace ZenRunner::Core

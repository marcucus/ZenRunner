#pragma once

#include <QString>
#include <QColor>
#include <vector>
#include <cstdint>

namespace ZenRunner::Core {

/**
 * @brief Styled text segment with color and formatting information
 */
struct StyledSegment {
    QString text;                    ///< Text content
    QColor foregroundColor;          ///< Foreground (text) color
    QColor backgroundColor;          ///< Background color
    bool bold{false};                ///< Bold text
    bool italic{false};              ///< Italic text
    bool underline{false};           ///< Underlined text
    
    StyledSegment() 
        : foregroundColor(Qt::white)
        , backgroundColor(Qt::transparent) 
    {}
};

/**
 * @brief Interface for ANSI/VT100 escape sequence parser
 * 
 * This interface defines the contract for parsing ANSI escape codes
 * in terminal output and converting them to styled text segments.
 * The parser must be efficient and work asynchronously to avoid
 * blocking the UI thread.
 */
class IAnsiParser {
public:
    virtual ~IAnsiParser() = default;

    /**
     * @brief Parse text with ANSI escape sequences
     * @param text Input text containing ANSI codes
     * @return Vector of styled segments
     */
    virtual std::vector<StyledSegment> parse(const QString& text) const = 0;

    /**
     * @brief Strip ANSI escape sequences from text
     * @param text Input text containing ANSI codes
     * @return Plain text without ANSI codes
     */
    virtual QString stripAnsiCodes(const QString& text) const = 0;

    /**
     * @brief Check if text contains ANSI escape sequences
     * @param text Input text
     * @return true if ANSI codes are detected
     */
    virtual bool containsAnsiCodes(const QString& text) const = 0;

    /**
     * @brief Enable or disable ANSI parsing
     * @param enabled true to enable parsing
     */
    virtual void setEnabled(bool enabled) = 0;

    /**
     * @brief Check if ANSI parsing is enabled
     * @return true if enabled
     */
    virtual bool isEnabled() const = 0;
};

} // namespace ZenRunner::Core

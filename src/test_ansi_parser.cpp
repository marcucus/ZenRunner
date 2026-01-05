#include "core/IAnsiParser.hpp"
#include "core/AnsiParser.h"
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <cassert>

using namespace ZenRunner::Core;

// Test helper to print styled segments
void printSegments(const QString& testName, const std::vector<StyledSegment>& segments) {
    qDebug() << "\n====" << testName << "====";
    for (size_t i = 0; i < segments.size(); ++i) {
        const auto& seg = segments[i];
        qDebug() << "Segment" << i << ":";
        qDebug() << "  Text:" << seg.text;
        qDebug() << "  FG Color:" << seg.foregroundColor.name();
        qDebug() << "  BG Color:" << seg.backgroundColor.name();
        qDebug() << "  Bold:" << seg.bold;
        qDebug() << "  Italic:" << seg.italic;
        qDebug() << "  Underline:" << seg.underline;
    }
}

// Test basic color parsing
void testBasicColors() {
    auto parser = createAnsiParser();
    
    // Test standard red text
    QString input = "\x1b[31mError message\x1b[0m";
    auto segments = parser->parse(input);
    
    assert(!segments.empty());
    assert(segments[0].text == "Error message");
    assert(segments[0].foregroundColor.red() > 150); // Should be reddish
    
    printSegments("Basic Red Color", segments);
    qDebug() << "✓ Basic colors test passed";
}

// Test bold and italic
void testTextAttributes() {
    auto parser = createAnsiParser();
    
    // Test bold text
    QString input = "\x1b[1mBold text\x1b[0m";
    auto segments = parser->parse(input);
    
    assert(!segments.empty());
    assert(segments[0].text == "Bold text");
    assert(segments[0].bold == true);
    
    printSegments("Bold Text", segments);
    
    // Test italic text
    input = "\x1b[3mItalic text\x1b[0m";
    segments = parser->parse(input);
    
    assert(!segments.empty());
    assert(segments[0].text == "Italic text");
    assert(segments[0].italic == true);
    
    printSegments("Italic Text", segments);
    
    qDebug() << "✓ Text attributes test passed";
}

// Test multiple colors in one line
void testMultipleColors() {
    auto parser = createAnsiParser();
    
    QString input = "\x1b[32mSuccess:\x1b[0m \x1b[33mWarning:\x1b[0m \x1b[31mError\x1b[0m";
    auto segments = parser->parse(input);
    
    assert(segments.size() >= 5); // Should have multiple segments including spaces
    
    printSegments("Multiple Colors", segments);
    qDebug() << "✓ Multiple colors test passed";
}

// Test stripping ANSI codes
void testStripAnsiCodes() {
    auto parser = createAnsiParser();
    
    QString input = "\x1b[31mColored\x1b[0m plain \x1b[32mtext\x1b[0m";
    QString stripped = parser->stripAnsiCodes(input);
    
    assert(stripped == "Colored plain text");
    qDebug() << "Stripped:" << stripped;
    qDebug() << "✓ Strip ANSI codes test passed";
}

// Test ANSI code detection
void testContainsAnsiCodes() {
    auto parser = createAnsiParser();
    
    QString withAnsi = "\x1b[31mRed text\x1b[0m";
    QString withoutAnsi = "Plain text";
    
    assert(parser->containsAnsiCodes(withAnsi) == true);
    assert(parser->containsAnsiCodes(withoutAnsi) == false);
    
    qDebug() << "✓ Contains ANSI codes test passed";
}

// Test complex real-world log output
void testRealWorldLogs() {
    auto parser = createAnsiParser();
    
    // Simulate npm output
    QString npmLog = "\x1b[32m✓\x1b[0m Tests passed: \x1b[1m42\x1b[0m/42";
    auto segments = parser->parse(npmLog);
    
    printSegments("NPM-style log", segments);
    
    // Simulate compiler error
    QString compilerError = "\x1b[1merror:\x1b[0m \x1b[1m\x1b[31mundefined reference to 'main'\x1b[0m";
    segments = parser->parse(compilerError);
    
    printSegments("Compiler error", segments);
    
    qDebug() << "✓ Real-world logs test passed";
}

// Test 256-color mode
void test256Colors() {
    auto parser = createAnsiParser();
    
    // Test 256-color foreground
    QString input = "\x1b[38;5;208mOrange text\x1b[0m";
    auto segments = parser->parse(input);
    
    assert(!segments.empty());
    assert(segments[0].text == "Orange text");
    
    printSegments("256-color mode", segments);
    qDebug() << "✓ 256-color test passed";
}

// Test RGB true color mode
void testTrueColor() {
    auto parser = createAnsiParser();
    
    // Test RGB foreground (255, 100, 50)
    QString input = "\x1b[38;2;255;100;50mRGB text\x1b[0m";
    auto segments = parser->parse(input);
    
    assert(!segments.empty());
    assert(segments[0].text == "RGB text");
    assert(segments[0].foregroundColor.red() == 255);
    assert(segments[0].foregroundColor.green() == 100);
    assert(segments[0].foregroundColor.blue() == 50);
    
    printSegments("RGB true color", segments);
    qDebug() << "✓ RGB true color test passed";
}

// Performance test
void testPerformance() {
    auto parser = createAnsiParser();
    
    // Create a large log with many ANSI codes
    QString largeLog;
    for (int i = 0; i < 1000; ++i) {
        largeLog += QString("\x1b[%1mLine %2\x1b[0m\n").arg(31 + (i % 7)).arg(i);
    }
    
    QElapsedTimer timer;
    timer.start();
    
    auto segments = parser->parse(largeLog);
    
    qint64 elapsed = timer.elapsed();
    
    qDebug() << "\n==== Performance Test ====";
    qDebug() << "Parsed 1000 colored lines in" << elapsed << "ms";
    qDebug() << "Segments created:" << segments.size();
    qDebug() << "Average time per line:" << (double)elapsed / 1000.0 << "ms";
    
    // Performance target: should parse 1000 lines in less than 100ms
    assert(elapsed < 100);
    
    qDebug() << "✓ Performance test passed (< 100ms)";
}

// Test empty and edge cases
void testEdgeCases() {
    auto parser = createAnsiParser();
    
    // Empty string
    auto segments = parser->parse("");
    assert(segments.size() == 1); // Should have one empty segment
    
    // Only ANSI codes
    segments = parser->parse("\x1b[31m\x1b[0m");
    assert(segments.size() >= 1);
    
    // Incomplete ANSI code
    segments = parser->parse("Text \x1b[ incomplete");
    assert(!segments.empty());
    
    // Text without ANSI codes
    segments = parser->parse("Plain text without colors");
    assert(segments.size() == 1);
    assert(segments[0].text == "Plain text without colors");
    
    qDebug() << "✓ Edge cases test passed";
}

// Test enable/disable parsing
void testEnableDisable() {
    auto parser = createAnsiParser();
    
    QString input = "\x1b[31mRed text\x1b[0m";
    
    // Enabled
    assert(parser->isEnabled() == true);
    auto segments = parser->parse(input);
    assert(!segments.empty());
    
    // Disabled
    parser->setEnabled(false);
    assert(parser->isEnabled() == false);
    segments = parser->parse(input);
    assert(segments.size() == 1);
    assert(segments[0].text == input); // Should return unparsed
    
    // Re-enable
    parser->setEnabled(true);
    segments = parser->parse(input);
    assert(segments[0].text == "Red text");
    
    qDebug() << "✓ Enable/disable test passed";
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "==============================================";
    qDebug() << "    ANSI Parser Test Suite";
    qDebug() << "==============================================\n";
    
    testBasicColors();
    testTextAttributes();
    testMultipleColors();
    testStripAnsiCodes();
    testContainsAnsiCodes();
    testRealWorldLogs();
    test256Colors();
    testTrueColor();
    testEdgeCases();
    testEnableDisable();
    testPerformance();
    
    qDebug() << "\n==============================================";
    qDebug() << "    ✓ ALL TESTS PASSED";
    qDebug() << "==============================================\n";
    
    return 0;
}

//
// Date       : 18/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include <utility>

#include "common/text_mod_common.h"

namespace tm_parse {

// Bad name but fuck it, good enough
class ErrorWithContext : public std::runtime_error {
   public:
    using LineContext = std::optional<std::deque<str>>;

   private:
    size_t m_LineNumber;
    size_t m_PosInLine;
    std::optional<str> m_ErrorLine;
    LineContext m_ContextLines;

   public:
    using std::runtime_error::what;

   public:
    explicit ErrorWithContext(
        const std::string& msg,
        size_t line_number = 0,
        size_t pos_in_line = 0,
        std::optional<str> error_line = std::nullopt,
        LineContext context_lines = std::nullopt
    )
        : std::runtime_error(msg),
          m_LineNumber(line_number),
          m_PosInLine(pos_in_line),
          m_ErrorLine(std::move(error_line)),
          m_ContextLines(std::move(context_lines)) {};

   public:
    [[nodiscard]] bool has_context() const noexcept { return m_ContextLines.has_value(); }
    [[nodiscard]] bool has_error_line() const noexcept { return m_ErrorLine.has_value(); }

   public:
    [[nodiscard]] size_t line_number() const noexcept { return m_LineNumber; }
    [[nodiscard]] size_t pos_in_line() const noexcept { return m_PosInLine; }
    [[nodiscard]] const LineContext::value_type& context_lines() const noexcept { return *m_ContextLines; }
    [[nodiscard]] const str& error_line() const noexcept { return *m_ErrorLine; }

    [[nodiscard]] std::string error_caret() const noexcept {
        if (!has_error_line() || m_ErrorLine->empty()) {
            return "Has no error line";
        }

        if (m_PosInLine == 0) {
            return "^";
        }

        // We assume the error is at the end of the error line
        std::string line(m_ErrorLine->length(), ' ');
        line[line.length() - 1] = '^';
        return line;
    }

   public:
    std::string build_error_message() const noexcept {
        std::stringstream out{};

        // Primary error line
        out << "(ERROR) ~ " << what() << " at line " << line_number() << "\n";

        // Dump the context line if it exists
        if (has_context()) {
            for (const str& line : context_lines()) {
                out << "  > " << utils::narrow(line) << "\n";
            }
        }

        // Dump the error line if it exists
        if (has_error_line()) {
            out << "  > " << utils::narrow(error_line()) << "\n";
            out << "  > " << error_caret() << "\n";
        }

        return out.str();
    }
};

}  // namespace tm_parse
//
// Date       : 18/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

#include "common/text_mod_common.h"
#include "common/text_mod_tokens.h"

namespace tm_parse {

class TextModLexer;
class TextModParser;

class ParsingError : public std::runtime_error {
   private:
    TokenTextView m_ErrorRegion;
    std::deque<TokenTextView> m_ContextLines;
    size_t m_LineNumber;
    std::optional<std::string> m_ExtraInfo;

   public:
    ParsingError(
        const std::string& msg,
        const TokenTextView& error_region,
        std::deque<TokenTextView>&& context_lines,
        size_t line_number,
        decltype(m_ExtraInfo)&& extra_info = std::nullopt
    );
    ~ParsingError() = default;

   public:
    const TokenTextView& error_region() const { return m_ErrorRegion; }
    const std::deque<TokenTextView>& context_lines() const { return m_ContextLines; }
    size_t line_number() const { return m_LineNumber; }
    const std::optional<std::string>& extra_info() const { return m_ExtraInfo; }

    std::string build_error_message(str_view src_text) const;
    std::string build_error_message(const TextModLexer& lexer) const;
    std::string build_error_message(const TextModParser& parser) const;
};

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

    [[nodiscard]] str error_caret() const noexcept {
        if (!has_error_line() || m_ErrorLine->empty()) {
            return TXT("Has no error line");
        }

        if (m_PosInLine == 0) {
            return TXT("^");
        }

        // We assume the error is at the end of the error line
        str line(m_ErrorLine->length(), TXT(' '));
        line[line.length() - 1] = TXT('^');
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
                out << "  > " << to_str<std::string>(line) << "\n";
            }
        }

        // Dump the error line if it exists
        if (has_error_line()) {
            out << "  > " << to_str<std::string>(error_line()) << "\n";
            out << "  > " << to_str<std::string>(error_caret()) << "\n";
        }

        return out.str();
    }
};

}  // namespace tm_parse
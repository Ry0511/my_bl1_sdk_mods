//
// Date       : 14/07/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"

#include "common/text_mod_errors.h"
#include "lexer/text_mod_lexer.h"
#include "parser/text_mod_parser.h"

namespace tm_parse {

ParsingError::ParsingError(
    const std::string& msg,
    const TokenTextView& error_region,
    std::deque<TokenTextView>&& context_lines,
    size_t line_number,
    decltype(m_ExtraInfo)&& extra_info
)
    : std::runtime_error(msg),
      m_ErrorRegion(error_region),
      m_ContextLines(std::move(context_lines)),
      m_LineNumber(line_number),
      m_ExtraInfo(extra_info) {}

ParsingError ParsingError::create(std::string&& msg, const TextModLexer& lexer) {
    size_t line_number = lexer.get_line_number(lexer.position());
    const TokenTextView& error_line = lexer.get_line(lexer.position());

    const str_view tx = lexer.text();
    std::deque<TokenTextView> context_lines{};
    size_t line_start = lexer.get_line_start(lexer.position());

    for (size_t i = 0; i < 5; ++i) {

        if (line_start > 0 && line_start < tx.size()) {
            line_start = lexer.get_line_start(line_start - 2);
        } else {
            break;
        }

        size_t line_end = lexer.get_line_end(line_start);
        str_view ln = tx.substr(line_start, line_end - line_start);
        context_lines.emplace_front(line_start, line_end - line_start);
    }

    return ParsingError(std::move(msg), error_line, std::move(context_lines), line_number, std::nullopt);
}

ParsingError ParsingError::create(std::string&& msg, const TextModParser& parser) {
    return create(std::move(msg), *parser.lexer());
}

std::string ParsingError::build_error_message(str_view src_text) const {
    std::stringstream out{};

    // Error: This text failed to parse
    //   > Context Line 1
    //   > Context Line 2
    //   > Context Line N
    //   > Error Line

    out << what() << " at line " << line_number() << "\n";

    for (const TokenTextView& line : context_lines()) {
        str_view vw = line.view_from(src_text);

        if (vw.empty()) {
            out << "  >\n";
        } else {
            out << "  > " << to_str<std::string>(str{vw});
            if (vw.back() != '\n') {
                out << "\n";
            }
        }
    }

    if (error_region().is_valid()) {
        str_view error_line = error_region().view_from(src_text);
        out << "  > " << to_str<std::string>(str{error_line});
        if (error_line.back() != '\n') {
            out << "\n";
        }
    }

    if (extra_info()) {
        out << "\nAdditional Information\n";

        std::string info = extra_info().value();
        std::stringstream info_stream(info);
        for (std::string line; std::getline(info_stream, line);) {
            if (line.empty()) {
                out << "\n";
                continue;
            }

            // Just assuming this never fails
            size_t start = line.find_first_not_of(' ');
            std::string_view trimmed_line = line.substr(start);
            out << "  " << trimmed_line << "\n";
        }
    }

    return out.str();
}

std::string ParsingError::build_error_message(const TextModLexer& lexer) const {
    return build_error_message(lexer.text());
}

std::string ParsingError::build_error_message(const TextModParser& parser) const {
    return build_error_message(parser.text());
}

}  // namespace tm_parse
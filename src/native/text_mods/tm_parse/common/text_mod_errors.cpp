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

std::string ParsingError::build_error_message(str_view src_text) const {
    std::stringstream out{};

    // Error: This text failed to parse
    //   > Context Line 1
    //   > Context Line 2
    //   > Context Line N
    //   > Error Line

    out << "Error: " << what() << " at line " << line_number() << "\n";

    // TODO: Escape special characters such as \r \n \b \0
    for (const TokenTextView& line : context_lines()) {
        out << "  > " << to_str<std::string>(str{line.view_from(src_text)}) << "\n";
    }

    if (error_region().is_valid()) {
        out << " > " << to_str<std::string>(str{error_region().view_from(src_text)}) << "\n";
    }

    if (extra_info()) {
        out << "\nAdditional Information\n";

        std::string info = extra_info().value();
        std::stringstream info_stream(info);
        for (std::string line; std::getline(info_stream, line);) {

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
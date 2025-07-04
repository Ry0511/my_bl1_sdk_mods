//
// Date       : 16/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"

#define CATCH_CONFIG_RUNNER
#include "tests/catch.hpp"

#include "common/text_mod_common.h"
#include "lexer/text_mod_lexer.h"
#include "parser/text_mod_parser.h"

////////////////////////////////////////////////////////////////////////////////
// | MAIN |
////////////////////////////////////////////////////////////////////////////////

using namespace tm_parse;

int main() {
    TXT_LOG("== SYMBOLS =====================================================================");
    for (TokenProxy proxy : SymbolTokenIterator{}) {
        TXT_LOG("  {:>2} -> {}", proxy.as_int(), proxy.as_str());
    }

    TXT_LOG("== KEYWORDS ====================================================================");
    for (TokenProxy proxy : KeywordTokenIterator{}) {
        TXT_LOG("  {:>2} -> {}", proxy.as_int(), proxy.as_str());
    }

    {
        fs::path wpc_dump = fs::current_path() / "wpc_obj_dump_utf-8.txt";
        if (fs::is_regular_file(wpc_dump)) {
            std::wifstream stream{wpc_dump};
            using It = std::istreambuf_iterator<wchar_t>;
            str content = std::wstring{It{stream}, It{}};

            TextModLexer lexer{content};
            TextModParser parser{&lexer};

            try {
                ProgramRule program = ProgramRule::create(parser);
                fs::path parse_result_path = fs::current_path() / "00_parse_result.txt";
                TXT_LOG("Parse result path: {}", std::wstring{parse_result_path.c_str()});
                std::ofstream ofs{parse_result_path, std::ios::trunc};

                ofs << "#\n";
                ofs << "# Program Rule\n";
                ofs << std::format("#   > Rule Count: {}\n", program.rules().size());

                for (const auto& rule : program.rules()) {
                    std::visit(
                        [&parser, &ofs](auto&& inner) {
                            using T = std::decay_t<decltype(inner)>;
                            if constexpr (std::is_same_v<T, SetCommandRule>) {
                                ofs << std::format("{}\n", str{inner.to_string(parser)});
                            }

                            // Object definitions
                            else if (std::is_same_v<T, ObjectDefinitionRule>) {
                                const ObjectDefinitionRule& r = inner;

                                ofs << std::format(
                                    "Begin Object Class={} Name={}\n",
                                    str{r.clazz().to_string(parser)},
                                    str{r.name().to_string(parser)}
                                );

                                for (const auto& obj : r.child_objects()) {
                                    ofs << std::format(
                                        "# Child Object: Class={} Name={}\n",
                                        str{obj->clazz().to_string(parser)},
                                        str{obj->name().to_string(parser)}
                                    );
                                }

                                for (const auto& prop : r.assignments()) {
                                    ofs << std::format("  {}\n", str{prop.to_string(parser)});
                                }

                                ofs << "End Object\n\n";
                            }
                        },
                        rule
                    );
                }
            } catch (const std::runtime_error& err) {
                TXT_LOG("Error parsing wpc dump\n {}", err.what());
            }
        }
    }

    // Will pickup tests in linked source files
    int result = Catch::Session().run();
    TXT_LOG("Lexer test main exited with: {}", result);

    return 0;
}

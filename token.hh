#pragma once

#include<string_view>
#include"xyz.hh"

namespace zlt::ilispc {
  namespace token {
    enum {
      E0F,
      EOL,
      ID,
      NUMBER,
      STRING,
      SYMBOL
    };

    static inline constexpr std::initializer_list<std::string_view> symbols = {
      // keywords begin
      "callee",
      "def",
      "defer",
      "forward",
      "guard",
      "if",
      "length",
      "return",
      "throw",
      "try",
      // keywords end
      // preproc directions begin
      "#",
      "##",
      "#def",
      "#if...",
      "#ifdef",
      "#ifn...",
      "#ifndef",
      "#include",
      "#movedef",
      "#undef",
      // preproc directions end
      "!",
      "%",
      "&",
      "&&",
      "(",
      ")",
      "*",
      "**",
      "+",
      ",",
      "-",
      ".",
      "/",
      "<",
      "<<",
      "<=",
      "<=>",
      "=",
      "==",
      ">",
      ">=",
      ">>",
      ">>>",
      "@",
      "^",
      "^^",
      "|",
      "||",
      "~"
    };

    template<size_t N>
    struct Symbol {
      int value;
      consteval Symbol(const char (&s)[N]): value(SYMBOL + initializerListIndexOf(symbols, s)) {}
    };

    template<int T>
    using Constant = std::integral_constant<int, T>;

    template<int ...T>
    using Sequence = std::integer_sequence<int, T...>;

    bool isNumber(double &dest, std::string_view raw) noexcept;
    bool isSymbol(int &dest, std::string_view raw) noexcept;
    std::string_view raw(int t) noexcept;
  }

  template<token::Symbol symbol>
  consteval int operator ""_token() {
    return symbol.value;
  }
}

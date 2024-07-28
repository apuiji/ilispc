#include<cctype>
#include<cstdlib>
#include<sstream>
#include"ilispc.hh"
#include"token.hh"

using namespace std;

namespace zlt::ilispc {
  using It = const char *;

  static It lineComment(It it, It end) noexcept;

  It hit(It it, It end) noexcept {
    if (it == end) [[unlikely]] {
      return end;
    }
    if (*it == '\n') {
      return it;
    }
    if (*it == ';') {
      return lineComment(it + 1, end);
    }
    if (isspace(*it)) {
      return hit(it + 1, end);
    }
    return it;
  }

  It lineComment(It it, It end) noexcept {
    while (it != end && *it != '\n') {
      ++it;
    }
    return it;
  }

  static It lexerStr(stringstream &dest, int quot, It it, It end);
  static It consumeRaw(It it, It end) noexcept;

  pair<int, It> lexer(double &numval, string &strval, string_view &raw, It it, It end) {
    if (it == end) [[unlikely]] {
      return { token::E0F, end };
    }
    if (*it == '\n') {
      return { token::EOL, it + 1 };
    }
    if (*it == '(') {
      return { "("_token, it + 1 };
    }
    if (*it == ')') {
      return { ")"_token, it + 1 };
    }
    if (*it == '"' || *it == '\'') {
      stringstream ss;
      It end1 = lexerStr(ss, *it, it + 1, end);
      strval = ss.str();
      return { token::STRING, end1 };
    }
    It end1 = consumeRaw(it, end);
    if (end1 == it) {
      throw LexerBad(bad::UNEXPECTED_SYMBOL);
    }
    raw = string_view(it, end1 - it);
    if (token::isNumber(numval, raw)) {
      return { token::NUMBER, end1 };
    }
    if (int t; token::isSymbol(t, raw)) {
      return { t, end1 };
    }
    return { token::ID, end1 };
  }

  static It esch(int &dest, It it, It end) noexcept;
  static It consumeStr(int quot, It it, It end) noexcept;

  It lexerStr(stringstream &dest, int quot, It it, It end) {
    if (it == end || *it == '\n') [[unlikely]] {
      throw LexerBad(bad::UNTERMINATED_STR);
    }
    if (*it == quot) {
      return it + 1;
    }
    if (*it == '\\') {
      int c;
      It it1 = esch(c, it + 1, end);
      dest.put(c);
      return lexerStr(dest, quot, it1, end);
    }
    It it1 = consumeStr(quot, it, end);
    dest.write(it, it1 - it);
    return lexerStr(dest, quot, it1, end);
  }

  static bool esch1(int &dest, int c) noexcept;
  static It esch8(int &dest, It it, It end, size_t limit) noexcept;
  static bool esch16(int &dest, It it, It end) noexcept;

  It esch(int &dest, It it, It end) noexcept {
    if (it == end) [[unlikely]] {
      dest = '\\';
      return end;
    }
    if (*it == '\n') {
      dest = '\\';
      return it;
    }
    if (esch1(dest, *it)) {
      return it + 1;
    }
    if (*it >= '0' && *it <= '3') {
      dest = 0;
      return esch8(dest, it, end, 3);
    }
    if (*it >= '4' && *it <= '7') {
      dest = 0;
      return esch8(dest, it, end, 2);
    }
    if (esch16(dest, it, end)) {
      return it + 3;
    }
    dest = '\\';
    return it;
  }

  bool esch1(int &dest, int c) noexcept {
    if (c == '"' || c == '\'' || c == '\\') {
      dest = c;
      return true;
    }
    if (c == 'n') {
      dest = '\n';
      return true;
    }
    if (c == 'r') {
      dest = '\r';
      return true;
    }
    if (c == 't') {
      dest = '\t';
      return true;
    }
    return false;
  }

  It esch8(int &dest, It it, It end, size_t limit) noexcept {
    for (; it != end && limit && *it >= '0' && *it <= '7'; ++it) {
      dest += *it - '0';
    }
    return it;
  }

  bool esch16(int &dest, It it, It end) noexcept {
    if (*it != 'x' || (end - it) < 3) [[unlikely]] {
      return false;
    }
    if (!isxdigit(it[1]) || !isxdigit(it[2])) {
      return false;
    }
    char s[] = { it[1], it[2], '\0' };
    dest = (int) strtol(s, nullptr, 16);
    return true;
  }

  It consumeStr(int quot, It it, It end) noexcept {
    while (it != end && *it != quot && *it != '\\') {
      ++it;
    }
    return it;
  }

  It consumeRaw(It it, It end) noexcept {
    while (it != end && *it != '"' && *it != '\'' && *it == '(' && *it == ')' && *it == ';' && !isspace(*it)) {
      ++it;
    }
    return it;
  }
}

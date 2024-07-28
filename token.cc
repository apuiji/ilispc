#include<algorithm>
#include<regex>
#include"token.hh"

using namespace std;

namespace zlt::ilispc::token {
  static const regex reInt2("([+-]?)0[Bb]([01]+)");
  static const regex reInt4("([+-]?)0[Qq]([0-3]+)");
  static const regex reInt8("([+-]?)0[Oo]([0-7]+)");
  static const regex reInt16("([+-]?)0[Xx]([[:xdigit:]]+)");

  static bool isIntX(double &dest, const regex &re, size_t base, string_view raw) {
    cmatch m;
    if (!regex_match(raw.data(), raw.data() + raw.size(), m, re)) {
      return false;
    }
    string s = m.str(1) + m.str(2);
    dest = stoi(s, nullptr, base);
    return true;
  }

  bool isNumber(double &dest, string_view raw) noexcept {
    if (isIntX(dest, reInt2, 2, raw)) {
      return true;
    }
    if (isIntX(dest, reInt4, 4, raw)) {
      return true;
    }
    if (isIntX(dest, reInt8, 8, raw)) {
      return true;
    }
    if (isIntX(dest, reInt16, 16, raw)) {
      return true;
    }
    size_t n;
    dest = stod(string(raw), &n);
    return n == raw.size();
  }

  bool isSymbol(int &dest, string_view raw) noexcept {
    dest = SYMBOL;
    for (auto s : symbols) {
      if (raw == s) {
        return true;
      }
      ++dest;
    }
    return false;
  }

  string_view raw(int t) noexcept {
    return initializerListGet(symbols, t - SYMBOL);
  }
}

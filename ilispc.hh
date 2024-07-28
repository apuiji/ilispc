#pragma once

#include<list>
#include<map>
#include<memory>
#include<set>
#include<string>
#include<vector>

namespace zlt::ilispc {
  struct Pos {
    const std::string *file;
    int li;
    const Pos *prev;
    Pos(const std::string *file, int li, const Pos *prev = nullptr) noexcept: file(file), li(li), prev(prev) {}
  };

  struct PosSetCompare {
    bool operator ()(const Pos &a, const Pos &b) const noexcept;
  };

  using PosSet = std::set<Pos, PosSetCompare>;

  struct Node {
    const Pos *pos;
    Node(const Pos *pos) noexcept: pos(pos) {}
    virtual ~Node() = default;
  };

  using UniqNode = std::unique_ptr<Node>;
  using UniqNodes = std::list<UniqNode>;

  // bad begin
  struct Bad {
    int code;
    const Pos *pos;
    Bad(int code, const Pos *pos) noexcept: code(code), pos(pos) {}
  };

  namespace bad {
    enum {
      ILL_DEF,
      INV_INCLUDE_FILE,
      INV_LHS_EXPR,
      MACRO_ALREADY_DEFINED,
      UNEXPECTED_SYMBOL,
      UNEXPECTED_TOKEN,
      UNTERMINATED_LIST,
      UNTERMINATED_STR
    };
  }

  struct LexerBad {
    int code;
    LexerBad(int code) noexcept: code(code) {}
  };
  // bad end

  const char *hit(const char *it, const char *end) noexcept;

  /// @return [token, next]
  /// @throw LexerBad
  std::pair<int, const char *> lexer(
    double &numval, std::string &strval, std::string_view &raw, const char *it, const char *end);

  static inline std::pair<int, const char *> lexer(const char *it, const char *end) {
    double d;
    std::string s;
    std::string_view sv;
    return lexer(d, s, sv, it, end);
  }

  static inline std::pair<int, const char *> lexer(
    double &numval, std::string &strval, std::string_view &raw, const Pos *pos, const char *it, const char *end) {
    try {
      return lexer(numval, strval, raw, it, end);
    } catch (LexerBad lb) {
      throw Bad(lb.code, pos);
    }
  }

  static inline std::pair<int, const char *> lexer(const Pos *pos, const char *it, const char *end) {
    try {
      return lexer(it, end);
    } catch (LexerBad lb) {
      throw Bad(lb.code, pos);
    }
  }

  struct ParseContext {
    std::set<std::string> &strs;
    PosSet &posSet;
    ParseContext(std::set<std::string> &strs, PosSet &posSet) noexcept: strs(strs), posSet(posSet) {}
  };

  /// @throw Bad
  void parse(UniqNodes &dest, ParseContext &ctx, const Pos *pos, const char *it, const char *end);

  namespace preproc {
    struct Macro {
      using Params = std::vector<const std::string *>;
      Params params;
      UniqNodes body;
    };

    using Macros = std::map<const std::string *, Macro>;

    struct Context {
      std::set<std::string> &strs;
      PosSet &posSet;
      Macros &macros;
      Context(std::set<std::string> &strs, PosSet &posSet, Macros &macros) noexcept: strs(strs), posSet(posSet), macros(macros)
      {}
    };

    /// @throw Bad
    void preproc(UniqNodes &dest, Context &ctx, UniqNode &src);

    static inline void preproc(UniqNodes &dest, Context &ctx, UniqNodes &src) {
      for (; src.size(); src.pop_front()) {
        preproc(dest, ctx, src.front());
      }
    }

    void expand(UniqNodes &dest, Context &ctx, const Pos *pos, const Macro &macro, UniqNodes &src);
  }

  void trans(std::set<const std::string *> &defs, UniqNode &src);

  static inline void trans(std::set<const std::string *> &defs, UniqNodes::iterator it, UniqNodes::iterator end) {
    for (; it != end; ++it) {
      trans(defs, *it);
    }
  }

  void optimize(UniqNodes &dest, bool global, UniqNodes &src);
}

#include<algorithm>
#include<filesystem>
#include<fstream>
#include<iterator>
#include<sstream>
#include"ilispc.hh"
#include"nodes.hh"
#include"token.hh"
#include"xyz.hh"

using namespace std;

namespace zlt::ilispc::preproc {
  static void preprocList(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls);

  void preproc(UniqNodes &dest, Context &ctx, UniqNode &src) {
    if (auto ls = dynamic_cast<ListNode *>(src.get()); ls) {
      preprocList(dest, ctx, src, *ls);
    } else {
      dest.push_back(std::move(src));
    }
  }

  static const Macro *isMacro(const Macros &macros, const UniqNode &src) noexcept;
  static void preprocList1(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls);

  using token::Constant;
  using token::Sequence;

  static void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#"_token>);
  static void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"##"_token>);
  static void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#def"_token>);
  static void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#if..."_token>);
  static void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#ifdef"_token>);
  static void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#ifn..."_token>);
  static void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#ifndef"_token>);
  static void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#include"_token>);
  static void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#movedef"_token>);
  static void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#undef"_token>);

  template<int T, int ...U>
  inline void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, int t, Sequence<T, U...>) {
    if (t == T) {
      preproc(dest, ctx, src, ls, Constant<T>());
      return;
    }
    if constexpr (sizeof...(U)) {
      preproc(dest, ctx, src, ls, t, Sequence<U...>());
    } else {
      preprocList1(dest, ctx, src, ls);
    }
  }

  void preprocList(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls) {
    if (ls.items.empty()) [[unlikely]] {
      dest.push_back(std::move(src));
      return;
    }
    auto &a = ls.items.front();
    if (auto m = isMacro(ctx.macros, a); m) {
      ls.items.pop_front();
      UniqNodes ex;
      expand(ex, ctx, ls.pos, *m, ls.items);
      preproc(dest, ctx, ex);
      return;
    }
    auto tn = dynamic_cast<const TokenNode *>(a.get());
    if (!tn) {
      preprocList1(dest, ctx, src, ls);
      return;
    }
    using S = Sequence<
      "#"_token,
      "##"_token,
      "#def"_token,
      "#if..."_token,
      "#ifdef"_token,
      "#ifn..."_token,
      "#ifndef"_token,
      "#include"_token,
      "#movedef"_token,
      "#undef"_token>;
    preproc(dest, ctx, src, ls, tn->token, S());
  }

  void preprocList1(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls) {
    UniqNodes items;
    preproc(items, ctx, ls.items);
    ls.items = std::move(items);
    dest.push_back(std::move(src));
  }

  static inline const string *isID(const UniqNode &src) noexcept {
    auto id = dynamic_cast<const ID *>(src.get());
    return id ? id->raw : nullptr;
  }

  const Macro *isMacro(const Macros &macros, const UniqNode &src) noexcept {
    auto id = isID(src);
    if (!id) {
      return nullptr;
    }
    auto it = macros.find(id);
    if (it == macros.end()) {
      return nullptr;
    }
    return &it->second;
  }

  static string_view toStr(const UniqNode &src);

  void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#"_token>) {
    ls.items.pop_front();
    string_view raw;
    if (ls.items.size()) {
      raw = toStr(ls.items.front());
    }
    auto itStr = ctx.strs.insert(string(raw)).first;
    dest.push_back({});
    dest.back().reset(new StringLiteral(src->pos, &*itStr));
  }

  string_view toStr(const UniqNode &src) {
    auto rn = dynamic_cast<const RawNode *>(src.get());
    if (!rn) {
      throw Bad(bad::UNEXPECTED_TOKEN, src->pos);
    }
    return rn->getRaw();
  }

  static void rawCat(stringstream &dest, UniqNodes &src);

  void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"##"_token>) {
    ls.items.pop_front();
    string raw;
    {
      stringstream ss;
      rawCat(ss, ls.items);
      raw = ss.str();
    }
    dest.push_back({});
    if (double d; token::isNumber(d, raw)) {
      auto itRaw = ctx.strs.insert(std::move(raw)).first;
      dest.back().reset(new NumberNode(src->pos, d, &*itRaw));
    } else if (int t; token::isSymbol(t, raw)) {
      dest.back().reset(new TokenNode(src->pos, t));
    } else {
      auto itRaw = ctx.strs.insert(std::move(raw)).first;
      dest.back().reset(new ID(src->pos, &*itRaw));
    }
  }

  void rawCat(stringstream &dest, UniqNodes &src) {
    for (; src.size(); src.pop_front()) {
      auto &a = src.front();
      auto rn = dynamic_cast<const RawNode *>(a.get());
      if (!rn) {
        throw Bad(bad::UNEXPECTED_TOKEN, a->pos);
      }
      dest << rn->getRaw();
    }
  }

  static void makeMacro(Macro &dest, Context &ctx, UniqNodes &src);

  void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#def"_token>) {
    ls.items.pop_front();
    if (ls.items.empty()) [[unlikely]] {
      return;
    }
    auto id = isID(ls.items.front());
    if (!id) {
      throw Bad(bad::UNEXPECTED_TOKEN, ls.items.front()->pos);
    }
    ls.items.pop_front();
    if (ctx.macros.find(id) != ctx.macros.end()) {
      throw Bad(bad::MACRO_ALREADY_DEFINED, src->pos);
    }
    makeMacro(ctx.macros[id], ctx, ls.items);
  }

  static void makeMacroParams(Macro::Params &dest, Context &ctx, UniqNodes &src);
  static void clearPrevPos(PosSet &posSet, UniqNode &src);

  static inline void clearPrevPos(PosSet &posSet, UniqNodes &src) {
    for (auto &a : src) {
      clearPrevPos(posSet, a);
    }
  }

  void makeMacro(Macro &dest, Context &ctx, UniqNodes &src) {
    if (src.empty()) [[unlikely]] {
      return;
    }
    auto &a = src.front();
    auto ls = dynamic_cast<ListNode *>(a.get());
    if (!ls) {
      throw Bad(bad::UNEXPECTED_TOKEN, a->pos);
    }
    makeMacroParams(dest.params, ctx, ls->items);
    dest.params.shrink_to_fit();
    src.pop_front();
    clearPrevPos(ctx.posSet, src);
    dest.body = std::move(src);
  }

  void makeMacroParams(Macro::Params &dest, Context &ctx, UniqNodes &src) {
    for (; src.size(); src.pop_front()) {
      auto &a = src.front();
      if (auto id = isID(a); id) {
        dest.push_back(id);
        if (string_view(*id).starts_with("...")) {
          break;
        }
      } else if (auto ls = dynamic_cast<const ListNode *>(a.get()); ls && ls->items.empty()) {
        dest.push_back(nullptr);
      } else {
        throw Bad(bad::UNEXPECTED_TOKEN, a->pos);
      }
    }
  }

  void clearPrevPos(PosSet &posSet, UniqNode &src) {
    if (auto ls = dynamic_cast<ListNode *>(src.get()); ls) {
      clearPrevPos(posSet, ls->items);
    }
    auto itPos = posSet.insert(Pos(src->pos->file, src->pos->li)).first;
    src->pos = &*itPos;
  }

  static inline void preprocIf(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, auto predicate) {
    ls.items.pop_front();
    if (ls.items.size() && predicate(ls.items.front())) {
      ls.items.pop_front();
      preproc(dest, ctx, ls.items);
    }
  }

  static bool ifNone(const UniqNode &src) noexcept {
    auto ls = dynamic_cast<const ListNode *>(src.get());
    return ls && ls->items.empty();
  }

  void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#if..."_token>) {
    preprocIf(dest, ctx, src, ls, not_fn(ifNone));
  }

  void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#ifn..."_token>) {
    preprocIf(dest, ctx, src, ls, ifNone);
  }

  static bool isdef(const Macros &macros, const UniqNode &src);

  void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#ifdef"_token>) {
    preprocIf(dest, ctx, src, ls, [&ctx] (auto &a) { return isdef(ctx.macros, a); });
  }

  void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#ifndef"_token>) {
    preprocIf(dest, ctx, src, ls, [&ctx] (auto &a) { return !isdef(ctx.macros, a); });
  }

  bool isdef(const Macros &macros, const UniqNode &src) {
    auto id = dynamic_cast<const ID *>(src.get());
    if (!id) {
      throw Bad(bad::UNEXPECTED_TOKEN, src->pos);
    }
    return macros.find(id->raw) != macros.end();
  }

  static void includePath(filesystem::path &dest, Context &ctx, const Pos *pos, const UniqNode &src);

  void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#include"_token>) {
    ls.items.pop_front();
    if (ls.items.empty()) [[unlikely]] {
      return;
    }
    filesystem::path path;
    includePath(path, ctx, ls.pos, ls.items.front());
    string s;
    {
      ifstream ifs(path);
      stringstream ss;
      copy(istreambuf_iterator(ifs), istreambuf_iterator<char>(), ostreambuf_iterator(ss));
      s = ss.str();
    }
    UniqNodes inc;
    ParseContext pc(ctx.strs, ctx.posSet);
    auto itFile = ctx.strs.insert(path.string()).first;
    auto itPos = ctx.posSet.insert(Pos(&*itFile, 0, ls.pos)).first;
    parse(inc, pc, &*itPos, s.data(), s.data() + s.size());
    preproc(dest, ctx, inc);
  }

  void includePath(filesystem::path &dest, Context &ctx, const Pos *pos, const UniqNode &src) {
    if (auto rn = dynamic_cast<const RawNode *>(src.get()); rn) {
      dest = filesystem::path(rn->getRaw());
    } else if (auto sl = dynamic_cast<const StringLiteral *>(src.get()); sl) {
      dest = filesystem::path(*sl->value);
    } else {
      throw Bad(bad::UNEXPECTED_TOKEN, src->pos);
    }
    if (dest.is_relative()) {
      dest = filesystem::path(*pos->file) / dest;
    }
    try {
      dest = filesystem::canonical(dest);
    } catch (filesystem::filesystem_error) {
      throw Bad(bad::INV_INCLUDE_FILE, pos);
    }
  }

  void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#movedef"_token>) {
    ls.items.pop_front();
    if (ls.items.empty()) [[unlikely]] {
      return;
    }
    auto from = isID(ls.items.front());
    if (!from) {
      throw Bad(bad::UNEXPECTED_TOKEN, ls.items.front()->pos);
    }
    auto it = ctx.macros.find(from);
    if (it == ctx.macros.end()) {
      return;
    }
    ls.items.pop_front();
    auto to = isID(ls.items.front());
    if (!to) {
      throw Bad(bad::UNEXPECTED_TOKEN, ls.items.front()->pos);
    }
    ctx.macros[to] = std::move(it->second);
    ctx.macros.erase(it);
  }

  void preproc(UniqNodes &dest, Context &ctx, UniqNode &src, ListNode &ls, Constant<"#undef"_token>) {
    ls.items.pop_front();
    if (ls.items.empty()) [[unlikely]] {
      return;
    }
    auto id = isID(ls.items.front());
    if (!id) {
      throw Bad(bad::UNEXPECTED_TOKEN, ls.items.front()->pos);
    }
    ctx.macros.erase(id);
  }
}

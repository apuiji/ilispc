#include<algorithm>
#include<iterator>
#include"nodes.hh"
#include"nodes1.hh"

using namespace std;

namespace zlt::ilispc {
  using Defs = Function::Defs;

  static void transList(Defs &defs, UniqNode &src, ListNode &ls);

  void trans(Defs &defs, UniqNode &src) {
    if (auto n = dynamic_cast<const NumberNode *>(src.get()); n) {
      src.reset(new NumberLiteral(src->pos, n->value));
      return;
    }
    if (Dynamicastable<StringLiteral, ID> {}(src.get())) {
      return;
    }
    if (auto ls = dynamic_cast<ListNode *>(src.get()); ls) {
      transList(defs, src, *ls);
      return;
    }
    int t = dynamic_cast<const TokenNode *>(src.get())->token;
    if (t == "callee"_token) {
      src.reset(new Callee(src->pos));
      return;
    }
    throw Bad(bad::UNEXPECTED_TOKEN, src->pos);
  }

  using It = UniqNodes::iterator;

  static void transCall(Call &dest, Defs &defs, It it, It end);

  using token::Constant;

  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"callee"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"def"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"forward"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"if"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"length"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"return"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"!"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"%"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"&"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"&&"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"*"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"**"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"+"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<","_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"-"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"."_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"/"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"<"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"<<"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"<="_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"<=>"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"="_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"=="_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<">"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<">="_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<">>"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<">>>"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"@"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"^"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"^^"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"|"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"||"_token>);
  static void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"~"_token>);

  template<int T, int ...U>
  inline void trans(Defs &defs, UniqNode &src, ListNode &ls, int t, token::Sequence<T, U...>) {
    if (t == T) {
      trans(defs, src, ls, Constant<T>());
      return;
    }
    if constexpr (sizeof...(U)) {
      trans(defs, src, ls, t, token::Sequence<U...>());
    } else {
      throw Bad(bad::UNEXPECTED_TOKEN, ls.items.front()->pos);
    }
  }

  void transList(Defs &defs, UniqNode &src, ListNode &ls) {
    if (ls.items.empty()) [[unlikely]] {
      src.reset(new Null(src->pos));
      return;
    }
    auto &a = ls.items.front();
    if (Dynamicastable<NumberNode, StringLiteral, ID, ListNode> {}(a.get())) {
      Call call;
      transCall(call, defs, ls.items.begin(), ls.items.end());
      src.reset(new CallNode(src->pos, std::move(call)));
      return;
    }
    auto tn = dynamic_cast<const TokenNode *>(a.get());
    using S = token::Sequence<
      "callee"_token,
      "def"_token,
      "forward"_token,
      "if"_token,
      "length"_token,
      "return"_token,
      "!"_token,
      "%"_token,
      "&"_token,
      "&&"_token,
      "*"_token,
      "**"_token,
      "+"_token,
      ","_token,
      "-"_token,
      "."_token,
      "/"_token,
      "<"_token,
      "<<"_token,
      "<="_token,
      "<=>"_token,
      "="_token,
      "=="_token,
      ">"_token,
      ">="_token,
      ">>"_token,
      ">>>"_token,
      "@"_token,
      "^"_token,
      "^^"_token,
      "|"_token,
      "||"_token,
      "~"_token>;
    trans(defs, src, ls, tn->token, S());
  }

  void transCall(Call &dest, Defs &defs, It it, It end) {
    trans(defs, it, end);
    if (it != end) {
      dest.callee = std::move(*it);
      ++it;
    } else {
      dest.callee.reset(new Null(nullptr));
    }
    move(it, end, back_insert_iterator(dest.args));
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"callee"_token>) {
    Call call;
    transCall(call, defs, ls.items.begin(), ls.items.end());
    src.reset(new CallNode(src->pos, std::move(call)));
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"def"_token>) {
    ls.items.pop_front();
    if (ls.items.empty()) [[unlikely]] {
      throw Bad(bad::ILL_DEF, src->pos);
    }
    auto &a = ls.items.front();
    auto id = dynamic_cast<const ID *>(a.get());
    if (!id) {
      throw Bad(bad::UNEXPECTED_TOKEN, a->pos);
    }
    if (find(defs.begin(), defs.end(), id->raw) == defs.end()) {
      defs.push_back(id->raw);
    }
    src.reset(new ID(src->pos, id->raw));
  }

  static void sequence(UniqNode &dest, It it, It end);

  template<class T>
  inline void transUnary(Defs &defs, UniqNode &src, ListNode &ls) {
    ls.items.pop_front();
    trans(defs, ls.items.begin(), ls.items.end());
    UniqNode value;
    sequence(value, ls.items.begin(), ls.items.end());
    src.reset(new T(src->pos, std::move(value)));
  }

  void sequence(UniqNode &dest, It it, It end) {
    UniqNodes s;
    move(it, end, back_insert_iterator(s));
    if (s.empty()) {
      dest.reset(new Null(nullptr));
    } else if (s.size() == 1) {
      dest = std::move(s.front());
    } else {
      auto pos = s.front()->pos;
      dest.reset(new Sequence(pos, std::move(s)));
    }
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"forward"_token>) {
    ls.items.pop_front();
    Call call;
    transCall(call, defs, ls.items.begin(), ls.items.end());
    src.reset(new Forward(src->pos, std::move(call)));
  }

  static void transIf(UniqNode &dest, Defs &defs, const Pos *pos, It it, It end);

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"if"_token>) {
    ls.items.pop_front();
    transIf(src, defs, src->pos, ls.items.begin(), ls.items.end());
  }

  static bool isIf(const UniqNode &src) noexcept {
    auto tn = dynamic_cast<const TokenNode *>(src.get());
    return tn && tn->token == "if"_token;
  }

  void transIf(UniqNode &dest, Defs &defs, const Pos *pos, It it, It end) {
    if (it == end) [[unlikely]] {
      dest.reset(new Null(pos));
      return;
    }
    auto cond = std::move(*it);
    trans(defs, cond);
    auto it1 = find_if(++it, end, isIf);
    trans(defs, it, it1);
    UniqNode then;
    sequence(then, it, it1);
    UniqNode elze;
    if (it1 == end) {
      elze.reset(new Null(nullptr));
    } else {
      auto pos1 = (**it1).pos;
      transIf(elze, defs, pos1, ++it1, end);
    }
    dest.reset(new If(pos, std::move(cond), std::move(then), std::move(elze)));
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"length"_token>) {
    transUnary<LengthOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"return"_token>) {
    transUnary<Return>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"!"_token>) {
    transUnary<LogicNotOper>(defs, src, ls);
  }

  template<class T>
  inline void transXnary(Defs &defs, UniqNode &src, ListNode &ls) {
    ls.items.pop_front();
    trans(defs, ls.items.begin(), ls.items.end());
    src.reset(new T(src->pos, std::move(ls.items)));
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"%"_token>) {
    transXnary<ArithModOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"&"_token>) {
    transXnary<BitwsAndOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"&&"_token>) {
    transXnary<LogicAndOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"*"_token>) {
    transXnary<ArithMulOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"**"_token>) {
    transXnary<ArithPowOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"+"_token>) {
    if (ls.items.size() == 1) [[unlikely]] {
      auto &a = ls.items.front();
      trans(defs, a);
      src.reset(new PositiveOper(src->pos, std::move(a)));
      return;
    }
    transXnary<ArithAddOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<","_token>) {
    ls.items.pop_front();
    trans(defs, ls.items.begin(), ls.items.end());
    sequence(src, ls.items.begin(), ls.items.end());
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"-"_token>) {
    if (ls.items.size() == 1) [[unlikely]] {
      auto &a = ls.items.front();
      trans(defs, a);
      src.reset(new NegativeOper(src->pos, std::move(a)));
      return;
    }
    transXnary<ArithSubOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"."_token>) {
    trans(defs, ls.items.begin(), ls.items.end());
    if (ls.items.empty()) {
      src.reset(new Null(src->pos));
    } else if (ls.items.size() == 1) {
      src = std::move(ls.items.front());
    } else {
      src.reset(new GetMembOper(src->pos, std::move(ls.items)));
    }
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"/"_token>) {
    transXnary<ArithDivOper>(defs, src, ls);
  }

  static void transBinary1(array<UniqNode, 2> &dest, Defs &defs, ListNode &ls);

  template<class T>
  inline void transBinary(Defs &defs, UniqNode &src, ListNode &ls) {
    array<UniqNode, 2> items;
    transBinary1(items, defs, ls);
    src.reset(new T(src->pos, std::move(items)));
  }

  void transBinary1(array<UniqNode, 2> &dest, Defs &defs, ListNode &ls) {
    if (ls.items.size()) {
      auto &a = ls.items.front();
      trans(defs, a);
      dest[0] = std::move(a);
      ls.items.pop_front();
    } else {
      dest[0].reset(new Null(nullptr));
    }
    trans(defs, ls.items.begin(), ls.items.end());
    sequence(dest[1], ls.items.begin(), ls.items.end());
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"<"_token>) {
    transBinary<CmpLtOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"<<"_token>) {
    transXnary<LshOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"<="_token>) {
    transBinary<CmpLteqOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"<=>"_token>) {
    transBinary<Cmp3wayOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"="_token>) {
    array<UniqNode, 2> items;
    transBinary1(items, defs, ls);
    auto &a = items[0];
    if (dynamic_cast<const ID *>(a.get())) {
      src.reset(new AssignOper(src->pos, std::move(items)));
    } else if (auto b = dynamic_cast<GetMembOper *>(a.get()); b) {
      array<UniqNode, 3> items1;
      items1[1] = std::move(b->items.back());
      items1[0] = std::move(a);
      items1[2] = std::move(items[1]);
      src.reset(new SetMembOper(src->pos, std::move(items1)));
    } else {
      throw Bad(bad::INV_LHS_EXPR, a->pos);
    }
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"=="_token>) {
    transBinary<CmpEqOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<">"_token>) {
    transBinary<CmpGtOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<">="_token>) {
    transBinary<CmpGteqOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<">>"_token>) {
    transXnary<RshOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<">>>"_token>) {
    transXnary<UshOper>(defs, src, ls);
  }

  static void makeFnParams(Defs &defs, size_t &paramc, UniqNode &src);

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"@"_token>) {
    ls.items.pop_front();
    Defs defs1;
    size_t paramc;
    if (ls.items.size()) {
      makeFnParams(defs1, paramc, ls.items.front());
      ls.items.pop_front();
    } else {
      paramc = 0;
    }
    trans(defs1, ls.items.begin(), ls.items.end());
    defs1.shrink_to_fit();
    UniqNode a;
    a.reset(new Null(nullptr));
    a.reset(new Return(nullptr, std::move(a)));
    ls.items.push_back(std::move(a));
    src.reset(new Function(src->pos, std::move(defs1), paramc, std::move(ls.items)));
  }

  static void makeFnParams(Defs &defs, It it, It end);

  void makeFnParams(Defs &defs, size_t &paramc, UniqNode &src) {
    if (auto id = dynamic_cast<const ID *>(src.get()); id) [[unlikely]] {
      defs.push_back(id->raw);
      paramc = 0;
    } else if (auto ls = dynamic_cast<ListNode *>(src.get()); ls) {
      makeFnParams(defs, ls->items.begin(), ls->items.end());
      paramc = ls->items.size();
    } else {
      throw Bad(bad::UNEXPECTED_TOKEN, src->pos);
    }
  }

  static void clearDupParams(Defs::iterator it, Defs::iterator end, const string *name) noexcept;

  void makeFnParams(Defs &defs, It it, It end) {
    for (; it != end; ++it) {
      if (auto id = dynamic_cast<const ID *>(it->get()); id) {
        clearDupParams(defs.begin(), defs.end(), id->raw);
        defs.push_back(id->raw);
      } else if (auto ls = dynamic_cast<const ListNode *>(it->get()); ls && ls->items.empty()) {
        defs.push_back(nullptr);
      } else {
        throw Bad(bad::UNEXPECTED_TOKEN, (**it).pos);
      }
    }
  }

  void clearDupParams(Defs::iterator it, Defs::iterator end, const string *name) noexcept {
    for (; it != end; ++it) {
      if (*it == name) {
        *it = nullptr;
      }
    }
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"^"_token>) {
    transXnary<BitwsXorOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"^^"_token>) {
    transXnary<LogicXorOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"|"_token>) {
    transXnary<BitwsOrOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"||"_token>) {
    transXnary<LogicOrOper>(defs, src, ls);
  }

  void trans(Defs &defs, UniqNode &src, ListNode &ls, Constant<"~"_token>) {
    transUnary<BitwsNotOper>(defs, src, ls);
  }
}

#include<cmath>
#include"nodes.hh"
#include"nodes1.hh"

using namespace std;

namespace zlt::ilispc {
  static void optimize(bool global, UniqNode &src);
  static bool isTerminated(const UniqNode &src) noexcept;

  void optimize(UniqNodes &dest, bool global, UniqNodes &src) {
    for (; src.size(); src.pop_front()) {
      optimize(global, src.front());
      dest.push_back(std::move(src.front()));
      if (isTerminated(dest.back())) {
        break;
      }
    }
  }

  bool isTerminated(const UniqNode &src) noexcept {
    if (Dynamicastable<Forward, Return, Throw> {}(src.get())) {
      return true;
    }
    if (auto i = dynamic_cast<const If *>(src.get()); i) {
      return isTerminated(i->then) && isTerminated(i->elze);
    }
    if (auto s = dynamic_cast<const Sequence *>(src.get()); s) {
      return isTerminated(s->items.back());
    }
    return false;
  }

  static void optimizeCall(bool global, Call &call);
  static bool isConstBool(bool &dest, const UniqNode &src) noexcept;
  static void optimizeOper1(UniqNode &src);
  static void optimizeOper2(UniqNode &src);
  static void optimizeOperX(UniqNode &src);

  void optimize(bool global, UniqNode &src) {
    if (dynamic_cast<const Callee *>(src.get())) {
      if (global) {
        src.reset(new Null(src->pos));
      }
    } else if (auto c = dynamic_cast<CallNode *>(src.get()); c) {
      optimizeCall(global, *c);
    } else if (auto d = dynamic_cast<Defer *>(src.get()); d) {
      optimize(global, d->value);
    } else if (auto f = dynamic_cast<Forward *>(src.get()); f) {
      optimizeCall(global, *f);
    } else if (auto f = dynamic_cast<Function *>(src.get()); f) {
      UniqNodes body;
      optimize(body, false, f->body);
      f->body = std::move(body);
    } else if (auto g = dynamic_cast<Guard *>(src.get()); g) {
      optimize(global, g->value);
    } else if (auto i = dynamic_cast<If *>(src.get()); i) {
      optimize(global, i->cond);
      optimize(global, i->then);
      optimize(global, i->elze);
      if (bool b; isConstBool(b, i->cond)) {
        src = b ? std::move(i->then) : std::move(i->elze);
      }
    } else if (auto r = dynamic_cast<Return *>(src.get()); r) {
      optimize(global, r->value);
    } else if (auto t = dynamic_cast<Throw *>(src.get()); t) {
      optimize(global, t->value);
    } else if (auto t = dynamic_cast<Try *>(src.get()); t) {
      optimizeCall(global, *t);
    } else if (auto s = dynamic_cast<Sequence *>(src.get()); s) {
      UniqNodes items;
      optimize(items, global, s->items);
      s->items = std::move(items);
    } else if (auto o = dynamic_cast<Operation<1> *>(src.get()); o) {
      optimize(global, o->item);
      optimizeOper1(src);
    } else if (auto o = dynamic_cast<Operation<2> *>(src.get()); o) {
      optimize(global, o->items[0]);
      optimize(global, o->items[1]);
      optimizeOper2(src);
    } else if (auto o = dynamic_cast<Operation<3> *>(src.get()); o) {
      optimize(global, o->items[0]);
      optimize(global, o->items[1]);
      optimize(global, o->items[2]);
    } else if (auto o = dynamic_cast<Operation<-1> *>(src.get()); o) {
      for (auto &item : o->items) {
        optimize(global, item);
      }
      optimizeOperX(src);
    }
  }

  void optimizeCall(bool global, Call &call) {
    optimize(global, call.callee);
    for (auto &arg : call.args) {
      optimize(global, arg);
    }
  }

  bool isConstBool(bool &dest, const UniqNode &src) noexcept {
    if (Dynamicastable<NumberLiteral, StringLiteral, Callee, Function> {}(src.get())) {
      dest = true;
      return true;
    }
    if (dynamic_cast<Null *>(src.get())) {
      dest = false;
      return true;
    }
    return false;
  }

  static void optimizeOper(UniqNode &dest, LogicNotOper &src);
  static void optimizeOper(UniqNode &dest, BitwsNotOper &src);
  static void optimizeOper(UniqNode &dest, PositiveOper &src);
  static void optimizeOper(UniqNode &dest, NegativeOper &src);
  static void optimizeOper(UniqNode &dest, LengthOper &src);

  template<class T, class ...U>
  inline void optimizeOper(UniqNode &src, Types<T, U...>) {
    if (auto a = dynamic_cast<T *>(src.get()); a) {
      optimizeOper(src, *a);
      return;
    }
    if constexpr (sizeof...(U)) {
      optimizeOper(src, Types<U...>());
    }
  }

  void optimizeOper1(UniqNode &src) {
    using S = Types<LogicNotOper, BitwsNotOper, PositiveOper, NegativeOper, LengthOper>;
    optimizeOper(src, S());
  }

  static void makeBool(UniqNode &dest, const Pos *pos, bool b) {
    if (b) {
      dest.reset(new NumberLiteral(pos, 1));
    } else {
      dest.reset(new Null(pos));
    }
  }

  void optimizeOper(UniqNode &dest, LogicNotOper &src) {
    bool b;
    if (isConstBool(b, src.item)) {
      makeBool(dest, src.pos, !b);
    }
  }

  static bool isConstNum(double &dest, const UniqNode &src) noexcept;

  void optimizeOper(UniqNode &dest, BitwsNotOper &src) {
    double d;
    if (isConstNum(d, src.item)) {
      dest.reset(new NumberLiteral(src.pos, ~(int) d));
    }
  }

  bool isConstNum(double &dest, const UniqNode &src) noexcept {
    if (dynamic_cast<const NumberLiteral *>(src.get())) {
      return true;
    }
    if (Dynamicastable<StringLiteral, Callee, Function, Null> {}(src.get())) {
      dest = NAN;
      return true;
    }
    return false;
  }

  void optimizeOper(UniqNode &dest, PositiveOper &src) {
    double d;
    if (isConstNum(d, src.item)) {
      dest.reset(new NumberLiteral(src.pos, d));
    }
  }

  void optimizeOper(UniqNode &dest, NegativeOper &src) {
    double d;
    if (isConstNum(d, src.item)) {
      dest.reset(new NumberLiteral(src.pos, -d));
    }
  }

  void optimizeOper(UniqNode &dest, LengthOper &src) {
    auto a = dynamic_cast<const StringLiteral *>(src.item.get());
    if (a) {
      dest.reset(new NumberLiteral(src.pos, a->value->size()));
    }
  }

  static void optimizeOper(UniqNode &dest, CmpEqOper &src);
  static void optimizeOper(UniqNode &dest, CmpLtOper &src);
  static void optimizeOper(UniqNode &dest, CmpGtOper &src);
  static void optimizeOper(UniqNode &dest, CmpLteqOper &src);
  static void optimizeOper(UniqNode &dest, CmpGteqOper &src);
  static void optimizeOper(UniqNode &dest, Cmp3wayOper &src);

  void optimizeOper2(UniqNode &src) {
    using S = Types<CmpEqOper, CmpLtOper, CmpGtOper, CmpLteqOper, CmpGteqOper, Cmp3wayOper>;
    optimizeOper(src, S());
  }

  static void constCmp(int &dest, const NumberLiteral &a, const NumberLiteral &b) noexcept;
  static void constCmp(int &dest, const StringLiteral &a, const StringLiteral &b) noexcept;

  static inline void constCmp(int &dest, const Callee &a, const Callee &b) noexcept {
    dest = 0b010;
  }

  static inline void constCmp(int &dest, const Function &a, const Function &b) noexcept {
    dest = 0;
  }

  static inline void constCmp(int &dest, const Null &a, const Null &b) noexcept {
    dest = 0b010;
  }

  template<class T, class ...U>
  inline void constCmp(int &dest, const UniqNode &a, const UniqNode &b, Types<T, U...>) noexcept {
    if constexpr (sizeof...(U)) {
      if (auto x = dynamic_cast<const T *>(a.get()); x) {
        if (auto y = dynamic_cast<const T *>(b.get()); y) {
          constCmp(dest, *x, *y);
        } else {
          dest = 0;
        }
      }
    } else {
      constCmp(dest, dynamic_cast<const T &>(*a), dynamic_cast<const T &>(*b));
    }
  }

  static bool isConstCmp(int &dest, const UniqNode &a, const UniqNode &b) noexcept {
    Dynamicastable<NumberLiteral, StringLiteral, Callee, Function, Null> d;
    if (d(a.get()) && d(b.get())) {
      constCmp(dest, a, b, Types<NumberLiteral, StringLiteral, Callee, Function, Null>());
      return true;
    } else {
      return false;
    }
  }

  void constCmp(int &dest, const NumberLiteral &a, const NumberLiteral &b) noexcept {
    dest = a.value < b.value ? 0b100 : a.value > b.value ? 0b001 : a.value == b.value ? 0b010 : 0;
  }

  void constCmp(int &dest, const StringLiteral &a, const StringLiteral &b) noexcept {
    int diff = a.value->compare(*b.value);
    dest = diff < 0 ? 0b100 : diff > 0 ? 0b001 : 0b010;
  }

  void optimizeOper(UniqNode &dest, CmpEqOper &src) {
    int diff;
    if (isConstCmp(diff, src.items[0], src.items[1])) {
      makeBool(dest, src.pos, diff == 0b010);
    }
  }

  void optimizeOper(UniqNode &dest, CmpLtOper &src) {
    int diff;
    if (isConstCmp(diff, src.items[0], src.items[1])) {
      makeBool(dest, src.pos, diff == 0b100);
    }
  }

  void optimizeOper(UniqNode &dest, CmpGtOper &src) {
    int diff;
    if (isConstCmp(diff, src.items[0], src.items[1])) {
      makeBool(dest, src.pos, diff == 0b001);
    }
  }

  void optimizeOper(UniqNode &dest, CmpLteqOper &src) {
    int diff;
    if (isConstCmp(diff, src.items[0], src.items[1])) {
      makeBool(dest, src.pos, diff & 0b110);
    }
  }

  void optimizeOper(UniqNode &dest, CmpGteqOper &src) {
    int diff;
    if (isConstCmp(diff, src.items[0], src.items[1])) {
      makeBool(dest, src.pos, diff & 0b011);
    }
  }

  void optimizeOper(UniqNode &dest, Cmp3wayOper &src) {
    int diff;
    if (isConstCmp(diff, src.items[0], src.items[1])) {
      dest.reset(new NumberLiteral(src.pos, diff == 0b100 ? -1 : diff == 0b001 ? 1 : diff == 0b010 ? 0 : NAN));
    }
  }

  // arithmetical operations begin
  static void optimizeOper(UniqNode &dest, ArithAddOper &src);
  static void optimizeOper(UniqNode &dest, ArithSubOper &src);
  static void optimizeOper(UniqNode &dest, ArithMulOper &src);
  static void optimizeOper(UniqNode &dest, ArithDivOper &src);
  // arithmetical operations end
  // logical operations begin
  static void optimizeOper(UniqNode &dest, LogicAndOper &src);
  static void optimizeOper(UniqNode &dest, LogicOrOper &src);
  static void optimizeOper(UniqNode &dest, LogicXorOper &src);
  // logical operations end
  // bitwise operations begin
  static void optimizeOper(UniqNode &dest, BitwsAndOper &src);
  static void optimizeOper(UniqNode &dest, BitwsOrOper &src);
  static void optimizeOper(UniqNode &dest, BitwsXorOper &src);
  static void optimizeOper(UniqNode &dest, LshOper &src);
  static void optimizeOper(UniqNode &dest, RshOper &src);
  static void optimizeOper(UniqNode &dest, UshOper &src);
  // bitwise operations end

  void optimizeOperX(UniqNode &src) {
    using S = Types<
      // arithmetical operations begin
      ArithAddOper,
      ArithSubOper,
      ArithMulOper,
      ArithDivOper,
      // arithmetical operations begin
      // logical operations begin
      LogicAndOper,
      LogicOrOper,
      LogicXorOper,
      // logical operations end
      // bitwise operations begin
      BitwsAndOper,
      BitwsOrOper,
      BitwsXorOper,
      LshOper,
      RshOper,
      UshOper
      // bitwise operations end
      >;
    optimizeOper(src, S());
  }

  // arithmetical operations begin
  using It = UniqNodes::iterator;

  inline void arith(double &dest, UniqNodes &src, It it, It end, auto f) {
    while (it != end) {
      if (double d; isConstNum(d, *it)) {
        dest = f(dest, d);
        it = src.erase(it);
      } else {
        ++it;
      }
    }
  }

  void optimizeOper(UniqNode &dest, ArithAddOper &src) {
    double d = 0;
    size_t n = src.items.size();
    arith(d, src.items, src.items.begin(), src.items.end(), plus<double>());
    if (src.items.empty()) {
      dest.reset(new NumberLiteral(src.pos, d));
      return;
    }
    if (src.items.size() != n) {
      UniqNode a(new NumberLiteral(nullptr, d));
      src.items.push_front(std::move(a));
    }
  }

  void optimizeOper(UniqNode &dest, ArithSubOper &src) {
    double d;
    if (isConstNum(d, src.items.front())) {
      size_t n = src.items.size();
      arith(d, src.items, next(src.items.begin()), src.items.end(), minus<double>());
      if (src.items.size() == 1) {
        dest.reset(new NumberLiteral(src.pos, d));
      } else {
        src.items.front().reset(new NumberLiteral(nullptr, d));
      }
    } else {
      d = 0;
      size_t n = src.items.size();
      arith(d, src.items, next(src.items.begin()), src.items.end(), plus<double>());
      UniqNode a(new NumberLiteral(nullptr, d));
      src.items.push_back(std::move(a));
    }
  }

  void optimizeOper(UniqNode &dest, ArithMulOper &src) {
    double d = 1;
    size_t n = src.items.size();
    arith(d, src.items, src.items.begin(), src.items.end(), multiplies<double>());
    if (src.items.empty()) {
      dest.reset(new NumberLiteral(src.pos, d));
      return;
    }
    if (src.items.size() != n) {
      UniqNode a(new NumberLiteral(nullptr, d));
      src.items.push_front(std::move(a));
    }
  }

  void optimizeOper(UniqNode &dest, ArithDivOper &src) {
    double d;
    if (isConstNum(d, src.items.front())) {
      size_t n = src.items.size();
      arith(d, src.items, next(src.items.begin()), src.items.end(), divides<double>());
      if (src.items.size() == 1) {
        dest.reset(new NumberLiteral(src.pos, d));
      } else {
        src.items.front().reset(new NumberLiteral(nullptr, d));
      }
    } else {
      d = 0;
      size_t n = src.items.size();
      arith(d, src.items, next(src.items.begin()), src.items.end(), multiplies<double>());
      UniqNode a(new NumberLiteral(nullptr, d));
      src.items.push_back(std::move(a));
    }
  }
  // arithmetical operations end

  // logical operations begin
  static void logicAnd(UniqNodes &dest, UniqNodes &src);

  void optimizeOper(UniqNode &dest, LogicAndOper &src) {
    UniqNodes items;
    logicAnd(items, src.items);
    if (items.size() == 1) {
      dest = std::move(items.front());
    } else {
      src.items = std::move(items);
    }
  }

  void logicAnd(UniqNodes &dest, UniqNodes &src) {
    for (; src.size() > 1; src.pop_front()) {
      bool b;
      if (!isConstBool(b, src.front())) {
        dest.push_back(std::move(src.front()));
        continue;
      }
      if (!b) {
        return;
      }
    }
    dest.push_back(std::move(src.front()));
  }

  static void logicOr(UniqNodes &dest, UniqNodes &src);

  void optimizeOper(UniqNode &dest, LogicOrOper &src) {
    UniqNodes items;
    logicOr(items, src.items);
    if (items.size() == 1) {
      dest = std::move(items.front());
    } else {
      src.items = std::move(items);
    }
  }

  void logicOr(UniqNodes &dest, UniqNodes &src) {
    for (; src.size() > 1; src.pop_front()) {
      bool b;
      if (!isConstBool(b, src.front())) {
        dest.push_back(std::move(src.front()));
        continue;
      }
      if (b) {
        return;
      }
    }
    dest.push_back(std::move(src.front()));
  }

  static void logicXor(bool &dest, UniqNodes &src, It it, It end);

  void optimizeOper(UniqNode &dest, LogicXorOper &src) {
    bool b = false;
    size_t n = src.items.size();
    logicXor(b, src.items, src.items.begin(), src.items.end());
    if (src.items.empty()) {
      makeBool(dest, src.pos, b);
      return;
    }
    if (src.items.size() != n) {
      UniqNode a;
      makeBool(a, nullptr, b);
      src.items.push_front(std::move(a));
    }
  }

  void logicXor(bool &dest, UniqNodes &src, It it, It end) {
    while (it != end) {
      if (bool b; isConstBool(b, *it)) {
        dest = dest ^ b;
        it = src.erase(it);
      } else {
        ++it;
      }
    }
  }
  // logical operations end

  // bitwise operations begin
  static inline bool isConstInt(int &dest, const UniqNode &src) noexcept {
    double d;
    if (!isConstNum(d, src)) {
      return false;
    }
    dest = (int) d;
    return true;
  }

  inline void bitwise(int &dest, UniqNodes &src, It it, It end, auto f) {
    while (it != end) {
      if (int i; isConstInt(i, *it)) {
        dest = f(dest, i);
        it = src.erase(it);
      } else {
        ++it;
      }
    }
  }

  void optimizeOper(UniqNode &dest, BitwsAndOper &src) {
    int i = -1;
    size_t n = src.items.size();
    bitwise(i, src.items, src.items.begin(), src.items.end(), [] (int i, int j) { return i & j; });
    if (src.items.empty()) {
      dest.reset(new NumberLiteral(src.pos, i));
      return;
    }
    if (src.items.size() != n) {
      UniqNode a(new NumberLiteral(nullptr, i));
      src.items.push_front(std::move(a));
    }
  }

  void optimizeOper(UniqNode &dest, BitwsOrOper &src) {
    int i = 0;
    size_t n = src.items.size();
    bitwise(i, src.items, src.items.begin(), src.items.end(), [] (int i, int j) { return i | j; });
    if (src.items.empty()) {
      dest.reset(new NumberLiteral(src.pos, i));
      return;
    }
    if (src.items.size() != n) {
      UniqNode a(new NumberLiteral(nullptr, i));
      src.items.push_front(std::move(a));
    }
  }

  void optimizeOper(UniqNode &dest, BitwsXorOper &src) {
    int i = 0;
    size_t n = src.items.size();
    bitwise(i, src.items, src.items.begin(), src.items.end(), [] (int i, int j) { return i ^ j; });
    if (src.items.empty()) {
      dest.reset(new NumberLiteral(src.pos, i));
      return;
    }
    if (src.items.size() != n) {
      UniqNode a(new NumberLiteral(nullptr, i));
      src.items.push_front(std::move(a));
    }
  }

  inline void shift(UniqNode &dest, const Pos *pos, UniqNodes &items, auto f) {
    int i;
    if (isConstInt(i, items.front())) {
      size_t n = items.size();
      bitwise(i, items, next(items.begin()), items.end(), f);
      if (items.size() == 1) {
        dest.reset(new NumberLiteral(pos, i));
      } else {
        items.front().reset(new NumberLiteral(nullptr, i));
      }
    } else {
      i = 0;
      size_t n = items.size();
      bitwise(i, items, next(items.begin()), items.end(), f);
      UniqNode a(new NumberLiteral(nullptr, i));
      items.push_back(std::move(a));
    }
  }

  void optimizeOper(UniqNode &dest, LshOper &src) {
    shift(dest, src.pos, src.items, [] (int i, int j) { return i << j; });
  }

  void optimizeOper(UniqNode &dest, RshOper &src) {
    shift(dest, src.pos, src.items, [] (int i, int j) { return i >> j; });
  }

  void optimizeOper(UniqNode &dest, UshOper &src) {
    shift(dest, src.pos, src.items, [] (int i, int j) { return (unsigned) i >> j; });
  }
  // bitwise operations end
}

#include"nodes3.hh"

using namespace std;

namespace zlt::ilispc {
  using Defs = Function2::Defs;

  struct Scope {
    Defs &defs;
    bool hasGuard;
    Scope(Defs &defs) noexcept: defs(defs) {}
  };

  static void trans2(Scope &scope, UniqNode &src);

  using It = UniqNodes::iterator;

  static inline void trans2(Scope &scope, It it, It end) {
    for (; it != end; ++it) {
      trans2(scope, *it);
    }
  }

  void trans2(UniqNode &dest, UniqNodes &src) {
    Defs defs;
    Scope scope(defs);
    trans2(scope, src.begin(), src.end());
    dest.reset(new Function2(nullptr, {}, {}, 0, std::move(src), scope.hasGuard));
  }

  static void transCall(Scope &scope, Call &src);

  static void makeRef2s(UniqNodes &dest, Function1::Defs &defs);
  static void trans2(UniqNode &dest, Scope &scope, AssignOper &src);
  static void trans2(UniqNode &dest, Scope &scope, ReferenceNode &src);

  void trans2(Scope &scope, UniqNode &src) {
    if (auto a = dynamic_cast<CallNode *>(src.get()); a) {
      transCall(scope, *a);
    } else if (auto a = dynamic_cast<Forward *>(src.get()); a) {
      transCall(scope, *a);
    } else if (auto a = dynamic_cast<If *>(src.get()); a) {
      trans2(scope, a->cond);
      trans2(scope, a->then);
      trans2(scope, a->elze);
    } else if (auto a = dynamic_cast<Return *>(src.get()); a) {
      trans2(scope, a->value);
    } else if (auto a = dynamic_cast<AssignOper *>(src.get()); a) {
      trans2(src, scope, *a);
    } else if (auto a = dynamic_cast<Operation<1> *>(src.get()); a) {
      trans2(scope, a->item);
    } else if (auto a = dynamic_cast<Operation<2> *>(src.get()); a) {
      trans2(scope, a->items[0]);
      trans2(scope, a->items[1]);
    } else if (auto a = dynamic_cast<Operation<3> *>(src.get()); a) {
      trans2(scope, a->items[0]);
      trans2(scope, a->items[1]);
      trans2(scope, a->items[2]);
    } else if (auto a = dynamic_cast<Operation<-1> *>(src.get()); a) {
      trans2(scope, a->items.begin(), a->items.end());
    } else if (auto a = dynamic_cast<Function1 *>(src.get()); a) {
      Scope scope1(a->defs);
      trans2(scope1, a->body.begin(), a->body.end());
      bool hasGuard = scope1.hasGuard;
      auto f = new Function2(src->pos, std::move(a->defs), std::move(a->closureDefs), a->paramc, std::move(a->body), hasGuard);
      src.reset(f);
    } else if (auto a = dynamic_cast<ReferenceNode *>(src.get()); a) {
      trans2(src, scope, *a);
    }
  }

  void transCall(Scope &scope, Call &src) {
    trans2(scope, src.callee);
    trans2(scope, src.args.begin(), src.args.end());
  }

  static bool isClosureRef(Defs &defs, const Reference &ref) noexcept;

  void trans2(UniqNode &dest, Scope &scope, AssignOper &src) {
    auto &ref = static_cast<const ReferenceNode &>(*src.items[0]);
    trans2(scope, src.items[1]);
    if (isClosureRef(scope.defs, ref)) {
      dest.reset(new SetRef2Oper(src.pos, std::move(src.items)));
    }
  }

  bool isClosureRef(Defs &defs, const Reference &ref) noexcept {
    if (ref.scope == Reference::GLOBAL_SCOPE) {
      return false;
    }
    if (ref.scope == Reference::CLOSURE_SCOPE) {
      return true;
    }
    auto it = find_if(defs.begin(), defs.end(), [&ref] (auto &def) { return def.name == ref.name; });
    return it->closure;
  }

  void trans2(UniqNode &dest, Scope &scope, ReferenceNode &src) {
    if (isClosureRef(scope.defs, src)) {
      dest.reset(new GetRef2Oper(src.pos, std::move(dest)));
    }
  }
}

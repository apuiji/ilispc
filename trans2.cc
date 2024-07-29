#include"nodes3.hh"

using namespace std;

namespace zlt::ilispc {
  struct Scope {
    Function1::Defs &defs;
    bool hasGuard;
    Scope(Function1::Defs &defs) noexcept: defs(defs) {}
  };

  static void trans2(Scope &scope, UniqNode &src);

  using It = UniqNodes::iterator;

  static inline void trans2(Scope &scope, It it, It end) {
    for (; it != end; ++it) {
      trans2(scope, *it);
    }
  }

  void trans2(UniqNode &dest, UniqNodes &src) {
    Function1::Defs defs;
    Scope scope(defs);
    trans2(scope, src.begin(), src.end());
    dest.reset(new Function2(nullptr, {}, {}, 0, std::move(src), scope.hasGuard));
  }

  static void transCall(Scope &scope, Call &src);

  static inline void copyDefs(
    Function1::Defs::iterator it, Function1::Defs::iterator end, Function2::Defs::iterator outIt) noexcept {
    for (; it != end; ++it) {
      *outIt = it->name;
    }
  }

  static void makeRef2s(UniqNodes &dest, Function1::Defs &defs);
  static void trans2(UniqNode &dest, Scope &scope, AssignOper &src);
  static void trans2(UniqNode &dest, Scope &scope, ReferenceNode &src);

  void trans2(Scope &scope, UniqNode &src) {
    if (auto a = dynamic_cast<CallNode *>(src.get()); a) {
      transCall(scope, *a);
    } else if (auto a = dynamic_cast<Defer *>(src.get()); a) {
      trans2(scope, a->value);
    } else if (auto a = dynamic_cast<Forward *>(src.get()); a) {
      transCall(scope, *a);
    } else if (auto a = dynamic_cast<Guard *>(src.get()); a) {
      scope.hasGuard = true;
      trans2(scope, a->value);
    } else if (auto a = dynamic_cast<If *>(src.get()); a) {
      trans2(scope, a->cond);
      trans2(scope, a->then);
      trans2(scope, a->elze);
    } else if (auto a = dynamic_cast<Return *>(src.get()); a) {
      trans2(scope, a->value);
    } else if (auto a = dynamic_cast<Throw *>(src.get()); a) {
      trans2(scope, a->value);
    } else if (auto a = dynamic_cast<Try *>(src.get()); a) {
      transCall(scope, *a);
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
      Function2::Defs defs1(a->defs.size());
      copyDefs(a->defs.begin(), a->defs.end(), defs1.begin());
      UniqNodes body;
      makeRef2s(body, a->defs);
      Scope scope1(a->defs);
      trans2(scope1, a->body.begin(), a->body.end());
      move(a->body.begin(), a->body.end(), back_insert_iterator(body));
      src.reset(
        new Function2(src->pos, std::move(defs1), std::move(a->closureDefs), a->paramc, std::move(body), scope1.hasGuard));
    } else if (auto a = dynamic_cast<ReferenceNode *>(src.get()); a) {
      trans2(src, scope, *a);
    }
  }

  void transCall(Scope &scope, Call &src) {
    trans2(scope, src.callee);
    trans2(scope, src.args.begin(), src.args.end());
  }

  void makeRef2s(UniqNodes &dest, Function1::Defs &defs) {
    for (int i = 0; i < defs.size(); ++i) {
      if (defs[i].closure) {
        Reference ref(Reference::LOCAL_SCOPE, i, defs[i].name);
        UniqNode a;
        a.reset(new ReferenceNode(nullptr, ref));
        a.reset(new MakeRef2(nullptr, std::move(a)));
        dest.push_back(std::move(a));
      }
    }
  }

  static bool isClosureRef(Function1::Defs &defs, const Reference &ref) noexcept;

  void trans2(UniqNode &dest, Scope &scope, AssignOper &src) {
    trans2(scope, src.items[1]);
    if (auto a = dynamic_cast<const ReferenceNode *>(src.items[0].get()); a && isClosureRef(scope.defs, *a)) {
      dest.reset(new SetRef2Oper(src.pos, std::move(src.items)));
    } else {
      trans2(scope, src.items[0]);
    }
  }

  bool isClosureRef(Function1::Defs &defs, const Reference &ref) noexcept {
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

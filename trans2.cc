#include"nodes3.hh"

using namespace std;

namespace zlt::ilispc {
  using Defs = Function1::Defs;

  static void trans2(Defs *defs, UniqNode &src);

  using It = UniqNodes::iterator;

  static inline void trans2(Defs *defs, It it, It end) {
    for (; it != end; ++it) {
      trans2(defs, *it);
    }
  }

  void trans2(UniqNodes &src) {
    trans2(nullptr, src.begin(), src.end());
  }

  static void transCall(Defs *defs, Call &src);

  static inline void copyDefs(Defs::iterator it, Defs::iterator end, Function2::Defs::iterator outIt) noexcept {
    for (; it != end; ++it) {
      *outIt = it->name;
    }
  }

  static void makeRef2s(UniqNodes &dest, Defs &defs, size_t paramc);
  static void makeRef2s1(UniqNodes &dest, Defs &defs, int i, size_t defc);
  static void trans2(UniqNode &dest, Defs *defs, AssignOper &src);
  static void trans2(UniqNode &dest, Defs *defs, ReferenceNode &src);

  void trans2(Defs *defs, UniqNode &src) {
    if (auto a = dynamic_cast<CallNode *>(src.get()); a) {
      transCall(defs, *a);
    } else if (auto a = dynamic_cast<Defer *>(src.get()); a) {
      trans2(defs, a->value);
    } else if (auto a = dynamic_cast<Forward *>(src.get()); a) {
      transCall(defs, *a);
    } else if (auto a = dynamic_cast<Guard *>(src.get()); a) {
      trans2(defs, a->value);
    } else if (auto a = dynamic_cast<If *>(src.get()); a) {
      trans2(defs, a->cond);
      trans2(defs, a->then);
      trans2(defs, a->elze);
    } else if (auto a = dynamic_cast<Return *>(src.get()); a) {
      trans2(defs, a->value);
    } else if (auto a = dynamic_cast<Throw *>(src.get()); a) {
      trans2(defs, a->value);
    } else if (auto a = dynamic_cast<Try *>(src.get()); a) {
      transCall(defs, *a);
    } else if (auto a = dynamic_cast<AssignOper *>(src.get()); a) {
      trans2(src, defs, *a);
    } else if (auto a = dynamic_cast<Operation<1> *>(src.get()); a) {
      trans2(defs, a->item);
    } else if (auto a = dynamic_cast<Operation<2> *>(src.get()); a) {
      trans2(defs, a->items[0]);
      trans2(defs, a->items[1]);
    } else if (auto a = dynamic_cast<Operation<3> *>(src.get()); a) {
      trans2(defs, a->items[0]);
      trans2(defs, a->items[1]);
      trans2(defs, a->items[2]);
    } else if (auto a = dynamic_cast<Operation<-1> *>(src.get()); a) {
      trans2(defs, a->items.begin(), a->items.end());
    } else if (auto a = dynamic_cast<Function1 *>(src.get()); a) {
      Function2::Defs defs1(a->defs.size());
      copyDefs(a->defs.begin(), a->defs.end(), defs1.begin());
      UniqNodes body;
      makeRef2s(body, a->defs, a->paramc);
      makeRef2s1(body, a->defs, a->paramc, a->defs.size());
      trans2(&a->defs, a->body.begin(), a->body.end());
      move(a->body.begin(), a->body.end(), back_insert_iterator(body));
      src.reset(new Function2(src->pos, std::move(defs1), std::move(a->closureDefs), std::move(body)));
    } else if (auto a = dynamic_cast<ReferenceNode *>(src.get()); a) {
      trans2(src, defs, *a);
    }
  }

  void transCall(Defs *defs, Call &src) {
    trans2(defs, src.callee);
    trans2(defs, src.args.begin(), src.args.end());
  }

  void makeRef2s(UniqNodes &dest, Defs &defs, size_t paramc) {
    for (int i = 0; i < paramc; ++i) {
      if (defs[i].closure) {
        Reference ref(Reference::LOCAL_SCOPE, i, defs[i].name);
        UniqNode a;
        a.reset(new ReferenceNode(nullptr, ref));
        a.reset(new MakeRef2(nullptr, std::move(a)));
        dest.push_back(std::move(a));
      }
    }
  }

  void makeRef2s1(UniqNodes &dest, Defs &defs, int i, size_t defc) {
    for (; i < defc; ++i) {
      if (defs[i].closure) {
        Reference ref(Reference::LOCAL_SCOPE, i, defs[i].name);
        UniqNode a(new ReferenceNode(nullptr, ref));
        UniqNode b(new Null(nullptr));
        a.reset(new AssignOper(nullptr, { std::move(a), std::move(b) }));
        a.reset(new MakeRef2(nullptr, std::move(a)));
        dest.push_back(std::move(a));
      }
    }
  }

  static bool isClosureRef(Defs *defs, const string *name) noexcept;

  void trans2(UniqNode &dest, Defs *defs, AssignOper &src) {
    trans2(defs, src.items[1]);
    if (auto a = dynamic_cast<const ReferenceNode *>(src.items[0].get()); a && isClosureRef(defs, a->name)) {
      dest.reset(new SetRef2Oper(src.pos, std::move(src.items)));
    } else {
      trans2(defs, src.items[0]);
    }
  }

  bool isClosureRef(Defs *defs, const string *name) noexcept {
    auto it = find_if(defs->begin(), defs->end(), [name] (auto &def) { return def.name == name; });
    return it->closure;
  }

  void trans2(UniqNode &dest, Defs *defs, ReferenceNode &src) {
    if (isClosureRef(defs, src.name)) {
      dest.reset(new GetRef2Oper(src.pos, std::move(dest)));
    }
  }
}

#include<sstream>
#include"commons/opcode.hh"
#include"ilispc.hh"

using namespace std;

namespace zlt::ilispc {
  namespace opcode = ilisp::opcode;

  template<class T>
  inline void writeT(ostream &dest, T value) {
    dest.write((const char *) &value, sizeof(T));
  }

  static void compileCall(ostream &dest, bool hasGuard, const Call &src);
  static void compileIf(ostream &dest, bool hasGuard, const If &src);
  static void compileOper2(ostream &dest, bool hasGuard, int op, Operation<2> &src);

  using It = UniqNodes::const_iterator;

  static void compileOperX(ostream &dest, bool hasGuard, int op, It it, It end);
  static void logicAnd(string &dest, bool hasGuard, It it, It end);
  static void logicOr(string &dest, bool hasGuard, It it, It end);
  static void compileAssign(ostream &dest, bool hasGuard, const AssignOper &src);
  static void compileRef(ostream &dest, bool hasGuard, const Reference &src);
  static void compileFn(ostream &dest, bool hasGuard, const Function2 &src);

  void compile(ostream &dest, bool hasGuard, const UniqNode &src) {
    if (auto a = dynamic_cast<const StringLiteral *>(src.get()); a) {
      dest.put(opcode::SET_STR);
      writeT(dest, a->value);
    } else if (auto a = dynamic_cast<const CallNode *>(src.get()); a) {
      compileCall(dest, hasGuard, *a);
      dest.put(opcode::CALL);
      writeT(dest, a->args.size());
    } else if (auto a = dynamic_cast<const Callee *>(src.get()); a) {
      dest.put(opcode::GET_LOCAL);
      writeT(dest, (int) -1);
    } else if (auto a = dynamic_cast<const Defer *>(src.get()); a) {
      compile(dest, hasGuard, a->value);
      dest.put(opcode::DEFER);
    } else if (auto a = dynamic_cast<const Forward *>(src.get()); a) {
      // TODO guard
      compileCall(dest, hasGuard, *a);
      dest.put(opcode::FORWARD);
    } else if (auto a = dynamic_cast<const Guard *>(src.get()); a) {
      compile(dest, hasGuard, a->value);
      dest.put(opcode::PUSH_GUARD);
    } else if (auto a = dynamic_cast<const If *>(src.get()); a) {
      compileIf(dest, hasGuard, *a);
    } else if (auto a = dynamic_cast<const Null *>(src.get()); a) {
      dest.put(opcode::SET_NULL);
    } else if (auto a = dynamic_cast<const NumberLiteral *>(src.get()); a) {
      dest.put(opcode::SET_NUM);
      writeT(dest, a->value);
    } else if (auto a = dynamic_cast<const Return *>(src.get()); a) {
      // TODO guard
      compile(dest, hasGuard, a->value);
      dest.put(opcode::RETURN);
    } else if (auto a = dynamic_cast<const Throw *>(src.get()); a) {
      // TODO guard
      compile(dest, hasGuard, a->value);
      dest.put(opcode::THROW);
    } else if (auto a = dynamic_cast<const Try *>(src.get()); a) {
      // TODO catch
      compileCall(dest, hasGuard, *a);
    }
    // arithmetical operations begin
    else if (auto a = dynamic_cast<const ArithAddOper *>(src.get()); a) {
      compileOperX(dest, hasGuard, opcode::ADD, a->items.begin(), a->items.end());
    } else if (auto a = dynamic_cast<const ArithSubOper *>(src.get()); a) {
      compileOperX(dest, hasGuard, opcode::SUB, a->items.begin(), a->items.end());
    } else if (auto a = dynamic_cast<const ArithMulOper *>(src.get()); a) {
      compileOperX(dest, hasGuard, opcode::MUL, a->items.begin(), a->items.end());
    } else if (auto a = dynamic_cast<const ArithDivOper *>(src.get()); a) {
      compileOperX(dest, hasGuard, opcode::DIV, a->items.begin(), a->items.end());
    } else if (auto a = dynamic_cast<const ArithModOper *>(src.get()); a) {
      compileOperX(dest, hasGuard, opcode::MOD, a->items.begin(), a->items.end());
    } else if (auto a = dynamic_cast<const ArithPowOper *>(src.get()); a) {
      compileOperX(dest, hasGuard, opcode::POW, a->items.begin(), a->items.end());
    }
    // arithmetical operations end
    // logical operations begin
    else if (auto a = dynamic_cast<const LogicAndOper *>(src.get()); a) {
      string s;
      logicAnd(s, hasGuard, a->items.begin(), a->items.end());
      dest << s;
    } else if (auto a = dynamic_cast<const LogicOrOper *>(src.get()); a) {
      string s;
      logicOr(s, hasGuard, a->items.begin(), a->items.end());
      dest << s;
    } else if (auto a = dynamic_cast<const LogicNotOper *>(src.get()); a) {
      compile(dest, hasGuard, a->item);
      dest.put(opcode::LOGIC_NOT);
    } else if (auto a = dynamic_cast<const LogicXorOper *>(src.get()); a) {
      compileOperX(dest, hasGuard, opcode::LOGIC_XOR, a->items.begin(), a->items.end());
    }
    // logical operations end
    // bitwise operations begin
    else if (auto a = dynamic_cast<const BitwsAndOper *>(src.get()); a) {
      compileOperX(dest, hasGuard, opcode::BITWS_AND, a->items.begin(), a->items.end());
    } else if (auto a = dynamic_cast<const BitwsOrOper *>(src.get()); a) {
      compileOperX(dest, hasGuard, opcode::BITWS_OR, a->items.begin(), a->items.end());
    } else if (auto a = dynamic_cast<const BitwsNotOper *>(src.get()); a) {
      compile(dest, hasGuard, a->item);
      dest.put(opcode::BITWS_NOT);
    } else if (auto a = dynamic_cast<const BitwsXorOper *>(src.get()); a) {
      compileOperX(dest, hasGuard, opcode::BITWS_XOR, a->items.begin(), a->items.end());
    } else if (auto a = dynamic_cast<const LshOper *>(src.get()); a) {
      compileOperX(dest, hasGuard, opcode::LSH, a->items.begin(), a->items.end());
    } else if (auto a = dynamic_cast<const RshOper *>(src.get()); a) {
      compileOperX(dest, hasGuard, opcode::RSH, a->items.begin(), a->items.end());
    } else if (auto a = dynamic_cast<const UshOper *>(src.get()); a) {
      compileOperX(dest, hasGuard, opcode::USH, a->items.begin(), a->items.end());
    }
    // bitwise operations end
    // comparisons begin
    else if (auto a = dynamic_cast<const CmpEqOper *>(src.get()); a) {
      compileOper2(dest, hasGuard, opcode::CMP_EQ, *a);
    } else if (auto a = dynamic_cast<const CmpLtOper *>(src.get()); a) {
      compileOper2(dest, hasGuard, opcode::CMP_LT, *a);
    } else if (auto a = dynamic_cast<const CmpGtOper *>(src.get()); a) {
      compileOper2(dest, hasGuard, opcode::CMP_GT, *a);
    } else if (auto a = dynamic_cast<const CmpLteqOper *>(src.get()); a) {
      compileOper2(dest, hasGuard, opcode::CMP_LTEQ, *a);
    } else if (auto a = dynamic_cast<const CmpGteqOper *>(src.get()); a) {
      compileOper2(dest, hasGuard, opcode::CMP_GTEQ, *a);
    } else if (auto a = dynamic_cast<const Cmp3wayOper *>(src.get()); a) {
      compileOper2(dest, hasGuard, opcode::CMP_3WAY, *a);
    }
    // comparisons end
    // signed operations begin
    else if (auto a = dynamic_cast<const PositiveOper *>(src.get()); a) {
      compile(dest, hasGuard, a->item);
      dest.put(opcode::POSITIVE);
    } else if (auto a = dynamic_cast<const NegativeOper *>(src.get()); a) {
      compile(dest, hasGuard, a->item);
      dest.put(opcode::NEGATIVE);
    }
    // signed operations end
    else if (auto a = dynamic_cast<const AssignOper *>(src.get()); a) {
      compileAssign(dest, hasGuard, *a);
    } else if (auto a = dynamic_cast<const GetMembOper *>(src.get()); a) {
      compileOperX(dest, hasGuard, opcode::GET_MEMB, a->items.begin(), a->items.end());
    } else if (auto a = dynamic_cast<const LengthOper *>(src.get()); a) {
      compile(dest, hasGuard, a->item);
      dest.put(opcode::LENGTH);
    } else if (auto a = dynamic_cast<const Sequence *>(src.get()); a) {
      compile(dest, hasGuard, a->items.begin(), a->items.end());
    } else if (auto a = dynamic_cast<const SetMembOper *>(src.get()); a) {
      compile(dest, hasGuard, a->items[0]);
      dest.put(opcode::PUSH);
      compile(dest, hasGuard, a->items[1]);
      dest.put(opcode::PUSH);
      compile(dest, hasGuard, a->items[2]);
      dest.put(opcode::SET_MEMB);
    } else if (auto a = dynamic_cast<const ReferenceNode *>(src.get()); a) {
      compileRef(dest, hasGuard, *a);
    } else if (auto a = dynamic_cast<const GetRef2Oper *>(src.get()); a) {
      compile(dest, hasGuard, a->item);
      dest.put(opcode::GET_REF2);
    } else if (auto a = dynamic_cast<const SetRef2Oper *>(src.get()); a) {
      compileOper2(dest, hasGuard, opcode::SET_REF2, *a);
    } else if (auto a = dynamic_cast<const Function2 *>(src.get()); a) {
      compileFn(dest, hasGuard, *a);
    } else {
      throw Bad(bad::UNKNOWN_REASON, src->pos);
    }
  }

  void compileCall(ostream &dest, bool hasGuard, const Call &src) {
    compile(dest, hasGuard, src.callee);
    dest.put(opcode::PUSH);
    for (auto &arg : args) {
      compile(dest, hasGuard, arg);
      dest.put(opcode::PUSH);
    }
  }

  void compileIf(ostream &dest, bool hasGuard, const If &src) {
    compile(dest, hasGuard, a->cond);
    auto f = [] (string &dest, bool hasGuard, const UniqNode &src) {
      stringstream ss;
      compile(ss, hasGuard, src);
      dest = ss.str();
    }
    string then;
    f(then, hasGuard, a->then);
    string elze;
    f(elze, hasGuard, a->elze);
    dest.put(opcode::JIF);
    writeT(dest, elze.size() + 1 + sizeof(size_t));
    dest << elze;
    dest.put(opcode::JMP);
    writeT(dest, then.size());
    dest << then;
  }

  void compileOper2(ostream &dest, bool hasGuard, int op, Operation<2> &src) {
    compile(dest, hasGuard, src.items[0]);
    dest.put(opcode::PUSH);
    compile(dest, hasGuard, src.items[1]);
    dest.put(op);
  }

  void compileOperX(ostream &dest, bool hasGuard, int op, It it, It end) {
    compile(dest, hasGuard, *it);
    while (++it != end) {
      dest.put(opcode::PUSH);
      compile(dest, hasGuard, *it);
      dest.put(op);
    }
  }

  void logicAnd(string &dest, bool hasGuard, It it, It end) {
    stringstream ss;
    compile(ss, hasGuard, *it);
    string s;
    logicAnd(s, hasGuard, ++it, end);
    ss.put(opcode::JIF);
    writeT(ss, 1 + sizeof(size_t));
    ss.put(opcode::JMP);
    writeT(ss, s.size());
    ss << s;
    dest = ss.str();
  }

  void logicOr(string &dest, bool hasGuard, It it, It end) {
    stringstream ss;
    compile(ss, hasGuard, *it);
    string s;
    logicOr(s, hasGuard, ++it, end);
    ss.put(opcode::JIF);
    writeT(ss, s.size());
    ss << s;
    dest = ss.str();
  }

  void compileAssign(ostream &dest, bool hasGuard, const AssignOper &src) {
    compile(dest, hasGuard, src.items[1]);
    auto a = static_cast<const ReferenceNode *>(src.items[0].get());
    if (a->scope == Reference::CLOSURE_SCOPE) {
      dest.put(opcode::SET_CLOSURE);
      writeT(dest, a->index);
    } else if (a->scope == Reference::GLOBAL_SCOPE) {
      dest.put(opcode::SET_GLOBAL);
      writeT(dest, a->name);
    } else {
      dest.put(opcode::SET_LOCAL);
      writeT(dest, a->index);
    }
  }

  void compileRef(ostream &dest, bool hasGuard, const Reference &src) {
    if (src.scope == Reference::CLOSURE_SCOPE) {
      dest.put(opcode::GET_CLOSURE);
      writeT(dest, src.index);
    } else if (src.scope == Reference::GLOBAL_SCOPE) {
      dest.put(opcode::GET_GLOBAL);
      writeT(dest, src.name);
    } else {
      dest.put(opcode::GET_LOCAL);
      writeT(dest, src.index);
    }
  }

  static void compileFnBody(string &dest, bool hasGuard, const Function2 &src);

  void compileFn(ostream &dest, bool hasGuard, const Function2 &src) {
    string body;
    compileFnBody(body, hasGuard, src);
    dest.put(opcode::MAKE_FN);
    writeT(dest, src.closureDefs.size());
    writeT(dest, body.size());
    dest << body;
    if (src.closureDefs.size()) {
      dest.put(opcode::PUSH);
      for (int i = 0; i < src.closureDefs.size(); ++i) {
        compileRef(dest, hasGuard, src.closureDefs[i]);
        dest.put(opcode::SET_FN_CLOSURE);
        writeT(dest, i);
      }
      dest.put(opcode::POP);
    }
  }

  void compileFnBody(string &dest, bool hasGuard, const Function2 &src) {
    stringstream ss;
    ss.put(opcode::INIT_DEFC);
    writeT(ss, src.paramc);
    writeT(ss, src.defs.size());
    for (int i = 0; i < src.defs.size(); ++i) {
      if (src.defs[i].closure) {
        ss.put(opcode::MAKE_REF2);
        writeT(ss, i);
      }
    }
    compile(ss, hasGuard, src.body.begin(), src.body.end());
    dest = ss.str();
  }
}

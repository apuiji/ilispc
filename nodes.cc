#include"nodes.hh"
#include"token.hh"

using namespace std;

namespace zlt::ilispc {
  string_view NumberNode::getRaw() const noexcept {
    return *raw;
  }

  string_view ID::getRaw() const noexcept {
    return *raw;
  }

  string_view TokenNode::getRaw() const noexcept {
    return ilispc::token::raw(token);
  }
}

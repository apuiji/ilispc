#include"ilispc.hh"

using namespace std;

namespace zlt::ilispc {
  bool PosSetCompare::operator ()(const Pos &a, const Pos &b) const noexcept {
    if (a.file > b.file || a.li > b.li || a.prev > b.prev) {
      return false;
    }
    if (a.file < b.file || a.li < b.li || a.prev < b.prev) {
      return true;
    }
    return false;
  }
}

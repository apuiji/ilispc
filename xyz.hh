#pragma once

#include<utility>

namespace zlt {
  template<class ...T>
  struct Dynamicastable {
    template<class U>
    requires (std::is_pointer_v<U>)
    bool operator ()(U u) const noexcept {
      return (dynamic_cast<const T *>(u) || ...);
    }
    template<class U>
    requires (!std::is_pointer_v<U>)
    bool operator ()(U &&u) const noexcept {
      return operator ()(&u);
    }
  };

  template<class ...T>
  struct Types {};

  template<class T>
  static inline constexpr T initializerListGet(std::initializer_list<T> ts, size_t i) noexcept {
    for (T t : ts) {
      if (!i) {
        return t;
      }
      --i;
    }
    return T();
  }

  template<class T, class U>
  static inline constexpr int initializerListIndexOf(std::initializer_list<T> ts, U u) noexcept {
    int i = 0;
    for (T t : ts) {
      if (t == u) {
        return i;
      }
      ++i;
    }
    return -1;
  }
}

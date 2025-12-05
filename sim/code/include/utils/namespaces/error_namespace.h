#pragma once
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <iterator>
#include <source_location>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// must bound it with conf value. To keep it constexpr need to bind as
// config.h,in
constexpr bool GLOBAL_DEBUG_BUILD = 1;

namespace debug {

#ifndef CURRENT_MODULE_DEBUG
#define CURRENT_MODULE_DEBUG GLOBAL_DEBUG_BUILD
#endif

template <typename T>
constexpr void debug_assert(
    T &&condition, std::string_view message = "",
    const std::source_location location = std::source_location::current()) {

  if constexpr (CURRENT_MODULE_DEBUG) {
    if (!static_cast<bool>(condition)) {
      std::cerr << "ASSERT: " << message << " at " << location.file_name()
                << ":" << location.line() << " in " << location.function_name()
                << "\n";
      std::abort();
    }
  }
}

template <typename... Args>
inline constexpr void debug_print_unsafe(std::format_string<Args...> fmt,
                                         Args &&...args) {
  if constexpr (CURRENT_MODULE_DEBUG) {
    std::cout << "[DEBUG] " << std::format(fmt, std::forward<Args>(args)...)
              << std::endl;
    // << "\n";
  }
}

template <typename... Args>
inline constexpr void debug_print(std::string_view fmt, Args &&...args) {
  if constexpr (CURRENT_MODULE_DEBUG) {
    try {
      std::string message = std::vformat(fmt, std::make_format_args(args...));
      std::cout << "[DEBUG] " << message << std::endl;
      std::cout.flush();
    } catch (const std::format_error &e) {
      std::cerr << "[DEBUG FORMAT ERROR] " << e.what() << " for: " << fmt
                << std::endl;
    }
  }
}
} // namespace debug

namespace stringify {
template <typename Range, typename Value = typename Range::value_type>
std::string join(Range const &elements, const char *const delimiter) {
  std::ostringstream os;
  auto b = begin(elements), e = end(elements);

  if (b != e) {
    std::copy(b, prev(e), std::ostream_iterator<Value>(os, delimiter));
    b = prev(e);
  }
  if (b != e) {
    os << *b;
  }

  return os.str();
}

/*! note: imput is assumed to not contain NUL characters
 */
template <typename Input, typename Output,
          typename Value = typename Output::value_type>
void Split(char delimiter, Output &output, Input const &input) {
  using namespace std;
  for (auto cur = begin(input), beg = cur;; ++cur) {
    if (cur == end(input) || *cur == delimiter || !*cur) {
      output.insert(output.end(), Value(beg, cur));
      if (cur == end(input) || !*cur)
        break;
      else
        beg = next(cur);
    }
  }
}
template <typename T> std::string to_debug_string(const T &value) {
  if constexpr (requires { std::format("{}", value); }) {
    return std::format("{}", value);
  } else if constexpr (requires {
                         value.begin();
                         value.end();
                       }) {
    // Pretty-print для контейнеров
    std::vector<std::string> elements;
    for (const auto &elem : value) {
      elements.push_back(to_debug_string(elem));
    }
    return std::format("[{}]", join(elements, ", "));
  } else {
    return std::format("<non-printable: {}>", typeid(T).name());
  }
}

}; // namespace stringify

namespace error {

template <typename T>
concept PODType = std::is_trivial_v<T> && std::is_standard_layout_v<T>;

// Result union type
template <typename T>
  requires PODType<T>
struct Result {
  int code;

  union {
    T value;
    char err_msg[64];
  };
  constexpr Result() : code(0) { std::memset(err_msg, 0, sizeof(err_msg)); }
  static constexpr Result success(T val) {
    Result res;
    res.code = 0;
    res.value = val;
    return res;
  }

  static constexpr Result error(int err_code, const char *msg = "") {
    Result res;
    res.code = err_code;
    std::strncpy(res.err_msg, msg, sizeof(res.err_msg) - 1);
    res.err_msg[sizeof(res.err_msg) - 1] = '\0';
    return res;
  }

  constexpr bool is_ok() const noexcept { return code == 0; }
  constexpr bool is_error() const noexcept { return code != 0; }
  constexpr const T *operator->() const { return &value; }

  constexpr T *operator->() { return &value; }

  constexpr const T &operator*() const & { return value; }

  constexpr T &operator*() & { return value; }

  constexpr T unwrap() const { return value; }

  // Maybe will use if default needed for some non-core api stuff
  constexpr T value_or(T fallback) const { return is_ok() ? value : fallback; }
};

// Complex result type
template <typename T> class CResult {
  static_assert(!std::is_trivial_v<T>, "use Result for trivial types");

  union {
    alignas(alignof(T)) unsigned char value_storage_[sizeof(T)];
    alignas(alignof(char[32])) unsigned char error_storage_[32];
  };
  int code_;

private:
  CResult() noexcept : code_{0} {}

public:
  // SUCCESS
  template <typename Arg> static CResult success(Arg &&arg) {
    CResult res;
    res.code_ = 0;
    new (res.value_storage_) T(std::forward<Arg>(arg));
    return res;
  }

  // ERROR
  static CResult error(int err_code, const char *message = "") {
    CResult res;
    res.code_ = err_code;
    char *dst = reinterpret_cast<char *>(res.error_storage_);
    const char *src = message ? message : "";
    size_t i = 0;
    for (; i < sizeof(res.error_storage_) - 1 && src[i]; ++i)
      dst[i] = src[i];
    dst[i] = '\0';
    return res;
  }

  // Destructor
  ~CResult() {
    if (code_ == 0) {
      value().~T();
    }
  }

  // Move constructor
  CResult(CResult &&other) noexcept : code_(other.code_) {
    if (code_ == 0) {
      new (value_storage_) T(std::move(other.value()));
      other.value().~T();
    } else {
      std::memcpy(error_storage_, other.error_storage_, sizeof(error_storage_));
    }
  }

  // Move assignment
  CResult &operator=(CResult &&other) noexcept {
    if (this != &other) {
      // Destroy current
      if (code_ == 0) {
        value().~T();
      }

      code_ = other.code_;

      if (code_ == 0) {
        new (value_storage_) T(std::move(other.value()));
        other.value().~T();
      } else {
        std::memcpy(error_storage_, other.error_storage_,
                    sizeof(error_storage_));
      }
    }
    return *this;
  }

  // No copy - move only
  CResult(const CResult &) = delete;
  CResult &operator=(const CResult &) = delete;

  // Accessors
  constexpr bool is_ok() const noexcept { return code_ == 0; }
  constexpr bool is_error() const noexcept { return !is_ok(); }
  constexpr int code() const noexcept { return code_; }

  // Value access
  T &value() & { return *reinterpret_cast<T *>(value_storage_); }

  const T &value() const & {
    return *reinterpret_cast<const T *>(value_storage_);
  }

  T &&value() && { return std::move(*reinterpret_cast<T *>(value_storage_)); }

  // Error message access
  const char *error_message() const {
    return reinterpret_cast<const char *>(error_storage_);
  }

  // Safe value extraction
  T unwrap() { return std::move(value()); }

  T unwrap_or(T &&fallback) {
    return is_ok() ? std::move(value()) : std::forward<T>(fallback);
  }
};

} // namespace error

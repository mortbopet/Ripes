#pragma once

#include <functional>

namespace Ripes {

// A typeswitch performs dynamic dispatch on a pointer to a base class and
// executes the first function whose type matches the dynamic type of the
// pointer. All functions must return the same type.
template <typename BaseT, typename ResultT>
class TypeSwitch {
public:
  TypeSwitch(BaseT *ptr) : ptr_(ptr) {}

  template <typename DerivedT>
  TypeSwitch &Case(std::function<ResultT(DerivedT &)> func) {
    if (!result_ && dynamic_cast<DerivedT *>(ptr_)) {
      result_ = func(*static_cast<DerivedT *>(ptr_));
    }
    return *this;
  }

  TypeSwitch &Default(std::function<ResultT(BaseT *)> func) {
    assert(!hadDefault && "Default function already executed");
    hadDefault = true;
    if (!result_)
      result_ = func(ptr_);

    return *this;
  }

  operator ResultT() const {
    assert(result_.has_value() && "No matching type found in TypeSwitch and no "
                                  "Default function was executed.");
    return *result_;
  }

private:
  BaseT *ptr_;
  std::optional<ResultT> result_;
  bool hadDefault = false;
};

} // namespace Ripes

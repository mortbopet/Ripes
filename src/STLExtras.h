//===- llvm/ADT/STLExtras.h - Useful STL related functions ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains some templates that are useful if you are working with the
// STL at all.
//
// No library is required when using these functions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ADT_STLEXTRAS_H
#define LLVM_ADT_STLEXTRAS_H

#include <algorithm>
#include <iterator>

namespace llvm {

/// CRTP base class which implements the entire standard iterator facade
/// in terms of a minimal subset of the interface.
///
/// Use this when it is reasonable to implement most of the iterator
/// functionality in terms of a core subset. If you need special behavior or
/// there are performance implications for this, you may want to override the
/// relevant members instead.
///
/// Note, one abstraction that this does *not* provide is implementing
/// subtraction in terms of addition by negating the difference. Negation isn't
/// always information preserving, and I can see very reasonable iterator
/// designs where this doesn't work well. It doesn't really force much added
/// boilerplate anyways.
///
/// Another abstraction that this doesn't provide is implementing increment in
/// terms of addition of one. These aren't equivalent for all iterator
/// categories, and respecting that adds a lot of complexity for little gain.
///
/// Classes wishing to use `iterator_facade_base` should implement the following
/// methods:
///
/// Forward Iterators:
///   (All of the following methods)
///   - DerivedT &operator=(const DerivedT &R);
///   - bool operator==(const DerivedT &R) const;
///   - const T &operator*() const;
///   - T &operator*();
///   - DerivedT &operator++();
///
/// Bidirectional Iterators:
///   (All methods of forward iterators, plus the following)
///   - DerivedT &operator--();
///
/// Random-access Iterators:
///   (All methods of bidirectional iterators excluding the following)
///   - DerivedT &operator++();
///   - DerivedT &operator--();
///   (and plus the following)
///   - bool operator<(const DerivedT &RHS) const;
///   - DifferenceTypeT operator-(const DerivedT &R) const;
///   - DerivedT &operator+=(DifferenceTypeT N);
///   - DerivedT &operator-=(DifferenceTypeT N);
///
template <typename DerivedT, typename IteratorCategoryT, typename T, typename DifferenceTypeT = std::ptrdiff_t,
          typename PointerT = T*, typename ReferenceT = T&>
class iterator_facade_base {
public:
    using iterator_category = IteratorCategoryT;
    using value_type = T;
    using difference_type = DifferenceTypeT;
    using pointer = PointerT;
    using reference = ReferenceT;

protected:
    enum {
        IsRandomAccess = std::is_base_of<std::random_access_iterator_tag, IteratorCategoryT>::value,
        IsBidirectional = std::is_base_of<std::bidirectional_iterator_tag, IteratorCategoryT>::value,
    };

    /// A proxy object for computing a reference via indirecting a copy of an
    /// iterator. This is used in APIs which need to produce a reference via
    /// indirection but for which the iterator object might be a temporary. The
    /// proxy preserves the iterator internally and exposes the indirected
    /// reference via a conversion operator.
    class ReferenceProxy {
        friend iterator_facade_base;

        DerivedT I;

        ReferenceProxy(DerivedT I) : I(std::move(I)) {}

    public:
        operator ReferenceT() const { return *I; }
    };

    /// A proxy object for computing a pointer via indirecting a copy of a
    /// reference. This is used in APIs which need to produce a pointer but for
    /// which the reference might be a temporary. The proxy preserves the
    /// reference internally and exposes the pointer via a arrow operator.
    class PointerProxy {
        friend iterator_facade_base;

        ReferenceT R;

        template <typename RefT>
        PointerProxy(RefT&& R) : R(std::forward<RefT>(R)) {}

    public:
        PointerT operator->() const { return &R; }
    };

public:
    DerivedT operator+(DifferenceTypeT n) const {
        static_assert(std::is_base_of<iterator_facade_base, DerivedT>::value,
                      "Must pass the derived type to this template!");
        static_assert(IsRandomAccess, "The '+' operator is only defined for random access iterators.");
        DerivedT tmp = *static_cast<const DerivedT*>(this);
        tmp += n;
        return tmp;
    }
    friend DerivedT operator+(DifferenceTypeT n, const DerivedT& i) {
        static_assert(IsRandomAccess, "The '+' operator is only defined for random access iterators.");
        return i + n;
    }
    DerivedT operator-(DifferenceTypeT n) const {
        static_assert(IsRandomAccess, "The '-' operator is only defined for random access iterators.");
        DerivedT tmp = *static_cast<const DerivedT*>(this);
        tmp -= n;
        return tmp;
    }

    DerivedT& operator++() {
        static_assert(std::is_base_of<iterator_facade_base, DerivedT>::value,
                      "Must pass the derived type to this template!");
        return static_cast<DerivedT*>(this)->operator+=(1);
    }
    DerivedT operator++(int) {
        DerivedT tmp = *static_cast<DerivedT*>(this);
        ++*static_cast<DerivedT*>(this);
        return tmp;
    }
    DerivedT& operator--() {
        static_assert(IsBidirectional, "The decrement operator is only defined for bidirectional iterators.");
        return static_cast<DerivedT*>(this)->operator-=(1);
    }
    DerivedT operator--(int) {
        static_assert(IsBidirectional, "The decrement operator is only defined for bidirectional iterators.");
        DerivedT tmp = *static_cast<DerivedT*>(this);
        --*static_cast<DerivedT*>(this);
        return tmp;
    }

#ifndef __cpp_impl_three_way_comparison
    bool operator!=(const DerivedT& RHS) const { return !(static_cast<const DerivedT&>(*this) == RHS); }
#endif

    bool operator>(const DerivedT& RHS) const {
        static_assert(IsRandomAccess, "Relational operators are only defined for random access iterators.");
        return !(static_cast<const DerivedT&>(*this) < RHS) && !(static_cast<const DerivedT&>(*this) == RHS);
    }
    bool operator<=(const DerivedT& RHS) const {
        static_assert(IsRandomAccess, "Relational operators are only defined for random access iterators.");
        return !(static_cast<const DerivedT&>(*this) > RHS);
    }
    bool operator>=(const DerivedT& RHS) const {
        static_assert(IsRandomAccess, "Relational operators are only defined for random access iterators.");
        return !(static_cast<const DerivedT&>(*this) < RHS);
    }

    PointerProxy operator->() { return static_cast<DerivedT*>(this)->operator*(); }
    PointerProxy operator->() const { return static_cast<const DerivedT*>(this)->operator*(); }
    ReferenceProxy operator[](DifferenceTypeT n) {
        static_assert(IsRandomAccess, "Subscripting is only defined for random access iterators.");
        return static_cast<DerivedT*>(this)->operator+(n);
    }
    ReferenceProxy operator[](DifferenceTypeT n) const {
        static_assert(IsRandomAccess, "Subscripting is only defined for random access iterators.");
        return static_cast<const DerivedT*>(this)->operator+(n);
    }
};

namespace detail {

template <typename RangeT>
using IterOfRange = decltype(std::begin(std::declval<RangeT&>()));

template <typename RangeT>
using ValueOfRange = typename std::remove_reference<decltype(*std::begin(std::declval<RangeT&>()))>::type;

template <typename R>
class enumerator_iter;

template <typename R>
struct result_pair {
    using value_reference = typename std::iterator_traits<IterOfRange<R>>::reference;

    friend class enumerator_iter<R>;

    result_pair() = default;
    result_pair(std::size_t Index, IterOfRange<R> Iter) : Index(Index), Iter(Iter) {}

    result_pair(const result_pair<R>& Other) : Index(Other.Index), Iter(Other.Iter) {}
    result_pair& operator=(const result_pair& Other) {
        Index = Other.Index;
        Iter = Other.Iter;
        return *this;
    }

    std::size_t index() const { return Index; }
    const value_reference value() const { return *Iter; }
    value_reference value() { return *Iter; }

private:
    std::size_t Index = std::numeric_limits<std::size_t>::max();
    IterOfRange<R> Iter;
};

template <typename R>
class enumerator_iter : public iterator_facade_base<enumerator_iter<R>, std::forward_iterator_tag, result_pair<R>,
                                                    typename std::iterator_traits<IterOfRange<R>>::difference_type,
                                                    typename std::iterator_traits<IterOfRange<R>>::pointer,
                                                    typename std::iterator_traits<IterOfRange<R>>::reference> {
    using result_type = result_pair<R>;

public:
    explicit enumerator_iter(IterOfRange<R> EndIter) : Result(std::numeric_limits<size_t>::max(), EndIter) {}

    enumerator_iter(std::size_t Index, IterOfRange<R> Iter) : Result(Index, Iter) {}

    result_type& operator*() { return Result; }
    const result_type& operator*() const { return Result; }

    enumerator_iter& operator++() {
        assert(Result.Index != std::numeric_limits<size_t>::max());
        ++Result.Iter;
        ++Result.Index;
        return *this;
    }

    bool operator==(const enumerator_iter& RHS) const {
        // Don't compare indices here, only iterators.  It's possible for an end
        // iterator to have different indices depending on whether it was created
        // by calling std::end() versus incrementing a valid iterator.
        return Result.Iter == RHS.Result.Iter;
    }

    enumerator_iter(const enumerator_iter& Other) : Result(Other.Result) {}
    enumerator_iter& operator=(const enumerator_iter& Other) {
        Result = Other.Result;
        return *this;
    }

private:
    result_type Result;
};

template <typename R>
class enumerator {
public:
    explicit enumerator(R&& Range) : TheRange(std::forward<R>(Range)) {}

    enumerator_iter<R> begin() { return enumerator_iter<R>(0, std::begin(TheRange)); }

    enumerator_iter<R> end() { return enumerator_iter<R>(std::end(TheRange)); }

private:
    R TheRange;
};

}  // end namespace detail

//===----------------------------------------------------------------------===//
//     Extra additions to <iterator>
//===----------------------------------------------------------------------===//

namespace adl_detail {

using std::begin;

template <typename ContainerTy>
decltype(auto) adl_begin(ContainerTy&& container) {
    return begin(std::forward<ContainerTy>(container));
}

using std::end;

template <typename ContainerTy>
decltype(auto) adl_end(ContainerTy&& container) {
    return end(std::forward<ContainerTy>(container));
}

using std::swap;

template <typename T>
void adl_swap(T&& lhs, T&& rhs) noexcept(noexcept(swap(std::declval<T>(), std::declval<T>()))) {
    swap(std::forward<T>(lhs), std::forward<T>(rhs));
}

}  // end namespace adl_detail

template <typename ContainerTy>
decltype(auto) adl_begin(ContainerTy&& container) {
    return adl_detail::adl_begin(std::forward<ContainerTy>(container));
}

template <typename ContainerTy>
decltype(auto) adl_end(ContainerTy&& container) {
    return adl_detail::adl_end(std::forward<ContainerTy>(container));
}

/// Given an input range, returns a new range whose values are are pair (A,B)
/// such that A is the 0-based index of the item in the sequence, and B is
/// the value from the original sequence.  Example:
///
/// std::vector<char> Items = {'A', 'B', 'C', 'D'};
/// for (auto X : enumerate(Items)) {
///   printf("Item %d - %c\n", X.index(), X.value());
/// }
///
/// Output:
///   Item 0 - A
///   Item 1 - B
///   Item 2 - C
///   Item 3 - D
///
template <typename R>
detail::enumerator<R> enumerate(R&& TheRange) {
    return detail::enumerator<R>(std::forward<R>(TheRange));
}

/// Provide wrappers to std::for_each which take ranges instead of having to
/// pass begin/end explicitly.
template <typename R, typename UnaryFunction>
UnaryFunction for_each(R&& Range, UnaryFunction F) {
    return std::for_each(adl_begin(Range), adl_end(Range), F);
}

/// Provide wrappers to std::all_of which take ranges instead of having to pass
/// begin/end explicitly.
template <typename R, typename UnaryPredicate>
bool all_of(R&& Range, UnaryPredicate P) {
    return std::all_of(adl_begin(Range), adl_end(Range), P);
}

/// Provide wrappers to std::any_of which take ranges instead of having to pass
/// begin/end explicitly.
template <typename R, typename UnaryPredicate>
bool any_of(R&& Range, UnaryPredicate P) {
    return std::any_of(adl_begin(Range), adl_end(Range), P);
}

/// Provide wrappers to std::none_of which take ranges instead of having to pass
/// begin/end explicitly.
template <typename R, typename UnaryPredicate>
bool none_of(R&& Range, UnaryPredicate P) {
    return std::none_of(adl_begin(Range), adl_end(Range), P);
}

/// Wrapper function around std::transform to apply a function to a range and
/// store the result elsewhere.
template <typename R, typename OutputIt, typename UnaryFunction>
OutputIt transform(R&& Range, OutputIt d_first, UnaryFunction F) {
    return std::transform(adl_begin(Range), adl_end(Range), d_first, F);
}

/// Provide wrappers to std::find_if which take ranges instead of having to pass
/// begin/end explicitly.
template <typename R, typename UnaryPredicate>
auto find_if(R&& Range, UnaryPredicate P) {
    return std::find_if(adl_begin(Range), adl_end(Range), P);
}

template <typename R, typename UnaryPredicate>
auto find_if_not(R&& Range, UnaryPredicate P) {
    return std::find_if_not(adl_begin(Range), adl_end(Range), P);
}

}  // namespace llvm

#endif  // LLVM_ADT_STLEXTRAS_H

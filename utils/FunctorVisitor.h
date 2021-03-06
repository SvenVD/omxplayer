#pragma once

// Author: Torarin Hals Bakke (2012)

// Boost Software License - Version 1.0 - August 17th, 2003

// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:

// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include <type_traits>
#include <utility>

#include "utils/traits.hpp"

namespace functor_visitor_impl
{
  template <typename T>
  using arg_type = typename utils::function_traits<T>::template arg<0>::type;

  template <typename, typename = void>
  class op_call;

  template <typename T>
  class op_call<T, typename std::enable_if<
                     std::is_reference<arg_type<T>>::value>::type>
  {
    T fun;

  public:
    template <typename U>
    op_call(U&& u)
    : fun(std::forward<U>(u))
    {}

    void operator()(arg_type<T> t) {
      fun(std::forward<arg_type<T>>(t));
    }
  };

  template <typename T>
  class op_call<T, typename std::enable_if<
                     !std::is_reference<arg_type<T>>::value>::type>
  {
    T fun;

  public:
    template <typename U>
    op_call(U&& u)
    : fun(std::forward<U>(u))
    {}

    void operator()(const typename std::remove_cv<arg_type<T>>::type& t) {
      fun(t);
    }

    void operator()(const volatile arg_type<T>& t) {
      fun(t);
    }

    void operator()(typename std::remove_cv<arg_type<T>>::type&& t) {
      fun(std::move(t));
    }

    void operator()(volatile typename std::remove_cv<arg_type<T>>::type&& t) {
      fun(std::move(t));
    }

    void operator()(const typename std::remove_cv<arg_type<T>>::type&& t) {
      fun(std::move(t));
    }

    void operator()(const volatile arg_type<T>&& t) {
      fun(std::move(t));
    }
  };
}

template <typename... Ts>
struct functor_visitor;

template <>
struct functor_visitor<> {
  void operator()(struct dummy_type);
  typedef void result_type;
};

template <typename T, typename... Ts>
struct functor_visitor<T, Ts...>
: functor_visitor_impl::op_call<T>,
  functor_visitor<Ts...>
{
  template <typename U, typename... Us>
  functor_visitor(U&& u, Us&&... us)
  : functor_visitor_impl::op_call<T>(std::forward<U>(u)),
    functor_visitor<Ts...>(std::forward<Us>(us)...)
  {}

  using functor_visitor<Ts...>::operator();
  using functor_visitor_impl::op_call<T>::operator();
};

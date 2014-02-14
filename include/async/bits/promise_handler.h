//
// promise_handler.h
// ~~~~~~~~~~~~~~~~~
// A function object adapter to allow a promise to be used in a callback.
//
// Copyright (c) 2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASYNC_BITS_PROMISE_HANDLER_H
#define ASYNC_BITS_PROMISE_HANDLER_H

#include <exception>
#include <system_error>
#include <tuple>
#include <utility>

namespace std {

template <class... _Values>
struct __promise_handler
{
  promise<tuple<_Values...>> _M_promise;

  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : _M_promise(allocator_arg, __u.get_allocator()) {}

  template <class... _Args>
  void operator()(_Args&&... __args)
  {
    _M_promise.set_value(std::make_tuple(forward<_Args>(__args)...));
  }
};

template <>
struct __promise_handler<>
{
  promise<void> _M_promise;

  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : _M_promise(allocator_arg, __u.get_allocator()) {}

  void operator()()
  {
    _M_promise.set_value();
  }
};

template <>
struct __promise_handler<error_code>
{
  promise<void> _M_promise;

  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : _M_promise(allocator_arg, __u.get_allocator()) {}

  void operator()(const error_code& __e)
  {
    if (__e)
      _M_promise.set_exception(make_exception_ptr(system_error(__e)));
    else
      _M_promise.set_value();
  }
};

template <>
struct __promise_handler<exception_ptr>
{
  promise<void> _M_promise;

  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : _M_promise(allocator_arg, __u.get_allocator()) {}

  void operator()(const exception_ptr& __e)
  {
    if (__e)
      _M_promise.set_exception(__e);
    else
      _M_promise.set_value();
  }
};

template <class _Value>
struct __promise_handler<_Value>
{
  promise<_Value> _M_promise;

  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : _M_promise(allocator_arg, __u.get_allocator()) {}

  template <class _Arg>
  void operator()(_Arg&& __a)
  {
    _M_promise.set_value(forward<_Arg>(__a));
  }
};

template <class _Value>
struct __promise_handler<error_code, _Value>
{
  promise<_Value> _M_promise;

  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : _M_promise(allocator_arg, __u.get_allocator()) {}

  template <class _Arg>
  void operator()(const error_code& __e, _Arg&& __a)
  {
    if (__e)
      _M_promise.set_exception(make_exception_ptr(system_error(__e)));
    else
      _M_promise.set_value(forward<_Arg>(__a));
  }
};

template <class _Value>
struct __promise_handler<exception_ptr, _Value>
{
  promise<_Value> _M_promise;

  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : _M_promise(allocator_arg, __u.get_allocator()) {}

  template <class _Arg>
  void operator()(const exception_ptr& __e, _Arg&& __a)
  {
    if (__e)
      _M_promise.set_exception(__e);
    else
      _M_promise.set_value(forward<_Arg>(__a));
  }
};

template <class... _Values>
struct __promise_handler<error_code, _Values...>
{
  promise<tuple<_Values...>> _M_promise;

  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : _M_promise(allocator_arg, __u.get_allocator()) {}

  template <class... _Args>
  void operator()(const error_code& __e, _Args&&... __args)
  {
    if (__e)
      _M_promise.set_exception(make_exception_ptr(system_error(__e)));
    else
      _M_promise.set_value(std::make_tuple(forward<_Args>(__args)...));
  }
};

template <class... _Values>
struct __promise_handler<exception_ptr, _Values...>
{
  promise<tuple<_Values...>> _M_promise;

  template <class _Alloc>
  __promise_handler(use_future_t<_Alloc> __u)
    : _M_promise(allocator_arg, __u.get_allocator()) {}

  template <class... _Args>
  void operator()(const exception_ptr& __e, _Args&&... __args)
  {
    if (__e)
      _M_promise.set_exception(__e);
    else
      _M_promise.set_value(std::make_tuple(forward<_Args>(__args)...));
  }
};

template <class... _Values>
class async_result<__promise_handler<_Values...>>
{
public:
  typedef __promise_handler<_Values...> _Handler;
  typedef decltype(declval<_Handler>()._M_promise) _Promise;
  typedef decltype(declval<_Promise>().get_future()) type;

  async_result(_Handler& __h)
    : _M_future(__h._M_promise.get_future()) {}

  type get() { return std::move(_M_future); }

private:
  type _M_future;
};

template <class _Alloc, class _R, class... _Args>
struct handler_type<use_future_t<_Alloc>, _R(_Args...)>
{
  typedef __promise_handler<typename decay<_Args>::type...> type;
};

} // namespace std

#endif

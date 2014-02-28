#include <async/future>
#include <cassert>
#include <stdexcept>

template <class CompletionToken>
typename std::async_result<
  std::handler_type_t<CompletionToken, void(std::exception_ptr, int, int)>>::type
async_foo(bool fail, CompletionToken&& tok)
{
  std::handler_type_t<CompletionToken, void(std::exception_ptr, int, int)>
    handler(std::forward<CompletionToken>(tok));

  std::async_result<decltype(handler)> result(handler);

  try
  {
    if (fail)
      throw std::runtime_error("fail");

    handler(std::exception_ptr(), 1, 2);
  }
  catch (...)
  {
    handler(std::current_exception(), 1, 2);
  }

  return result.get();
}

int success_count = 0;
int fail_count = 0;

void handler1(const std::exception_ptr& e, int, int)
{
  e ? ++fail_count : ++success_count;
}

struct handler2
{
  handler2() {}
  void operator()(const std::exception_ptr& e, int, int)
  {
    e ? ++fail_count : ++success_count;
  }
};

int main()
{
  async_foo(false, handler1);
  async_foo(true, handler1);

  async_foo(false, &handler1);
  async_foo(true, &handler1);

  async_foo(false, handler2());
  async_foo(true, handler2());

  handler2 h1;
  async_foo(false, h1);
  async_foo(true, h1);

  const handler2 h2;
  async_foo(false, h2);
  async_foo(true, h2);

  async_foo(false, [](std::exception_ptr e, int, int){ e ? ++fail_count : ++success_count; });
  async_foo(true, [](std::exception_ptr e, int, int){ e ? ++fail_count : ++success_count; });

  std::future<std::tuple<int, int>> f1 = async_foo(false, std::use_future);
  try
  {
    f1.get();
    ++success_count;
  }
  catch (...)
  {
    ++fail_count;
  }

  std::future<std::tuple<int, int>> f2 = async_foo(true, std::use_future);
  try
  {
    f2.get();
    ++success_count;
  }
  catch (...)
  {
    ++fail_count;
  }

  assert(success_count == 7);
  assert(fail_count == 7);
}

#include <async/future>
#include <cassert>

template <class CompletionToken>
typename std::async_result<
  typename std::handler_type<CompletionToken,
    void(std::error_code, int)>::type>::type
async_foo(bool fail, CompletionToken&& tok)
{
  typename std::handler_type<CompletionToken,
    void(std::error_code, int)>::type handler(
      std::forward<CompletionToken>(tok));

  std::async_result<decltype(handler)> result(handler);

  if (fail)
    handler(make_error_code(std::errc::invalid_argument), 1);
  else
    handler(std::error_code(), 1);

  return result.get();
}

int success_count = 0;
int fail_count = 0;

void handler1(const std::error_code& e, int)
{
  e ? ++fail_count : ++success_count;
}

struct handler2
{
  handler2() {}
  void operator()(const std::error_code& e, int)
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

  async_foo(false, [](std::error_code e, int){ e ? ++fail_count : ++success_count; });
  async_foo(true, [](std::error_code e, int){ e ? ++fail_count : ++success_count; });

  std::future<int> f1 = async_foo(false, std::use_future);
  try
  {
    int v = f1.get();
    assert(v == 1);
    ++success_count;
  }
  catch (...)
  {
    ++fail_count;
  }

  std::future<int> f2 = async_foo(true, std::use_future);
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

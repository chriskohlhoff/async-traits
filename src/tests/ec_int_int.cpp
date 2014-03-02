#include <async/future>
#include <cassert>

template <class CompletionToken>
typename std::async_result<
  std::handler_type_t<CompletionToken, void(std::error_code, int, int)>>::type
async_foo(bool fail, CompletionToken&& tok)
{
  std::async_completion<CompletionToken, void(std::error_code, int, int)> completion(tok);

  if (fail)
    completion.handler(make_error_code(std::errc::invalid_argument), 1, 2);
  else
    completion.handler(std::error_code(), 1, 2);

  return completion.result.get();
}

int success_count = 0;
int fail_count = 0;

void handler1(const std::error_code& e, int, int)
{
  e ? ++fail_count : ++success_count;
}

struct handler2
{
  handler2() {}
  void operator()(const std::error_code& e, int, int)
  {
    e ? ++fail_count : ++success_count;
  }
};

struct handler3
{
  handler3() {}
  handler3(const handler3&) = delete;
  handler3(handler3&&) {}
  void operator()(const std::error_code& e, int, int)
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

  async_foo(false, handler3());
  async_foo(true, handler3());

  handler3 h3, h4;
  async_foo(false, std::move(h3));
  async_foo(true, std::move(h4));

  async_foo(false, [](std::error_code e, int, int){ e ? ++fail_count : ++success_count; });
  async_foo(true, [](std::error_code e, int, int){ e ? ++fail_count : ++success_count; });

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

  assert(success_count == 9);
  assert(fail_count == 9);
}

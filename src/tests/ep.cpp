#include <async/future>
#include <cassert>
#include <stdexcept>

template <class CompletionToken>
auto async_foo(bool fail, CompletionToken&& tok)
{
  std::async_completion<CompletionToken, void(std::exception_ptr)> completion(tok);

  try
  {
    if (fail)
      throw std::runtime_error("fail");

    completion.handler(std::exception_ptr());
  }
  catch (...)
  {
    completion.handler(std::current_exception());
  }

  return completion.result.get();
}

int success_count = 0;
int fail_count = 0;

void handler1(const std::exception_ptr& e)
{
  e ? ++fail_count : ++success_count;
}

struct handler2
{
  handler2() {}
  void operator()(const std::exception_ptr& e)
  {
    e ? ++fail_count : ++success_count;
  }
};

struct handler3
{
  handler3() {}
  handler3(const handler3&) = delete;
  handler3(handler3&&) {}
  void operator()(const std::exception_ptr& e)
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

  async_foo(false, [](std::exception_ptr e){ e ? ++fail_count : ++success_count; });
  async_foo(true, [](std::exception_ptr e){ e ? ++fail_count : ++success_count; });

  std::future<void> f1 = async_foo(false, std::use_future);
  try
  {
    f1.get();
    ++success_count;
  }
  catch (...)
  {
    ++fail_count;
  }

  std::future<void> f2 = async_foo(true, std::use_future);
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

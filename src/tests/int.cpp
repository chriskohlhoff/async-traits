#include <async/future>
#include <cassert>

template <class CompletionToken>
typename std::async_result<
  std::handler_type_t<CompletionToken, void(int)>>::type
async_foo(CompletionToken&& tok)
{
  std::async_completion<CompletionToken, void(int)> completion(tok);

  completion.handler(1);

  return completion.result.get();
}

int success_count = 0;

void handler1(int)
{
  ++success_count;
}

struct handler2
{
  handler2() {}
  void operator()(int) { ++success_count; }
};

struct handler3
{
  handler3() {}
  handler3(const handler3&) = delete;
  handler3(handler3&&) {}
  void operator()(int) { ++success_count; }
};

int main()
{
  async_foo(handler1);

  async_foo(&handler1);

  async_foo(handler2());

  handler2 h1;
  async_foo(h1);

  const handler2 h2;
  async_foo(h2);

  async_foo(handler3());

  handler3 h3;
  async_foo(std::move(h3));

  async_foo([](int){ ++success_count; });

  std::future<int> f = async_foo(std::use_future);
  int v = f.get();
  assert(v == 1);
  ++success_count;

  assert(success_count == 9);
}

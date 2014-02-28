#include <async/future>
#include <cassert>

template <class CompletionToken>
typename std::async_result<
  std::handler_type_t<CompletionToken, void(int)>>::type
async_foo(CompletionToken&& tok)
{
  std::handler_type_t<CompletionToken, void(int)>
    handler(std::forward<CompletionToken>(tok));

  std::async_result<decltype(handler)> result(handler);

  handler(1);

  return result.get();
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

int main()
{
  async_foo(handler1);

  async_foo(&handler1);

  async_foo(handler2());

  handler2 h1;
  async_foo(h1);

  const handler2 h2;
  async_foo(h2);

  async_foo([](int){ ++success_count; });

  std::future<int> f = async_foo(std::use_future);
  int v = f.get();
  assert(v == 1);
  ++success_count;

  assert(success_count == 7);
}

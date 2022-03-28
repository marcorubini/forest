#include <forest/sm2.hpp>
#include <iostream>

struct Root;
struct A;
struct B;
struct C;
struct D;
struct E;
struct F;
struct G;

static constexpr char const* machine_description = R"RAW(
   Root
   |-- A
   |   |-- B
   |-- C
   |-- D
   |-- [E]
        |-- F
        |-- G
       
)RAW";

using Machine = forest::sm::make_machine<machine_description> //
  ::bind<"Root", Root>                                        //
  ::bind<"A", A>                                              //
  ::bind<"B", B>                                              //
  ::bind<"C", C>                                              //
  ::bind<"D", D>                                              //
  ::bind<"E", E>                                              //
  ::bind<"F", F>                                              //
  ::bind<"G", G>;

struct e1
{
  int value;
};

struct e2
{};

struct Root : Machine::state<Root>
{
  void on_entry (state_context ctx)
  {
    std::cout << "Enter root\n";
  }

  auto react (state_context const& ctx, e1 event) -> transit_result<A, B>
  {
    if (event.value == 42) {
      return transit<A> {};
    } else if (event.value == 100) {
      return transit_none {};
    } else {
      return transit<B> {};
    }
  }
};

struct A : Machine::state<A>
{
  void on_entry (state_context const& ctx)
  {
    std::cout << "Enter A\n";
  }

  auto react (exact_context const& ctx, e2 event) -> transit_result<E>
  {
    std::cout << "A handles event 2\n";
    return transit<E> {};
  }
};

struct B : Machine::state<B>
{
  void on_entry (state_context const& ctx)
  {
    std::cout << "Enter B\n";
  }

  auto react (exact_context const& ctx, e2 event) -> transit_result<E>
  {
    std::cout << "B handles event 2\n";
    return transit<E> {};
  }
};

struct C : Machine::state<C>
{};

struct D : Machine::state<D>
{};

struct E : Machine::state<E>
{};

struct e3
{};

struct e4
{};

struct F : Machine::state<F>
{
  auto react (state_context const& ctx, e3) -> transit_result<>
  {
    std::cerr << "F reacts to event 3\n";
    return transit_none {};
  }
};

struct G : Machine::state<G>
{
  auto react (state_context const& ctx, e4) -> transit_result<>
  {
    std::cerr << "G reacts to event 4\n";
    return transit_none {};
  }
};

int main ()
{
  Machine::instance instance {};
  instance.start ();
  instance.react (e1 {});
  instance.react (e2 {});

  std::cout << "Root: " << instance.is_active<Root> () << "\n";
  std::cout << "A: " << instance.is_active<A> () << "\n";
  std::cout << "B: " << instance.is_active<B> () << "\n";
  std::cout << "C: " << instance.is_active<C> () << "\n";
  std::cout << "D: " << instance.is_active<D> () << "\n";
  std::cout << "E: " << instance.is_active<E> () << "\n";
  std::cout << "F: " << instance.is_active<F> () << "\n";
  std::cout << "G: " << instance.is_active<G> () << "\n";

  instance.react (e3 {});
  instance.react (e4 {});

  std::cout << "Root: " << instance.is_active<Root> () << "\n";
  std::cout << "A: " << instance.is_active<A> () << "\n";
  std::cout << "B: " << instance.is_active<B> () << "\n";
  std::cout << "C: " << instance.is_active<C> () << "\n";
  std::cout << "D: " << instance.is_active<D> () << "\n";
  std::cout << "E: " << instance.is_active<E> () << "\n";
  std::cout << "F: " << instance.is_active<F> () << "\n";
  std::cout << "G: " << instance.is_active<G> () << "\n";
}
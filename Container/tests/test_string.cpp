#include <cassert>
#include <string>

#include <forest/container.hpp>

int test_string (int, char**)
{
  using forest::container::literal_string;
  using std::string;

  assert (literal_string ("Hello").size () == 5);
  assert (literal_string ("").size () == 0);
  assert (literal_string<2> ("AB").size () == 2);

  assert (string ("Hello") == literal_string ("Hello").view ());
  assert (string ("") == literal_string ("").view ());
  assert (string ("AB") == literal_string<2> ("AB").view ());

  return 0;
}

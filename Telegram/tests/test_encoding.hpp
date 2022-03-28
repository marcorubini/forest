#include <doctest/doctest.h>
#include <forest/telegram/utility/base64.hpp>

TEST_CASE ("encoding")
{
  using forest::telegram::internal::base64_decode;
  using forest::telegram::internal::base64_encode;

  auto const do_encode = [] (std::string input) -> std::string {
    return base64_encode (input);
  };

  CHECK (do_encode ("Hello") == "SGVsbG8=");
  CHECK (do_encode ("hello") == "aGVsbG8=");
  CHECK (do_encode ("12345678987654321") == "MTIzNDU2Nzg5ODc2NTQzMjE=");
  CHECK (do_encode (" ") == "IA==");
  CHECK (do_encode ("") == "");

  auto const do_decode = [] (std::string input) -> std::string {
    return base64_decode (input);
  };

  CHECK (do_decode ("SGVsbG8=").size () == 5);
  CHECK (do_decode ("SGVsbG8=") == "Hello");
  CHECK (do_decode ("aGVsbG8=") == "hello");
  CHECK (do_decode ("MTIzNDU2Nzg5ODc2NTQzMjE=") == "12345678987654321");
  CHECK (do_decode ("IA==") == " ");
  CHECK (do_decode ("") == "");
}

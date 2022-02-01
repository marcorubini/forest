#include <iostream>
#include <nlohmann/json.hpp>

int main ()
{
  auto json = nlohmann::json::parse ("[0, 1, 2, 42]");

  std::cout << "Printing json \"[0, 1, 2, 42]\"" << std::endl;
  std::cout << json.dump () << std::endl;
}

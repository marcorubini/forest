#include <cpr/cpr.h>
#include <iostream>

int main ()
{
  std::cerr << "GET request to \"http://www.example.org/\"\n";
  cpr::Response r = cpr::Get (cpr::Url {"http://www.example.org/"});
  std::cerr << "Response header:\n" << r.raw_header << std::endl;
}

#include <cereal/archives/portable_binary.hpp>
#include <doctest/doctest.h>
#include <forest/telegram/utility/base64.hpp>
#include <variant>

TEST_CASE ("serialization")
{
  using forest::telegram::internal::base64_decode;
  using forest::telegram::internal::base64_encode;

  auto do_serialize = [] (auto payload) -> std::string {
    auto stream = std::stringstream (std::ios_base::binary | std::ios_base::out);
    auto archive = cereal::PortableBinaryOutputArchive (stream);
    archive (payload);
    return base64_encode (stream.str ());
  };

  CHECK (do_serialize (true) == "AQE=");
  CHECK (do_serialize (std::monostate {}) == "AQ==");

  auto do_serialize_index = [] (long index, auto payload) -> std::string {
    auto stream = std::stringstream (std::ios_base::binary | std::ios_base::out | std::ios_base::in);
    auto archive = cereal::PortableBinaryOutputArchive (stream);
    archive (index);
    archive (payload);

    return base64_encode (stream.str ());
  };

  CHECK (do_serialize_index (0, std::monostate {}) == "AQAAAAAAAAAA");

  auto do_unserialize_undex = [] (std::string encoded, auto payload) {
    std::string decoded = base64_decode (encoded);

    auto stream = std::istringstream (decoded, std::ios_base::binary | std::ios_base::in);
    auto archive = cereal::PortableBinaryInputArchive (stream);
    long index;
    archive (index);
    archive (payload);

    return std::pair (index, payload);
  };

  CHECK (do_unserialize_undex ("AQAAAAAAAAAA", std::monostate {}) == std::pair (0l, std::monostate {}));
}

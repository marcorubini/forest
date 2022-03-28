find_package(doctest 2.4.8 CONFIG)
if(NOT doctest_FOUND)
  cpmaddpackage(NAME doctest VERSION 2.4.8 GITHUB_REPOSITORY "doctest/doctest")
endif()

find_package(nlohmann_json 3.10.5 CONFIG)
if(NOT nlohmann_json_FOUND)
  cpmaddpackage(NAME nlohmann_json VERSION 3.10.5 GITHUB_REPOSITORY
                "nlohmann/json")
endif()

find_package(spdlog 1.9.2 CONFIG)
if(NOT spdlog_FOUND)
  cpmaddpackage(NAME spdlog VERSION 1.9.2 GITHUB_REPOSITORY "gabime/spdlog")
endif()

find_package(TgBot 1.3 CONFIG)
if(NOT TgBot_FOUND)
  cpmaddpackage(NAME tgbot-cpp VERSION 1.3 GITHUB_REPOSITORY "reo7sp/tgbot-cpp")
endif()

find_package(cereal 1.3.2 CONFIG)
if(NOT cereal_FOUND)
  cpmaddpackage(
    NAME
    cereal
    VERSION
    1.3.2
    GITHUB_REPOSITORY
    "USCiLab/cereal"
    OPTIONS
    "BUILD_TESTS OFF"
    "BUILD_SANDBOX OFF"
    "BUILD_DOC OFF"
    "SKIP_PERFORMANCE_COMPARISON ON")
endif()

find_package(Boost 1.76...1.78 CONFIG)
if(NOT Boost_FOUND)
  cpmaddpackage(NAME boost_mp11 VERSION 1.78 GITHUB_REPOSITORY "boostorg/mp11")
  set(BOOST_MP11_TARGET Boost::mp11)
else()
  set(BOOST_MP11_TARGET Boost::boost)
endif()

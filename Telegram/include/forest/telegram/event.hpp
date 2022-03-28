#pragma once
#include <forest/machine.hpp>
#include <forest/signal.hpp>
#include <forest/telegram/driver.hpp>
#include <forest/telegram/utility/base64.hpp>

#include <cereal/archives/portable_binary.hpp>
#include <tgbot/Api.h>

#include <iterator>
#include <string>

namespace forest::telegram
{
  template<class T>
  concept Event = std::move_constructible<T> && requires (T event)
  {
    // clang-format off
    { event.chat_id } -> meta::SameUnqual<api_chat_id>;
    // clang-format on
  };

  // command description

  template<class T>
  concept CommandDescription = requires
  {
    // clang-format off
    { T::name } -> std::convertible_to<std::string_view>;
    { T::description } -> std::convertible_to<std::string_view>;
    // clang-format on
  };

  template<container::literal_string Name, container::literal_string Description>
  struct command_description
  {
    static constexpr std::string_view name = Name;
    static constexpr std::string_view description = Description;
  };

  template<CommandDescription... Cs>
  using command_list = boost::mp11::mp_list<Cs...>;

  // button description

  template<class T>
  concept ButtonDescription = requires
  {
    typename T::payload_type;
  };

  template<class Payload = std::monostate>
  struct button_description
  {
    using payload_type = Payload;
  };

  template<ButtonDescription... Bs>
  using button_list = boost::mp11::mp_list<Bs...>;

  // command event

  template<CommandDescription C>
  struct command_event
  {
    api_chat_id chat_id;
    std::vector<std::string> parameters;
  };

  // button event

  template<ButtonDescription B>
  struct button_event
  {
    using payload_type = typename B::payload_type;

    api_chat_id chat_id;
    api_message_id message_id;
    std::string callback_id;
    payload_type payload;
  };

  // message event

  struct message_event
  {
    api_chat_id chat_id;
    api_message_id message_id;
    std::string text;
  };

  // traits

  namespace internal
  {
    template<class T>
    struct extract_commands : std::type_identity<boost::mp11::mp_list<>>
    {};

    template<class T>
      requires (requires { typename T::my_commands; })
    struct extract_commands<T> : std::type_identity<typename T::my_commands>
    {};

    template<class T>
    using extract_commands_t = typename extract_commands<T>::type;

    template<class... Ts>
    using join_commands_t = boost::mp11::mp_set_union<extract_commands_t<Ts>...>;

    template<class T>
    struct extract_buttons : std::type_identity<boost::mp11::mp_list<>>
    {};

    template<class T>
      requires (requires { typename T::my_buttons; })
    struct extract_buttons<T> : std::type_identity<typename T::my_buttons>
    {};

    template<class T>
    using extract_buttons_t = typename extract_buttons<T>::type;

    template<class... Ts>
    using join_buttons_t = boost::mp11::mp_set_union<extract_buttons_t<Ts>...>;

    template<forest::machine::MachineTraits Traits, ButtonDescription Desc>
    constexpr inline auto serialize_button = [] (typename Desc::payload_type payload) {
      using buttons = boost::mp11::mp_apply<join_buttons_t, typename Traits::list_type>;
      constexpr long index = boost::mp11::mp_find<buttons, Desc>::value;

      auto stream = std::stringstream (std::ios_base::binary | std::ios_base::in | std::ios_base::out);
      auto archive = cereal::PortableBinaryOutputArchive (stream);
      archive (index);
      archive (payload);

      return base64_encode (stream.str ());
    };

    template<forest::machine::MachineTraits Traits>
    constexpr inline auto unserialize_button = []<class Visitor> (std::string_view encoded, Visitor visitor) {
      using buttons = boost::mp11::mp_apply<join_buttons_t, typename Traits::list_type>;
      constexpr long num_buttons = boost::mp11::mp_size<buttons>::value;

      if constexpr (num_buttons == 0) {
        return visitor (std::monostate {}, std::monostate {});
      } else {
        try {
          auto decoded = base64_decode (encoded);
          auto decoded_stream = std::istringstream (decoded, std::ios_base::binary);

          auto archive = cereal::PortableBinaryInputArchive (decoded_stream);
          long index;
          archive (index);

          spdlog::debug ("Unserialize button, index = {}", index);

          if (index < 0 || index >= num_buttons)
            return visitor (std::monostate {}, std::monostate {});

          return boost::mp11::mp_with_index<num_buttons> (index, [&] (auto I) {
            using button_type = boost::mp11::mp_at_c<buttons, I>;
            using payload_type = typename button_type::payload_type;
            payload_type payload {};
            archive (payload);
            return visitor (button_type {}, std::move (payload));
          });
        } catch (...) {
          return visitor (std::monostate {}, std::monostate {});
        }
      }
    };
  } // namespace internal

} // namespace forest::telegram
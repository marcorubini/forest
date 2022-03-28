#pragma once
#include <iostream>

#include <forest/sm/button.hpp>

namespace forest
{

  template<class Derived, class FSM, Button... Buttons>
  class keyboard_handler
  {
  public:
    template<std::same_as<Derived> D = Derived>
    void react (events::button const& event, typename FSM::FullControl& control)
    {
      static_assert (std::derived_from<D, keyboard_handler>);

      std::cerr << "Keyboard handler reacting to button " << event.index << ", " << event.name << std::endl;
      bool reacted = false;
      auto const react_to = [&]<Button CurrentButton> () {
        if (!reacted && event.name == typeid (CurrentButton).name ()) {
          std::cerr << "Reaction found: " << typeid (CurrentButton).name () << std::endl;
          static_cast<D&> (*this).react (CurrentButton {}, event, control);
          reacted = true;
        }
      };
      (react_to.template operator()<Buttons> (), ...);
    }
  };
} // namespace forest
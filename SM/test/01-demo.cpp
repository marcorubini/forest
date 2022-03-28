#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <random>
#include <ranges>
#include <set>

#include <forest/sm/sm.hpp>

namespace demo
{
  namespace pin
  {
    struct PinRegion;
    struct PinNotOk;
    struct PinOk;
  } // namespace pin

  namespace centrale
  {
    struct CentraleRegion;
    struct CentraleIdle;
  } // namespace centrale

  namespace area
  {
    struct AreaRegion;
    struct AreaIdle;
    struct AreaMenu;
  } // namespace area

  namespace eventi
  {
    struct EventiRegion;
    struct EventiIdle;
    struct EventiMenu;
  } // namespace eventi

  namespace uscita
  {
    struct UscitaRegion;
    struct UscitaIdle;
    struct UscitaMenu;
  } // namespace uscita

  namespace notifiche
  {
    struct NotificheRegion;
    struct NotificheIdle;
  } // namespace notifiche

  using namespace pin;
  using namespace centrale;
  using namespace area;
  using namespace eventi;
  using namespace uscita;
  using namespace notifiche;

  using M = forest::Machine;
  using FSM = M::OrthogonalPeerRoot<                    //
    M::Composite<PinRegion, PinNotOk, PinOk>,           //
    M::Composite<CentraleRegion, CentraleIdle>,         //
    M::Composite<AreaRegion, AreaIdle, AreaMenu>,       //
    M::Composite<EventiRegion, EventiIdle, EventiMenu>, //
    M::Composite<UscitaRegion, UscitaIdle, UscitaMenu>, //
    M::Composite<NotificheRegion, NotificheIdle>>;

  struct storage
  {};

  template<class T, auto Member>
  struct name_map_t
  {
  private:
    std::map<std::string, T> _map;

  public:
    name_map_t () = default;

    bool contains (std::string name) const
    {
      return _map.contains (name);
    }

    T& at (std::string name)
    {
      return _map.at (name);
    }

    T& operator[] (std::string name)
    {
      return _map[name];
    }

    void erase (std::string name)
    {
      _map.erase (name);
    }

    auto begin ()
    {
      return _map.begin ();
    }

    auto begin () const
    {
      return _map.begin ();
    }

    auto end ()
    {
      return _map.end ();
    }

    auto end () const
    {
      return _map.end ();
    }

    friend void to_json (nlohmann::json& json, name_map_t const& map)
    {
      auto values = std::vector<T> ();
      for (auto&& x : map)
        values.push_back (x.second);
      json = nlohmann::json (values);
    }

    friend void from_json (nlohmann::json const& json, name_map_t& map)
    {
      std::vector<T> values = json;
      for (auto x : values)
        map._map[std::invoke (Member, x)] = x;
    }
  };

} // namespace demo

namespace demo::centrale
{
  enum class E_CENTRALE
  {
    DUPLICATE,
    MISSING,
    NOT_SELECTED,
    OK
  };

  auto feedback (E_CENTRALE err) -> std::string
  {
    if (err == E_CENTRALE::DUPLICATE)
      return "Centrale già nell'elenco.";
    if (err == E_CENTRALE::MISSING)
      return "Centrale non nell'elenco.";
    if (err == E_CENTRALE::NOT_SELECTED)
      return "Nessuna centrale selezionata.";
    return "OK";
  }

  namespace handler_centrale
  {
    auto get (forest::bot_context context) -> std::set<std::string>
    {
      if (auto centrali = context.get_value_json ("elenco_centrali"); centrali)
        return *centrali;
      return {};
    }

    auto contains (forest::bot_context context, std::string centrale) -> bool
    {
      return get (context).contains (centrale);
    }

    auto add (forest::bot_context context, std::string centrale) -> E_CENTRALE
    {
      auto centrali = get (context);
      if (centrali.contains (centrale))
        return E_CENTRALE::DUPLICATE;

      centrali.insert (centrale);
      context.set_value ("elenco_centrali", nlohmann::json (centrali));
      return E_CENTRALE::OK;
    }

    auto remove (forest::bot_context context, std::string centrale) -> E_CENTRALE
    {
      auto centrali = get (context);
      if (!centrali.contains (centrale))
        return E_CENTRALE::MISSING;

      centrali.erase (centrale);
      context.set_value ("elenco_centrali", nlohmann::json (centrali));
      return E_CENTRALE::OK;
    }

    auto select (forest::bot_context context, std::string centrale) -> E_CENTRALE
    {
      auto centrali = get (context);
      if (!centrali.contains (centrale))
        return E_CENTRALE::MISSING;

      context.set_value ("centrale_selezionata", centrale);
      return E_CENTRALE::OK;
    }

    auto selected (forest::bot_context context) -> std::optional<std::string>
    {
      return context.get_value ("centrale_selezionata");
    }

    auto is_selected (forest::bot_context context) -> E_CENTRALE
    {
      auto centrali = get (context);
      auto selezionata = selected (context);
      if (!selezionata.has_value ())
        return E_CENTRALE::NOT_SELECTED;
      if (!centrali.contains (*selezionata))
        return E_CENTRALE::MISSING;
      return E_CENTRALE::OK;
    }

  }; // namespace handler_centrale

  struct cmd_stato
  {
    static std::string prefix ()
    {
      return "statocentrale";
    }

    static std::string description ()
    {
      return "Stato della centrale selezionata";
    }
  };

  struct cmd_elenco
  {
    static std::string prefix ()
    {
      return "elencocentrali";
    }

    static std::string description ()
    {
      return "Mostra l'elenco delle centrali aggiunte.";
    }
  };

  struct cmd_aggiungi
  {
    static std::string prefix ()
    {
      return "aggiungicentrale";
    }

    static std::string description ()
    {
      return "Aggiungi una centrale.";
    }
  };

  struct cmd_rimuovi
  {
    static std::string prefix ()
    {
      return "rimuovicentrale";
    }

    static std::string description ()
    {
      return "Rimuovi una centrale";
    }
  };

  struct cmd_seleziona
  {
    static std::string prefix ()
    {
      return "selezionacentrale";
    }

    static std::string description ()
    {
      return "Seleziona una centrale.";
    }
  };

  struct CentraleRegion //
    : forest::command_handler<CentraleRegion, FSM, cmd_stato, cmd_aggiungi, cmd_rimuovi, cmd_elenco, cmd_seleziona>,
      FSM::State
  {
    using handler =
      forest::command_handler<CentraleRegion, FSM, cmd_stato, cmd_aggiungi, cmd_rimuovi, cmd_elenco, cmd_seleziona>;

    void react (cmd_stato, forest::events::command const& event, FSM::FullControl& control)
    {
      auto context = control.context ();

      if (auto err = handler_centrale::is_selected (context); err != E_CENTRALE::OK) {
        context.send_message (feedback (err));
        return;
      }

      auto centrale = handler_centrale::selected (context);

      // RICHIESTA ALLA CENTRALE

      std::string answer;
      answer += "Centrale selezionata: " + centrale.value () + "\n";
      context.send_message (answer);
    }

    void react (cmd_aggiungi, forest::events::command const& event, FSM::FullControl& control)
    {
      auto context = control.context ();

      if (event.parameters.empty ()) {
        context.send_message ("Il comando richiede un parametro. /aggiungicentrale nome");
      } else {

        if (auto err = handler_centrale::add (context, event.parameters); err != E_CENTRALE::OK) {
          context.send_message (feedback (err));
          return;
        }

        context.send_message ("Centrale aggiunta.");

        // TODO: AUTORIZZAZIONE
      }
    }

    void react (cmd_rimuovi, forest::events::command const& event, FSM::FullControl& control)
    {
      auto context = control.context ();

      if (event.parameters.empty ()) {
        context.send_message ("Il comando richiede un parametro. /rimuovicentrale nome");
      } else {
        if (auto err = handler_centrale::remove (context, event.parameters); err != E_CENTRALE::OK) {
          context.send_message (feedback (err));
          return;
        }

        context.send_message ("Centrale rimossa.");
      }
    }

    void react (cmd_seleziona, forest::events::command const& event, FSM::FullControl& control)
    {
      auto context = control.context ();

      if (event.parameters.empty ()) {
        context.send_message ("Il comando richiede un parametro. /selezionacentrale nome");
      } else {
        if (auto err = handler_centrale::select (context, event.parameters); err != E_CENTRALE::OK) {
          context.send_message (feedback (err));
          return;
        }

        context.send_message ("Centrale selezionata.");
      }
    }

    void react (cmd_elenco, forest::events::command const& event, FSM::FullControl& control)
    {
      auto context = control.context ();
      auto centrali = handler_centrale::get (context);

      if (centrali.empty ()) {
        context.send_message ("Nessuna centrale nell'elenco.");
      } else {
        std::string answer;
        for (auto x : centrali)
          answer += x + "\n";
        context.send_message (answer);
      }
    }

    using FSM::State::react;
    using handler::react;
  };

  struct CentraleIdle : FSM::State
  {};
} // namespace demo::centrale

namespace demo::area
{
  struct area_t
  {
    std::string name;
    bool armed;

    friend void to_json (nlohmann::json& json, area_t const& area)
    {
      json = {{"name", area.name}, {"armed", area.armed}};
    }

    friend void from_json (nlohmann::json const& json, area_t& area)
    {
      json.at ("name").get_to (area.name);
      json.at ("armed").get_to (area.armed);
    }
  };

  using area_vector_t = name_map_t<area_t, &area_t::name>;

  enum class E_AREA
  {
    DUPLICATE,
    MISSING,
    OK
  };

  auto feedback (E_AREA err)
  {
    if (err == E_AREA::DUPLICATE)
      return "L'area è già nell'elenco.";
    if (err == E_AREA::MISSING)
      return "L'area non è nell'elenco.";
    return "OK";
  }

  namespace handler_area
  {
    auto get (forest::bot_context context) -> std::map<std::string, area_vector_t>
    {
      if (auto aree = context.get_value_json ("elenco_aree"); aree)
        return *aree;
      return {};
    }

    auto add (forest::bot_context context, std::string centrale, area_t area) -> E_AREA
    {
      auto aree = get (context);
      if (aree[centrale].contains (area.name))
        return E_AREA::DUPLICATE;
      aree[centrale][area.name] = area;
      context.set_value ("elenco_aree", nlohmann::json (aree));
      return E_AREA::OK;
    }

    auto remove (forest::bot_context context, std::string centrale, std::string area) -> E_AREA
    {
      auto aree = get (context);
      if (!aree[centrale].contains (area))
        return E_AREA::MISSING;
      aree[centrale].erase (area);
      context.set_value ("elenco_aree", nlohmann::json (aree));
      return E_AREA::OK;
    }

    auto update (forest::bot_context context, std::string centrale, area_vector_t aree) -> void
    {
      auto old = get (context);
      old.at (centrale) = aree;
      context.set_value ("elenco_aree", nlohmann::json (old));
    }
  } // namespace handler_area

  struct cmd_aggiungi
  {
    static std::string prefix ()
    {
      return "aggiungiarea";
    }

    static std::string description ()
    {
      return "Aggiungi un'area";
    }
  };

  struct cmd_rimuovi
  {
    static std::string prefix ()
    {
      return "rimuoviarea";
    }

    static std::string description ()
    {
      return "Rimuovi un'area";
    }
  };

  struct cmd_area
  {
    static std::string prefix ()
    {
      return "area";
    }

    static std::string description ()
    {
      return "Mostra aree";
    }
  };

  struct AreaRegion //
    : forest::command_handler<AreaRegion, FSM, cmd_aggiungi, cmd_rimuovi, cmd_area>,
      FSM::State
  {
    using handler = forest::command_handler<AreaRegion, FSM, cmd_aggiungi, cmd_rimuovi, cmd_area>;

    void react (cmd_aggiungi, forest::events::command const& event, FSM::FullControl& control)
    {
      auto context = control.context ();

      if (event.parameters.empty ()) {
        context.send_message ("The command needs one parameter. /aggiungiarea nome");
        return;
      }

      if (auto err = centrale::handler_centrale::is_selected (context); err != E_CENTRALE::OK) {
        context.send_message (feedback (err));
        return;
      }

      auto centrale = centrale::handler_centrale::selected (context).value ();
      auto nome_area = event.parameters;

      if (auto err = handler_area::add (context, centrale, {nome_area, false}); err != E_AREA::OK) {
        context.send_message (feedback (err));
        return;
      }

      context.send_message ("Area aggiunta.");
    }

    void react (cmd_rimuovi, forest::events::command const& event, FSM::FullControl& control)
    {
      auto context = control.context ();

      if (event.parameters.empty ()) {
        context.send_message ("Il comando richiede un parametro. /rimuoviarea nome");
        return;
      }

      if (auto err = centrale::handler_centrale::is_selected (context); err != E_CENTRALE::OK) {
        context.send_message (feedback (err));
        return;
      }

      auto centrale = centrale::handler_centrale::selected (context).value ();
      auto nome_area = event.parameters;

      if (auto err = handler_area::remove (context, centrale, {nome_area, false}); err != E_AREA::OK) {
        context.send_message (feedback (err));
        return;
      }

      context.send_message ("Area rimossa.");
    }

    void react (cmd_area, forest::events::command const& event, FSM::FullControl& control)
    {
      auto context = control.context ();

      if (auto err = centrale::handler_centrale::is_selected (context); err != E_CENTRALE::OK) {
        context.send_message (feedback (err));
        return;
      }

      control.changeTo<AreaMenu> ();
    }

    using FSM::State::react;
    using handler::react;
  };

  struct AreaIdle : FSM::State
  {};

  struct button_on : forest::button<button_on>
  {};

  struct button_off : forest::button<button_off>
  {};

  struct button_state : forest::button<button_state>
  {};

  struct button_confirm : forest::button<button_confirm>
  {};

  struct button_cancel : forest::button<button_cancel>
  {};

  struct AreaMenu
    : forest::keyboard_handler<AreaMenu, FSM, button_on, button_off, button_state, button_confirm, button_cancel>,
      FSM::State
  {
    using handler =
      forest::keyboard_handler<AreaMenu, FSM, button_on, button_off, button_state, button_confirm, button_cancel>;

    std::map<std::string, int> _selection;
    std::int64_t _message_id;

    void enter (PlanControl& control)
    {
      auto context = control.context ();
      auto centrale = handler_centrale::selected (context).value ();
      auto aree = handler_area::get (context);
      for (auto [key, area] : aree[centrale])
        _selection[key] = 3;
      print (context);
    }

    void exit (PlanControl& control)
    {
      control.context ().delete_message (_message_id);
    }

    void reenter (PlanControl& control)
    {
      exit (control);
      enter (control);
    }

    auto content (forest::bot_context context) -> std::vector<std::vector<TgBot::InlineKeyboardButton::Ptr>>
    {
      auto centrale = handler_centrale::selected (context).value ();
      auto aree = handler_area::get (context);
      auto markup = std::vector<std::vector<TgBot::InlineKeyboardButton::Ptr>> ();
      markup.push_back (header (context));

      int index = 0;
      for (auto [key, area] : aree[centrale]) {
        using button_type = TgBot::InlineKeyboardButton;
        auto btn_name = button_type {.text = key, .callbackData = "N/A"};
        auto btn_info = button_type {.text = area.armed ? "ON" : "OFF", .callbackData = "N/A"};

        bool is_on_selected = _selection[key] == 1;
        bool is_off_selected = _selection[key] == 2;
        bool is_state_selected = _selection[key] == 3;

        std::string set_on_text = is_on_selected ? "[x]" : "[ ]";
        std::string set_off_text = is_off_selected ? "[x]" : "[ ]";
        std::string state_text = is_state_selected ? "[x]" : "[ ]";

        auto btn_on = button_on::to_bot_button (index, set_on_text, key);
        auto btn_off = button_off::to_bot_button (index, set_off_text, key);
        auto btn_stato = button_state::to_bot_button (index, state_text, key);

        auto row = std::vector<button_type::Ptr> ();
        row.push_back (std::make_shared<button_type> (btn_name));
        row.push_back (std::make_shared<button_type> (btn_info));
        row.push_back (std::make_shared<button_type> (btn_on));
        row.push_back (std::make_shared<button_type> (btn_off));
        row.push_back (std::make_shared<button_type> (btn_stato));
        markup.push_back (row);
      }

      markup.push_back (footer (context));
      return markup;
    }

    auto header (forest::bot_context context) -> std::vector<TgBot::InlineKeyboardButton::Ptr>
    {
      using button_type = TgBot::InlineKeyboardButton;
      auto result = std::vector<button_type::Ptr> ();

      auto btn_nome = button_type {.text = "Nome", .callbackData = "N/A"};
      auto btn_info = button_type {.text = "Info", .callbackData = "N/A"};
      auto btn_on = button_type {.text = "SET ON", .callbackData = "N/A"};
      auto btn_off = button_type {.text = "SET OFF", .callbackData = "N/A"};
      auto btn_stato = button_type {.text = "STATO", .callbackData = "N/A"};
      result.push_back (std::make_shared<button_type> (btn_nome));
      result.push_back (std::make_shared<button_type> (btn_info));
      result.push_back (std::make_shared<button_type> (btn_on));
      result.push_back (std::make_shared<button_type> (btn_off));
      result.push_back (std::make_shared<button_type> (btn_stato));
      return result;
    }

    auto footer (forest::bot_context context) -> std::vector<TgBot::InlineKeyboardButton::Ptr>
    {
      using button_type = TgBot::InlineKeyboardButton;
      auto btn_confirm = button_confirm::to_bot_button ("Confirm", "N/A");
      auto btn_cancel = button_cancel::to_bot_button ("Cancel", "N/A");
      return std::vector {std::make_shared<button_type> (btn_confirm), std::make_shared<button_type> (btn_cancel)};
    }

    void print (forest::bot_context context)
    {
      auto cnt = content (context);
      _message_id = context.send_message ("Seleziona un'azione per una o più aree.", cnt)->messageId;
    }

    void print_edit (forest::bot_context context)
    {
      auto cnt = content (context);
      context.edit_message (_message_id, cnt);
    }

    void react (button_on, forest::events::button const& event, FSM::FullControl& control)
    {
      bool modified = _selection.at (event.payload) != 1;
      _selection.at (event.payload) = 1;
      if (modified)
        print_edit (control.context ());
    }

    void react (button_off, forest::events::button const& event, FSM::FullControl& control)
    {
      bool modified = _selection.at (event.payload) != 2;
      _selection.at (event.payload) = 2;
      if (modified)
        print_edit (control.context ());
    }

    void react (button_state, forest::events::button const& event, FSM::FullControl& control)
    {
      bool modified = _selection.at (event.payload) != 3;
      _selection.at (event.payload) = 3;
      if (modified)
        print_edit (control.context ());
    }

    void react (button_confirm, forest::events::button const& event, FSM::FullControl& control)
    {
      auto context = control.context ();

      if (control.isActive<PinNotOk> ()) {
        context.send_message ("Pin non impostato o scaduto. Imposta il pin prima di confermare l'azione.");
        return;
      }

      std::string response;
      for (auto [area, sel] : _selection) {
        if (sel == 1)
          response += "Area " + area + " will be armed.\n";
        if (sel == 2)
          response += "Area " + area + " will be disarmed.\n";
      }

      if (response.empty ())
        response = "Nessuna azione selezionata.";

      context.send_message (response);
      auto centrale = handler_centrale::selected (context).value ();
      auto aree = handler_area::get (context);

      for (auto [area, sel] : _selection) {
        if (sel == 1)
          aree.at (centrale).at (area).armed = true;
        if (sel == 2)
          aree.at (centrale).at (area).armed = false;
      }

      handler_area::update (context, centrale, aree.at (centrale));

      aree = handler_area::get (context);

      response = "";
      for (auto [name, area] : aree.at (centrale))
        response += area.name + ": " + (area.armed ? "ON" : "OFF") + "\n";

      if (response == "")
        response = "Nessuna area nell'elenco.\n";
      else
        response = "Stato aree:\n" + response;

      context.send_message (response);

      control.changeTo<AreaIdle> ();
    }

    void react (button_cancel, forest::events::button const& event, FSM::FullControl& control)
    {
      control.context ().send_message ("Azione annullata.");
      control.changeTo<AreaIdle> ();
    }

    using FSM::State::react;
    using handler::react;
  };
} // namespace demo::area

namespace demo::eventi
{
  struct cmd_eventi
  {
    static std::string prefix ()
    {
      return "eventi";
    }

    static std::string description ()
    {
      return "Mostra eventi";
    }
  };

  struct EventiRegion : forest::command_handler<EventiRegion, FSM, cmd_eventi>, FSM::State
  {
    using handler = forest::command_handler<EventiRegion, FSM, cmd_eventi>;

    void react (cmd_eventi, forest::events::command const& event, FSM::FullControl& control)
    {
      auto context = control.context ();
      if (auto err = handler_centrale::is_selected (context); err != E_CENTRALE::OK) {
        context.send_message (feedback (err));
        return;
      }

      control.changeTo<EventiMenu> ();
    }

    using FSM::State::react;
    using handler::react;
  };

  struct EventiIdle : FSM::State
  {};

  struct button_prev : forest::button<button_prev>
  {};

  struct button_next : forest::button<button_next>
  {};

  struct EventiMenu : forest::keyboard_handler<EventiMenu, FSM, button_next, button_prev>, FSM::State
  {
    using handler = forest::keyboard_handler<EventiMenu, FSM, button_next, button_prev>;

    std::vector<std::string> _events;
    int _page;
    std::int64_t _messageid;

    void react (button_next, forest::events::button const& event, FSM::FullControl& control)
    {
      _page = std::max (0, _page - 1);
      print_edit (control.context ());
    }

    void react (button_prev, forest::events::button const& event, FSM::FullControl& control)
    {
      _page += 1;
      print_edit (control.context ());
    }

    void enter (PlanControl& control)
    {
      // generate random events
      _events = {};
      _page = 0;

      auto rng = std::mt19937 (std::random_device {}());
      int const num_events = std::uniform_int_distribution<int> (15, 50) (rng);

      _events.assign (num_events, "");
      for (int i = 0; i < num_events; ++i)
        _events[i] = std::to_string (rng () % 100);

      print (control.context ());
    }

    void exit (PlanControl& control)
    {
      control.context ().delete_message (_messageid);
    }

    void reenter (PlanControl& control)
    {
      exit (control);
      enter (control);
    }

    auto footer () -> std::vector<TgBot::InlineKeyboardButton::Ptr>
    {
      using button_type = TgBot::InlineKeyboardButton;
      auto row = std::vector<button_type::Ptr> ();

      auto btn_prev = button_next::to_bot_button ("<", "N/A");
      auto btn_next = button_prev::to_bot_button (">", "N/A");
      row.push_back (std::make_shared<button_type> (btn_prev));
      row.push_back (std::make_shared<button_type> (btn_next));
      return row;
    }

    auto content ()
    {
      int const events_per_page = 10;
      int const start = events_per_page * _page;
      int const end = start + events_per_page;
      std::string cnt = "Lista eventi (" + std::to_string (_page) + ")\n";

      for (int i = start; i < end && i < static_cast<int> (_events.size ()); ++i)
        cnt += ". " + _events[i] + "\n";

      return cnt;
    }

    void print (forest::bot_context context)
    {
      auto markup = std::vector<std::vector<TgBot::InlineKeyboardButton::Ptr>> ();
      markup.push_back (footer ());
      _messageid = context.send_message (content (), markup)->messageId;
    }

    void print_edit (forest::bot_context context)
    {
      auto markup = std::vector<std::vector<TgBot::InlineKeyboardButton::Ptr>> ();
      markup.push_back (footer ());

      try {
        context.edit_message (_messageid, content (), markup);
      } catch (...) {}
    }

    using FSM::State::react;
    using handler::react;
  };
} // namespace demo::eventi

namespace demo::uscita
{
  struct uscita_t
  {
    std::string nome;
    bool stato;

    friend void to_json (nlohmann::json& json, uscita_t const& uscita)
    {
      json = {{"nome", uscita.nome}, {"stato", uscita.stato}};
    }

    friend void from_json (nlohmann::json const& json, uscita_t& uscita)
    {
      json.at ("nome").get_to (uscita.nome);
      json.at ("stato").get_to (uscita.stato);
    }
  };

  using uscita_vector_t = name_map_t<uscita_t, &uscita_t::nome>;

  enum class E_USCITA
  {
    DUPLICATE,
    MISSING,
    OK
  };

  auto feedback (E_USCITA uscita) -> std::string
  {
    if (uscita == E_USCITA::DUPLICATE)
      return "L'uscita è già nell'elenco.";
    if (uscita == E_USCITA::MISSING)
      return "L'uscita non è nell'elenco.";
    return "OK";
  }

  namespace handler_uscita
  {
    auto get (forest::bot_context context) -> std::map<std::string, uscita_vector_t>
    {
      if (auto uscite = context.get_value_json ("elenco_uscite"); uscite)
        return *uscite;
      return {};
    }

    auto update (forest::bot_context context, std::string centrale, uscita_vector_t uscite) -> void
    {
      auto old = get (context);
      old[centrale] = uscite;
      context.set_value ("elenco_uscite", nlohmann::json (old));
    }

    auto add (forest::bot_context context, std::string centrale, uscita_t uscita) -> E_USCITA
    {
      auto uscite = get (context);
      if (uscite[centrale].contains (uscita.nome))
        return E_USCITA::DUPLICATE;
      uscite[centrale][uscita.nome] = uscita;
      update (context, centrale, uscite[centrale]);
      return E_USCITA::OK;
    }

    auto remove (forest::bot_context context, std::string centrale, std::string uscita) -> E_USCITA
    {
      auto uscite = get (context);
      if (uscite[centrale].contains (uscita) == false)
        return E_USCITA::MISSING;
      uscite[centrale].erase (uscita);
      return E_USCITA::OK;
    }

  } // namespace handler_uscita

  struct cmd_uscite
  {
    static std::string prefix ()
    {
      return "uscite";
    }

    static std::string description ()
    {
      return "Mostra uscite";
    }
  };

  struct cmd_add
  {
    static std::string prefix ()
    {
      return "aggiungiuscita";
    }

    static std::string description ()
    {
      return "Aggiungi una uscita";
    }
  };

  struct cmd_remove
  {
    static std::string prefix ()
    {
      return "rimuoviuscita";
    }

    static std::string description ()
    {
      return "Rimuovi una uscita";
    }
  };

  struct UscitaRegion : forest::command_handler<UscitaRegion, FSM, cmd_uscite, cmd_add, cmd_remove>, FSM::State
  {
    using handler = forest::command_handler<UscitaRegion, FSM, cmd_uscite, cmd_add, cmd_remove>;
    using FSM::State::react;
    using handler::react;

    void react (cmd_add, forest::events::command const& event, FSM::FullControl& control)
    {
      auto context = control.context ();

      if (event.parameters.empty ()) {
        context.send_message ("Il comando richiede un parametro. /aggiungiuscita nome");
        return;
      }

      if (auto err = handler_centrale::is_selected (context); err != E_CENTRALE::OK) {
        context.send_message (feedback (err));
        return;
      }

      auto centrale = handler_centrale::selected (context).value ();
      auto nome_uscita = event.parameters;
      if (auto err = handler_uscita::add (context, centrale, {nome_uscita, false}); err != E_USCITA::OK) {
        context.send_message (feedback (err));
        return;
      }

      context.send_message ("Uscita aggiunta.");
    }

    void react (cmd_remove, forest::events::command const& event, FSM::FullControl& control)
    {
      auto context = control.context ();

      if (event.parameters.empty ()) {
        context.send_message ("Il comando richiede un parametro. /rimuoviuscita nome");
        return;
      }

      if (auto err = handler_centrale::is_selected (context); err != E_CENTRALE::OK) {
        context.send_message (feedback (err));
        return;
      }

      auto centrale = handler_centrale::selected (context).value ();
      auto nome_uscita = event.parameters;
      if (auto err = handler_uscita::remove (context, centrale, {nome_uscita, false}); err != E_USCITA::OK) {
        context.send_message (feedback (err));
        return;
      }

      context.send_message ("Uscita rimossa.");
    }

    void react (cmd_uscite, forest::events::command const& event, FSM::FullControl& control)
    {
      auto context = control.context ();
      if (auto err = handler_centrale::is_selected (context); err != E_CENTRALE::OK) {
        context.send_message (feedback (err));
        return;
      }

      control.changeTo<UscitaMenu> ();
    }
  };

  struct button_toggle : forest::button<button_toggle>
  {};

  struct button_cancel : forest::button<button_cancel>
  {};

  struct button_confirm : forest::button<button_confirm>
  {};

  struct UscitaMenu : forest::keyboard_handler<UscitaMenu, FSM, button_toggle, button_cancel, button_confirm>, FSM::State
  {
    using handler = forest::keyboard_handler<UscitaMenu, FSM, button_toggle, button_cancel, button_confirm>;
    using bot_button = TgBot::InlineKeyboardButton;
    using button_row = std::vector<bot_button::Ptr>;
    using button_markup = std::vector<button_row>;

    using FSM::State::react;
    using handler::react;

    uscita_vector_t _uscite_old;
    uscita_vector_t _uscite_new;
    std::int64_t _messageid;

    void react (button_toggle, forest::events::button const& event, FSM::FullControl& control)
    {
      _uscite_new[event.payload].stato ^= 1;
      print_edit (control.context ());
    }

    void react (button_cancel, forest::events::button const& event, FSM::FullControl& control)
    {
      control.context ().send_message ("Azione annullata.");
      control.changeTo<UscitaIdle> ();
    }

    void react (button_confirm, forest::events::button const& event, FSM::FullControl& control)
    {
      auto context = control.context ();
      auto centrale = handler_centrale::selected (context).value ();

      if (control.isActive<PinNotOk> ()) {
        context.send_message ("Pin non impostato o scaduto. Imposta il pin prima di confermare l'azione.");
        return;
      }

      std::string answer;

      for (auto [key, u] : _uscite_new) {
        if (_uscite_old[key].stato != u.stato) {
          answer += key + " will be set " + (u.stato ? "ON" : "OFF");
        }
      }

      if (answer == "")
        answer = "Non è stato richiesto nessun cambiamento.\n";

      control.context ().send_message (answer);
      handler_uscita::update (context, centrale, _uscite_new);
      control.changeTo<UscitaIdle> ();
    }

    void enter (PlanControl& control)
    {
      auto context = control.context ();
      auto centrale = handler_centrale::selected (context).value ();
      _uscite_new = _uscite_old = handler_uscita::get (context)[centrale];
      print (context);
    }

    void exit (PlanControl& control)
    {
      control.context ().delete_message (_messageid);
    }

    void reenter (PlanControl& control)
    {
      exit (control);
      enter (control);
    }

    auto footer () -> button_row
    {
      auto row = button_row ();
      auto btn_confirm = button_confirm::to_bot_button ("Confirm", "N/A");
      auto btn_cancel = button_cancel::to_bot_button ("Cancel", "N/A");
      row.push_back (std::make_shared<bot_button> (btn_confirm));
      row.push_back (std::make_shared<bot_button> (btn_cancel));
      return row;
    }

    auto header () -> button_row
    {
      auto row = button_row ();
      auto btn_nome = bot_button {.text = "Nome", .callbackData = "N/A"};
      auto btn_info = bot_button {.text = "Info", .callbackData = "N/A"};
      auto btn_azione = bot_button {.text = "Azione", .callbackData = "N/A"};
      auto btn_padding = bot_button {.text = " ", .callbackData = "N/A"};
      row.push_back (std::make_shared<bot_button> (btn_nome));
      row.push_back (std::make_shared<bot_button> (btn_info));
      row.push_back (std::make_shared<bot_button> (btn_azione));
      row.push_back (std::make_shared<bot_button> (btn_padding));
      return row;
    }

    auto content () -> button_markup
    {
      auto markup = button_markup ();
      markup.push_back (header ());

      int index = 0;
      for (auto [key, u] : _uscite_old) {
        auto btn_nome = bot_button {.text = key, .callbackData = "N/A"};
        auto btn_info = bot_button {.text = u.stato ? "ON" : "OFF", .callbackData = "N/A"};

        auto changed = _uscite_new[key].stato != _uscite_old[key].stato;
        auto new_state = _uscite_new[key].stato;
        std::string azione = changed ? (new_state ? "ON" : "OFF") : " ";
        auto btn_azione = bot_button {.text = azione, .callbackData = "N/A"};

        auto btn_toggle = button_toggle::to_bot_button (index++, "Cambia", key);

        auto row = button_row ();
        row.push_back (std::make_shared<bot_button> (btn_nome));
        row.push_back (std::make_shared<bot_button> (btn_info));
        row.push_back (std::make_shared<bot_button> (btn_azione));
        row.push_back (std::make_shared<bot_button> (btn_toggle));
        markup.push_back (row);
      }

      markup.push_back (footer ());
      return markup;
    }

    void print (forest::bot_context context)
    {
      auto cnt = content ();
      _messageid = context.send_message ("Scegli una o più azioni", cnt)->messageId;
    }

    void print_edit (forest::bot_context context)
    {
      auto cnt = content ();
      context.edit_message (_messageid, cnt);
    }
  };

  struct UscitaIdle : FSM::State
  {};
} // namespace demo::uscita

namespace nlohmann
{
  template<typename Clock, typename Duration>
  struct adl_serializer<std::chrono::time_point<Clock, Duration>>
  {
    static void to_json (json& j, const std::chrono::time_point<Clock, Duration>& tp)
    {
      j["since_epoch"] = std::chrono::duration_cast<std::chrono::seconds> (tp.time_since_epoch ()).count ();
    }

    static void from_json (json const& j, std::chrono::time_point<Clock, Duration>& p)
    {
      long long duration = j.at ("since_epoch");
      p = std::chrono::time_point<Clock, Duration> (std::chrono::seconds (duration));
    }
  };
} // namespace nlohmann

namespace demo::pin
{
  struct pin_t
  {
    std::string pin;
    std::chrono::steady_clock::time_point expire;

    friend void from_json (nlohmann::json const& json, pin_t& pin)
    {
      json.at ("pin").get_to (pin.pin);
      json.at ("expire").get_to (pin.expire);
    }

    friend void to_json (nlohmann::json& json, pin_t const& pin)
    {
      json["pin"] = pin.pin;
      json["expire"] = pin.expire;
    }
  };

  struct cmd_setpin
  {
    static std::string prefix ()
    {
      return "setpin";
    }

    static std::string description ()
    {
      return "Imposta il pin";
    }
  };

  struct cmd_checkpin
  {
    static std::string prefix ()
    {
      return "checkpin";
    }

    static std::string description ()
    {
      return "Check pin expire time.";
    }
  };

  struct PinRegion : forest::command_handler<PinRegion, FSM, cmd_setpin, cmd_checkpin>, FSM::State
  {
    using handler = forest::command_handler<PinRegion, FSM, cmd_setpin, cmd_checkpin>;
    using FSM::State::react;
    using handler::react;

    void react (cmd_setpin, forest::events::command const& event, FSM::FullControl& control)
    {
      auto context = control.context ();

      if (event.parameters.empty ()) {
        context.send_message ("Il comando richiede un parametro. /setpin pin");
        return;
      }

      auto expire = std::chrono::steady_clock::now () + std::chrono::seconds (30);
      auto pin = pin_t {.pin = event.parameters, .expire = expire};
      context.set_value ("pin", nlohmann::json (pin));
      context.send_message ("Pin impostato.");
      context.delete_message (event.id);

      control.changeTo<PinOk> ();
    }

    void react (cmd_checkpin, forest::events::command const& event, FSM::FullControl& control)
    {
      auto context = control.context ();
      if (control.isActive<PinNotOk> ()) {
        context.send_message ("Pin expired.");
      }
    }
  };

  struct PinOk : forest::command_handler<PinOk, FSM, cmd_checkpin>, FSM::State
  {
    using handler = forest::command_handler<PinOk, FSM, cmd_checkpin>;
    using FSM::State::react;
    using handler::react;

    std::chrono::steady_clock::time_point expire;

    void enter (PlanControl& control)
    {
      expire = std::chrono::steady_clock::now () + std::chrono::seconds (30);
    }

    void update (FSM::FullControl& control)
    {
      if (std::chrono::steady_clock::now () >= expire) {
        control.changeTo<PinNotOk> ();
      }
    }

    void react (cmd_checkpin, forest::events::command const& event, FSM::FullControl& control)
    {
      auto context = control.context ();
      auto diff = expire - std::chrono::steady_clock::now ();
      auto count = std::chrono::duration_cast<std::chrono::seconds> (diff).count ();
      context.send_message ("Pin expires in " + std::to_string (count) + " seconds.");
    }
  };

  struct PinNotOk : FSM::State
  {};

} // namespace demo::pin

namespace demo::notifiche
{
  namespace notifiche_handler
  {
    auto get (forest::bot_context context) -> std::map<std::string, bool>
    {
      auto json = context.get_value_json ("notifiche");
      if (json)
        return json.value ();
      return {};
    }

    auto abilita (forest::bot_context context, std::string centrale) -> void
    {
      auto map = get (context);
      map[centrale] = true;
      context.set_value ("notifiche", nlohmann::json (map));
    }

    auto disabilita (forest::bot_context context, std::string centrale) -> void
    {
      auto map = get (context);
      map[centrale] = false;
      context.set_value ("notifiche", nlohmann::json (map));
    }
  } // namespace notifiche_handler

  struct cmd_abilita
  {
    static std::string prefix ()
    {
      return "abilitanotifiche";
    }

    static std::string description ()
    {
      return "Abilita notifiche";
    }
  };

  struct cmd_disabilita
  {
    static std::string prefix ()
    {
      return "disabilitanotifiche";
    }

    static std::string description ()
    {
      return "Disabilita notifiche";
    }
  };

  struct notification
  {
    std::string centrale;
    std::string text;
  };

  struct NotificheRegion : forest::command_handler<NotificheRegion, FSM, cmd_abilita, cmd_disabilita>, FSM::State
  {
    using handler = forest::command_handler<NotificheRegion, FSM, cmd_abilita, cmd_disabilita>;
    using FSM::State::react;
    using handler::react;

    void react (cmd_abilita, forest::events::command const& event, FSM::FullControl& control)
    {
      auto context = control.context ();
      if (auto err = handler_centrale::is_selected (context); err != E_CENTRALE::OK) {
        context.send_message (feedback (err));
        return;
      }

      auto centrale = handler_centrale::selected (context).value ();
      notifiche_handler::abilita (context, centrale);
      context.send_message ("Notifiche abilitate.");
    }

    void react (cmd_disabilita, forest::events::command const& event, FSM::FullControl& control)
    {
      auto context = control.context ();
      if (auto err = handler_centrale::is_selected (context); err != E_CENTRALE::OK) {
        context.send_message (feedback (err));
        return;
      }

      auto centrale = handler_centrale::selected (context).value ();
      notifiche_handler::disabilita (context, centrale);
      context.send_message ("Notifiche disabilitate.");
    }

    void react (notification const& event, FSM::FullControl& control)
    {
      auto context = control.context ();
      auto notifiche = notifiche_handler::get (context);
      auto centrale = event.centrale;
      if (notifiche[centrale]) {
        auto msg = std::string ("Notifica centrale ") + centrale + "\n";
        msg += event.text;
        context.send_message (msg);
      }
    }
  };

  struct NotificheIdle : FSM::State
  {};
} // namespace demo::notifiche

bool signal_happened = false;

void signal_handler (int sig)
{
  std::cerr << "Handler attivato" << std::endl;
  signal_happened = true;
}

int main ()
{
  std::signal (SIGTSTP, signal_handler);

  auto bot = forest::bot<demo::FSM> ("5193745507:AAHAzxtf4jXLZGDIisk0q2HmVcakkfer_7w", "db01.db");
  auto long_poll = forest::long_poll_bot (bot);

  try {
    while (true) {
      long_poll.start ();
      if (signal_happened) {
        signal_happened = false;
        for (auto id : bot.chat_ids ())
          bot.process_event (id, demo::notification {.centrale = "A1", .text = "hello world"});
      }
    }
  } catch (std::exception& e) {
    std::cerr << "exception: " << typeid (e).name () << "\n";
    std::cerr << "what: " << e.what () << "\n";
  }
}
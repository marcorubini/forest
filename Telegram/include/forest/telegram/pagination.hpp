#pragma once
#include <forest/telegram/button.hpp>

namespace forest::telegram
{
  template<class Derived, class Machine>
  struct pagination
  {
  private:
    struct button_prev : button<button_prev>
    {};
    struct button_next : button<button_next>
    {};

    using exact_context = typename Machine::template exact_context<Derived>;
    using transit_none = typename Machine::transit_none;

    template<class... Alternatives>
    using transit_result = typename Machine::template transit_result<Alternatives...>;

    std::int64_t message_id = -1;
    int page = 0;

  public:
    using my_buttons = button_list<button_prev, button_next>;

    template<std::same_as<Derived> D = Derived>
    void enter (exact_context const& ctx)
    {
      _print (ctx);
    }

    template<std::same_as<Derived> D = Derived>
    void exit (exact_context const& ctx)
    {
      ctx.delete_message (message_id);
      message_id = -1;
      page = 0;
    }

    template<std::same_as<Derived> D = Derived>
    void reenter (exact_context const& ctx)
    {
      _print (ctx);
    }

    auto react (exact_context const& ctx, button_event<button_prev> ev) -> transit_result<>
    {
      page -= 1;
      _print (ctx);
      return transit_none {};
    }

    auto react (exact_context const& ctx, button_event<button_next> ev) -> transit_result<>
    {
      page += 1;
      _print (ctx);
      return transit_none {};
    }

  private:
    std::vector<button_description> _pager ()
    {
      button_description btn_prev = button_prev {.text = '<', .payload = "N/A"};
      button_description btn_next = button_next {.text = '>', .payload = "N/A"};
      return {btn_prev, btn_next};
    }

    template<std::same_as<Derived> D>
    void _print (exact_context const& ctx)
    {
      D& self = static_cast<D&> (*this);

      std::string text = self.message ();
      std::vector<button_description> header = self.header ();
      std::vector<std::vector<button_description>> content = self.content ();
      std::vector<button_description> footer = self.footer ();

      std::vector<std::vector<button_description>> table;
      table.emplace_back (std::move (header));

      int num_rows = content.size ();
      int rows_per_page = self.page_size ();
      if (rows_per_page <= 0)
        rows_per_page = 1;
      int num_pages = num_rows / rows_per_page;

      int start = rows_per_page * page;
      for (int i = 0; i < rows_per_page && i + start < num_rows; ++i)
        table.emplace_back (std::move (content[i]));

      table.emplace_back (std::move (footer));
      table.emplace_back (_pager ());

      if (message_id == -1) {
        message_id = ctx.get ().send_message (text, table);
      } else {
        ctx.get ().seid_message (message_id, text, table);
      }
    }
  };

} // namespace forest::telegram
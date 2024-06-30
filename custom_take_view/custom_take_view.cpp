#include <iostream>
#include <ranges>
#include <vector>

template <std::ranges::view R>
class custom_take_view
    : public std::ranges::view_interface<custom_take_view<R>>
{
public:
    custom_take_view() = default;

    constexpr custom_take_view(R base, std::ranges::range_difference_t<R> count)
        : base_{ std::move(base) }
        , count_{ count }
    {

    }

    constexpr R base() const&
    {
        return base_;
    }

    constexpr R base()&&
    {
        return std::move(base_);
    }

    constexpr auto begin()
    {
        return std::ranges::begin(base_);
    }

    constexpr auto end()
    {
        return std::ranges::next(std::ranges::begin(base_), count_);
    }

private:
    R base_{};
    std::ranges::range_difference_t<R> count_{};
};

template <std::ranges::range R>
custom_take_view(R&&, std::ranges::range_difference_t<R>) -> custom_take_view<std::ranges::views::all_t<R>>;

template <std::integral T>
struct custom_take_range_adaptor_closure
{
    T count;

    constexpr custom_take_range_adaptor_closure(T count)
        : count{ count }
    {

    }

    template <std::ranges::viewable_range R>
    constexpr auto operator()(R&& r) const
    {
        return custom_take_view(std::forward<R>(r), count);
    }
};

struct custom_take_range_adaptor
{
    template <typename... Args>
    constexpr auto operator()(Args&&... args)
    {
        if constexpr (sizeof...(Args) == 1)
        {
            return custom_take_range_adaptor_closure{ args... };
        }
        else
        {
            return custom_take_view{ std::forward<Args>(args)... };
        }
    }
};

template <std::ranges::viewable_range R, std::invocable<R> Adaptor>
constexpr auto operator|(R&& r, Adaptor const& a)
{
    return a(std::forward<R>(r));
}

namespace views
{
    inline custom_take_range_adaptor custom_take;
}

int main()
{
    auto n{ std::views::iota(2, 10) | std::ranges::to<std::vector>() };
    auto v{ n | std::views::filter([](int e) { return e % 2 == 0; }) | views::custom_take(2) };

    std::ranges::copy(v, std::ostream_iterator<int>(std::cout, " "));
}

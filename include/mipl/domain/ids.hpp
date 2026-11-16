#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace mipl::domain {

struct GuildId {
    std::uint64_t value{};

    bool operator==(const GuildId&) const = default;
};

struct ChannelId {
    std::uint64_t value{};

    bool operator==(const ChannelId&) const = default;
};

struct UserId {
    std::uint64_t value{};

    bool operator==(const UserId&) const = default;
};

struct RoleId {
    std::uint64_t value{};

    bool operator==(const RoleId&) const = default;
};

struct GuildIdHash {
    std::size_t operator()(GuildId guild_id) const noexcept
    {
        return std::hash<std::uint64_t>{}(guild_id.value);
    }
};

} // namespace mipl::domain

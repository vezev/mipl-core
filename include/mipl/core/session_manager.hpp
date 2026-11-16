#pragma once

#include <unordered_map>

#include "mipl/domain/session.hpp"

namespace mipl::core {

class SessionManager {
public:
    [[nodiscard]]
    domain::SessionResult create(
        domain::GuildId guild_id,
        domain::ChannelId text_channel_id,
        domain::ChannelId voice_channel_id,
        domain::ModuleId module_id,
        domain::UserId host_id
    );

    [[nodiscard]]
    const domain::Session* find_open(
        domain::GuildId guild_id
    ) const noexcept;

    [[nodiscard]]
    domain::SessionResult mark_active(
        domain::GuildId guild_id,
        const domain::ModuleId& expected_module
    );

    [[nodiscard]]
    domain::SessionResult force_close(
        domain::GuildId guild_id
    );

    [[nodiscard]]
    domain::SessionResult cancel_before_start(
        domain::GuildId guild_id,
        const domain::ModuleId& expected_module,
        domain::UserId requester_id
    );

private:
    std::unordered_map<
        domain::GuildId,
        domain::Session,
        domain::GuildIdHash
    > open_sessions_by_guild_;
};

} // namespace mipl::core

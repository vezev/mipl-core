#include "mipl/core/session_manager.hpp"

#include <chrono>
#include <utility>

namespace mipl::core {

domain::SessionResult SessionManager::create(
    domain::GuildId guild_id,
    domain::ChannelId text_channel_id,
    domain::ChannelId voice_channel_id,
    domain::ModuleId module_id,
    domain::UserId host_id
)
{
    if (open_sessions_by_guild_.contains(guild_id)) {
        return domain::SessionResult{
            false,
            domain::SessionError::GuildAlreadyOccupied,
            std::nullopt
        };
    }

    domain::Session session{
        guild_id,
        text_channel_id,
        voice_channel_id,
        std::move(module_id),
        host_id,
        domain::SessionState::Preparing,
        std::chrono::system_clock::now()
    };

    const auto [iterator, inserted] =
        open_sessions_by_guild_.emplace(guild_id, session);

    if (!inserted) {
        return domain::SessionResult{
            false,
            domain::SessionError::GuildAlreadyOccupied,
            std::nullopt
        };
    }

    return domain::SessionResult{
        true,
        domain::SessionError::None,
        iterator->second
    };
}

const domain::Session* SessionManager::find_open(
    domain::GuildId guild_id
) const noexcept
{
    const auto session = open_sessions_by_guild_.find(guild_id);

    if (session == open_sessions_by_guild_.end()) {
        return nullptr;
    }

    return &session->second;
}

domain::SessionResult SessionManager::mark_active(
    domain::GuildId guild_id,
    const domain::ModuleId& expected_module
)
{
    auto session = open_sessions_by_guild_.find(guild_id);

    if (session == open_sessions_by_guild_.end()) {
        return domain::SessionResult{
            false,
            domain::SessionError::NoOpenSession,
            std::nullopt
        };
    }

    if (session->second.module_id != expected_module) {
        return domain::SessionResult{
            false,
            domain::SessionError::ModuleMismatch,
            std::nullopt
        };
    }

    if (session->second.state != domain::SessionState::Preparing) {
        return domain::SessionResult{
            false,
            domain::SessionError::SessionAlreadyActive,
            std::nullopt
        };
    }

    session->second.state = domain::SessionState::Active;

    return domain::SessionResult{
        true,
        domain::SessionError::None,
        session->second
    };
}

domain::SessionResult SessionManager::force_close(
    domain::GuildId guild_id
)
{
    auto session = open_sessions_by_guild_.find(guild_id);

    if (session == open_sessions_by_guild_.end()) {
        return domain::SessionResult{
            false,
            domain::SessionError::NoOpenSession,
            std::nullopt
        };
    }

    const domain::Session closed_session = session->second;
    open_sessions_by_guild_.erase(session);

    return domain::SessionResult{
        true,
        domain::SessionError::None,
        closed_session
    };
}

domain::SessionResult SessionManager::cancel_before_start(
    domain::GuildId guild_id,
    const domain::ModuleId& expected_module,
    domain::UserId requester_id
)
{
    auto session = open_sessions_by_guild_.find(guild_id);

    if (session == open_sessions_by_guild_.end()) {
        return domain::SessionResult{
            false,
            domain::SessionError::NoOpenSession,
            std::nullopt
        };
    }

    if (session->second.module_id != expected_module) {
        return domain::SessionResult{
            false,
            domain::SessionError::ModuleMismatch,
            std::nullopt
        };
    }

    if (session->second.state != domain::SessionState::Preparing) {
        return domain::SessionResult{
            false,
            domain::SessionError::SessionNotPreparing,
            std::nullopt
        };
    }

    if (session->second.host_id != requester_id) {
        return domain::SessionResult{
            false,
            domain::SessionError::RequesterIsNotHost,
            std::nullopt
        };
    }

    const domain::Session closed_session = session->second;
    open_sessions_by_guild_.erase(session);

    return domain::SessionResult{
        true,
        domain::SessionError::None,
        closed_session
    };
}

} // namespace mipl::core

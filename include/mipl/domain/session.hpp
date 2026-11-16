#pragma once

#include <chrono>
#include <optional>

#include "mipl/domain/ids.hpp"
#include "mipl/domain/module.hpp"

namespace mipl::domain {

enum class SessionState {
    Preparing,
    Active
};

struct Session {
    GuildId guild_id;
    ChannelId text_channel_id;
    ChannelId voice_channel_id;
    ModuleId module_id;
    UserId host_id;
    SessionState state{SessionState::Preparing};
    std::chrono::system_clock::time_point created_at;
};

enum class SessionError {
    None,
    GuildAlreadyOccupied,
    NoOpenSession,
    ModuleMismatch,
    SessionAlreadyActive,
    SessionNotPreparing,
    RequesterIsNotHost
};

struct SessionResult {
    bool success{false};
    SessionError error{SessionError::None};
    std::optional<Session> session;

    [[nodiscard]]
    explicit operator bool() const noexcept
    {
        return success;
    }
};

} // namespace mipl::domain

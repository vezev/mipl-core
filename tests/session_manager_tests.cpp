#include <iostream>
#include <string>

#include "mipl/core/session_manager.hpp"

namespace {

int failures = 0;

void expect(bool condition, const std::string& description)
{
    if (condition) {
        std::cout << "[OK]   " << description << '\n';
        return;
    }

    std::cerr << "[FAIL] " << description << '\n';
    ++failures;
}

constexpr mipl::domain::GuildId guild_a{100};
constexpr mipl::domain::GuildId guild_b{200};
constexpr mipl::domain::ChannelId text_a{101};
constexpr mipl::domain::ChannelId voice_a{102};
constexpr mipl::domain::ChannelId text_b{201};
constexpr mipl::domain::ChannelId voice_b{202};
constexpr mipl::domain::UserId host{1000};
constexpr mipl::domain::UserId other_user{2000};

const mipl::domain::ModuleId dummy{"dummy"};
const mipl::domain::ModuleId impostor{"impostor"};

void test_create_preparing_session()
{
    mipl::core::SessionManager manager;

    const auto result = manager.create(
        guild_a,
        text_a,
        voice_a,
        dummy,
        host
    );

    expect(result.success, "crear sesion en servidor libre esta permitido");
    expect(result.session.has_value(), "la creacion devuelve la sesion creada");
    expect(
        result.session.has_value() &&
        result.session->state == mipl::domain::SessionState::Preparing,
        "una sesion nueva nace en estado Preparing"
    );
    expect(
        result.session.has_value() &&
        result.session->host_id == host,
        "la sesion conserva el host creador"
    );
    expect(
        manager.find_open(guild_a) != nullptr,
        "la sesion queda abierta en el servidor"
    );
}

void test_one_session_per_guild()
{
    mipl::core::SessionManager manager;

    const auto first = manager.create(
        guild_a,
        text_a,
        voice_a,
        dummy,
        host
    );

    const auto duplicate = manager.create(
        guild_a,
        text_a,
        voice_a,
        impostor,
        other_user
    );

    expect(first.success, "la primera sesion del servidor se crea");
    expect(!duplicate.success, "una segunda sesion en el mismo servidor se rechaza");
    expect(
        duplicate.error == mipl::domain::SessionError::GuildAlreadyOccupied,
        "el rechazo informa GuildAlreadyOccupied"
    );

    const auto independent = manager.create(
        guild_b,
        text_b,
        voice_b,
        dummy,
        other_user
    );

    expect(
        independent.success,
        "otro servidor puede tener su propia sesion independiente"
    );
}

void test_activate_correct_module()
{
    mipl::core::SessionManager manager;

    const auto created = manager.create(
        guild_a,
        text_a,
        voice_a,
        dummy,
        host
    );

    expect(
        created.success,
        "la sesion de preparacion para activar fue creada"
    );

    const auto wrong_module = manager.mark_active(guild_a, impostor);

    expect(
        !wrong_module.success &&
        wrong_module.error == mipl::domain::SessionError::ModuleMismatch,
        "un modulo distinto no puede activar la sesion dummy"
    );
    expect(
        manager.find_open(guild_a)->state ==
        mipl::domain::SessionState::Preparing,
        "un intento de modulo incorrecto no cambia el estado"
    );

    const auto activated = manager.mark_active(guild_a, dummy);

    expect(activated.success, "el modulo correcto puede activar la sesion");
    expect(
        activated.session.has_value() &&
        activated.session->state == mipl::domain::SessionState::Active,
        "la activacion cambia el estado a Active"
    );

    const auto repeated_activation = manager.mark_active(guild_a, dummy);

    expect(
        !repeated_activation.success &&
        repeated_activation.error ==
        mipl::domain::SessionError::SessionAlreadyActive,
        "una sesion Active no puede iniciarse dos veces"
    );
}

void test_cancel_preparing_session_only_by_host()
{
    mipl::core::SessionManager manager;

    const auto created = manager.create(
        guild_a,
        text_a,
        voice_a,
        dummy,
        host
    );

    expect(
        created.success,
        "la sesion de preparacion para cancelar fue creada"
    );

    const auto unauthorized_cancel = manager.cancel_before_start(
        guild_a,
        dummy,
        other_user
    );

    expect(
        !unauthorized_cancel.success &&
        unauthorized_cancel.error ==
        mipl::domain::SessionError::RequesterIsNotHost,
        "un usuario distinto del host no cancela la preparacion"
    );
    expect(
        manager.find_open(guild_a) != nullptr,
        "la sesion sigue abierta tras una cancelacion no autorizada"
    );

    const auto host_cancel = manager.cancel_before_start(
        guild_a,
        dummy,
        host
    );

    expect(host_cancel.success, "el host puede cancelar durante Preparing");
    expect(
        manager.find_open(guild_a) == nullptr,
        "cancelar en Preparing libera el servidor"
    );

    const auto recreate = manager.create(
        guild_a,
        text_a,
        voice_a,
        dummy,
        host
    );

    expect(
        recreate.success,
        "el servidor liberado admite una nueva sesion"
    );
}

void test_force_close_removes_preparing_and_active_sessions()
{
    mipl::core::SessionManager manager;

    const auto preparing = manager.create(
        guild_a,
        text_a,
        voice_a,
        dummy,
        host
    );

    expect(preparing.success, "precondicion: sesion Preparing creada para force_close");

    const auto close_preparing = manager.force_close(guild_a);

    expect(close_preparing.success, "force_close elimina una sesion Preparing");
    expect(
        manager.find_open(guild_a) == nullptr,
        "force_close libera el servidor desde Preparing"
    );

    const auto active_candidate = manager.create(
        guild_a,
        text_a,
        voice_a,
        dummy,
        host
    );

    expect(active_candidate.success, "precondicion: segunda sesion creada para activar");

    const auto activated = manager.mark_active(guild_a, dummy);

    expect(activated.success, "precondicion: sesion cambiada a Active");

    const auto close_active = manager.force_close(guild_a);

    expect(close_active.success, "force_close elimina una sesion Active");
    expect(
        manager.find_open(guild_a) == nullptr,
        "force_close libera el servidor desde Active"
    );

    const auto close_missing = manager.force_close(guild_a);

    expect(
        !close_missing.success &&
        close_missing.error == mipl::domain::SessionError::NoOpenSession,
        "force_close informa cuando no existe sesion local"
    );
}

void test_active_session_is_not_preparation_cancelable()
{
    mipl::core::SessionManager manager;

    const auto created = manager.create(
        guild_a,
        text_a,
        voice_a,
        dummy,
        host
    );

    expect(
        created.success,
        "la sesion destinada a estado Active fue creada"
    );

    const auto activated = manager.mark_active(guild_a, dummy);

    expect(
        activated.success,
        "la sesion destinada a prueba de cancelacion paso a Active"
    );

    const auto cancel_active = manager.cancel_before_start(
        guild_a,
        dummy,
        host
    );

    expect(
        !cancel_active.success &&
        cancel_active.error ==
        mipl::domain::SessionError::SessionNotPreparing,
        "una sesion Active no se cancela mediante cancel_before_start"
    );
    expect(
        manager.find_open(guild_a) != nullptr,
        "la sesion Active permanece abierta tras ese rechazo"
    );
}

} // namespace

int main()
{
    std::cout << "=== Pruebas de SessionManager ===\n";

    test_create_preparing_session();
    test_one_session_per_guild();
    test_activate_correct_module();
    test_cancel_preparing_session_only_by_host();
    test_force_close_removes_preparing_and_active_sessions();
    test_active_session_is_not_preparation_cancelable();

    std::cout << '\n';

    if (failures == 0) {
        std::cout << "RESULTADO: todas las pruebas pasaron.\n";
        return 0;
    }

    std::cerr
        << "RESULTADO: "
        << failures
        << " prueba(s) fallaron.\n";

    return 1;
}

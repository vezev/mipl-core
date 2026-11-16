# mipl-core — gestión de sesiones

Extracto del núcleo de **Mipl**, un bot de Discord para minijuegos que estoy desarrollando en C++20.
El proyecto completo es privado; acá está la parte que maneja las partidas.

## Qué hace

Cada servidor de Discord puede tener **una sola partida abierta**. Una partida pasa por estos estados:

```
            create                mark_active
 (nada) ───────────► Preparing ───────────────► Active
              ▲          │                        │
              └──────────┤ cancel_before_start    │
              │          │ (solo el host)         │
              └──────────┴──── force_close ───────┘
```

- `create`: abre una partida si el servidor está libre.
- `mark_active`: la arranca (solo si está en preparación).
- `cancel_before_start`: la cancela, solo el que la creó y solo antes de empezar.
- `force_close`: la cierra en cualquier estado. Quién puede usarlo se decide en otra parte del bot.

## Decisiones de diseño

- **IDs tipados** (`GuildId`, `ChannelId`, `UserId`): todos los IDs de Discord son números de 64 bits;
  envolverlos en tipos distintos hace que el compilador no deje mezclarlos.
- **Sin excepciones para errores esperados**: cada operación devuelve un `SessionResult`
  con el resultado y, si falla, el motivo (`GuildAlreadyOccupied`, `RequesterIsNotHost`, ...).
- **Sin dependencia de Discord**: esta parte es C++ puro, así que se puede probar sin conectarse a nada.

## Estructura

```
include/mipl/domain/   ids.hpp, module.hpp, session.hpp   → tipos
include/mipl/core/     session_manager.hpp                → interfaz
src/core/              session_manager.cpp                → lógica
tests/                 session_manager_tests.cpp          → pruebas
```

## Compilar y probar

Requiere CMake 3.20+ y un compilador con C++20 (probado con GCC).

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Estado

En desarrollo. Las sesiones viven en memoria (se pierden al reiniciar).

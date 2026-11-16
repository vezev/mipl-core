## Qué hace

Cada servidor de Discord puede tener una sola partida abierta, que pasa por dos estados: `Preparing` y `Active`.

- `create`: abre una partida si el servidor está libre. Arranca en `Preparing`.
- `mark_active`: pasa la partida a `Active`. Solo funciona si estaba en `Preparing`.
- `cancel_before_start`: cancela la partida, pero solo el que la creó y solo antes de empezar.
- `force_close`: cierra la partida en cualquier estado. Quién puede usarlo se decide en otra parte del bot.

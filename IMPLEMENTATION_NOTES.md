# Implementación de control de niveles y Nivel 2

## Alcance

Se añadió un menú Qt para escoger entre los dos niveles. El Nivel 2 es una escena lateral jugable con movimiento, combate y el encuentro contra Silbón y Tureco. El Nivel 1 abre una pantalla informativa porque en el código recibido solo estaban sus componentes de física/agente; no había una escena de juego Qt que se pudiera iniciar.

## Archivos creados

| Archivo | Clase/función | Motivo |
|---|---|---|
| `GameplayConfig.h` | `GameplayConfig` | Reunir en un solo sitio los valores ajustables del Nivel 2. |
| `BoxAttack.h/.cpp` | `BoxAttack` | Implementar el ataque melee en un área rectangular delante del personaje. |
| `BoneAttack.h/.cpp` | `BoneAttack` | Implementar el ataque a distancia con proyectil de hueso y gravedad. |
| `Silbon.h/.cpp` | `Silbon` | Crear el jefe con persecución, melee, lanzamiento de hueso y patada a Tureco. |
| `Tureco.h/.cpp` | `Tureco` | Crear al compañero de Silbón, su trayectoria de ida y regreso y sus colisiones con el jugador. |
| `Level2GameWidget.h/.cpp` | `Level2GameWidget` | Renderizar el nivel con Qt, procesar teclado y actualizar la partida. |
| `LevelManager.h/.cpp` | `LevelManager`, `GameLevel` | Mostrar el menú y abrir la vista del nivel elegido. |
| `game_assets.qrc` | Recursos Qt | Empaquetar las imágenes para que estén disponibles al ejecutar desde Qt Creator. |
| `assets/player_spritesheet.png` | Sprite sheet del jugador | Recurso gráfico recibido para el personaje del jugador. |
| `assets/level2_background.png` | Fondo del Nivel 2 | Fondo lateral recibido. |

## Archivos modificados

| Archivo | Cambio | Motivo |
|---|---|---|
| `CMakeLists.txt` | Añadí los archivos del menú, nivel, ataques y recursos al ejecutable Qt. | Integrar el código nuevo en la compilación tanto con Qt 5 como con Qt 6. |
| `mainwindow.cpp` | La ventana principal ahora instala `LevelManager` y abre a 1280 × 720. | Arrancar directamente en el selector de niveles. |
| `Projectile.cpp` | Los proyectiles ignoran al jugador mientras está rodando. | Permitir que el roll atraviese huesos sin recibir ni bloquear daño. |
| `BoxAttack.h/.cpp` | Reemplacé la implementación anterior que estaba guardada bajo el nombre equivocado de `BoneAttack`. | Separar correctamente el ataque melee rectangular. |
| `BoneAttack.h/.cpp` | Activé y organicé el ataque de proyectil en los archivos con su nombre correcto. | Poder disparar los huesos parabólicos desde Silbón. |

## Controles

- `A` / `D`: moverse a izquierda / derecha.
- `Espacio`: saltar.
- `L`: atacar.
- `K`: iniciar parry.
- `I`: rodar en la dirección que mira el jugador.
- `Esc`: volver al menú.
- Al terminar la partida, `Enter` reinicia el nivel.

El jugador puede atravesar a Tureco durante el roll, y los proyectiles pasan de largo. Los ataques del jugador se desactivan durante el roll mediante el control de acción ya presente en `Player`.

## Valores ajustables

Todos estos valores están en `GameplayConfig.h`:

| Parámetro | Valor inicial |
|---|---:|
| Vida del jugador | 100 |
| Vida de Silbón | 180 |
| Vida de Tureco | 70 |
| Cooldown melee de Silbón | 1.15 s |
| Cooldown del hueso | 6.4 s |
| Cooldown de patada a Tureco | 8.0 s |
| Tiempo que Tureco permanece pegado al volver | 1.8 s |
| Separación mínima entre hueso y patada | 2.5 s |
| Velocidad inicial de la patada | 560 px/s |
| Aceleración de regreso | 720 px/s² |
| Fricción del movimiento de Tureco | 8.0 |
| Daño melee del jugador | 24 |
| Daño melee de Silbón | 12 |
| Daño del hueso | 18 |
| Daño de Tureco | 16 |
| Duración del roll | 0.32 s |
| Distancia del roll | 180 px |
| Enfriamiento del roll | 0.85 s |
| Ventana activa del parry | 0.28 s |
| Peso de Silbón para la colisión inelástica con Tureco | 10000 |
| Altura de la caja de Silbón | 184 px (el doble de 92 px) |

También son ajustables allí la gravedad, altura del suelo, velocidades, tamaño/alcance de ataques, recuperación del parry y duración del aturdimiento.

## Supuestos

1. La escena lateral usa coordenadas de 1280 × 720 y una línea de suelo fija; la ventana inicial se configura con esas dimensiones.
2. El sprite sheet recibido se usa como imagen de personaje lateral estática, tomando una celda de 320 × 256. No se asumieron filas de animación sin información sobre su distribución.
3. Silbón usa `BoneAttack` y `BoxAttack`; Tureco usa el melee `BoxAttack` al chocar con el jugador. No le asigné un ataque de hueso porque no aparece en su comportamiento descrito.
4. El choque jugador–Tureco es elástico si el jugador bloquea de frente: Tureco invierte su velocidad y vuelve hacia Silbón sin perder rapidez. Si el bloqueo apunta al lado contrario, Tureco pasa sin hacer daño.
5. El hueso calcula una trayectoria parabólica para interceptar la posición prevista del jugador según su movimiento actual.
6. Silbón patea a Tureco en dirección al jugador. Cuando Tureco vuelve, permanece pegado 1.8 s; durante ese tiempo Silbón reanuda su rutina normal. La colisión Tureco–Silbón es inelástica; el peso de Silbón es muy alto para que Tureco se detenga casi por completo.
7. Un golpe del jugador que daña a Silbón también resta la misma cantidad a Tureco mientras permanece pegado. Si Tureco está separado, se resuelve como objetivo independiente.
8. La muerte de Tureco o del jugador termina en derrota; la muerte de Silbón da la victoria, incluso si ambos enemigos reciben el último golpe en el mismo ataque.

## Revisión

La configuración y compilación finalizaron correctamente con Qt 6.11.2, MinGW 13.1.0 y Ninja. No se hizo una prueba interactiva de juego. Durante la primera ejecución de MOC apareció un aviso no fatal del servicio de licencias Qt; la compilación y el enlace del ejecutable terminaron correctamente. Los recursos gráficos están incluidos mediante `game_assets.qrc`.

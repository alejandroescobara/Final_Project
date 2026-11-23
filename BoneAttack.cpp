/*
#include "bone.h"
#include "player.h"
#include <algorithm>

bone::bone()
    : finalx(0), finaly(0), startX(0), startY(0),
      initialVx(0), initialVy(0), gravity(900.0),
      elapsed(0), flightTime(0), flying(false)
{}

void bone::launchFromTo(qreal originX, qreal originY,
                        qreal targetX, qreal targetY, qreal seconds)
{
    pos_x = originX;
    pos_y = originY;
    launchTo(targetX, targetY, seconds);
}

void bone::launchTo(qreal targetX, qreal targetY, qreal seconds)
{
    if (seconds <= 0)
        return;

    startX = pos_x;
    startY = pos_y;
    finalx = targetX;
    finaly = targetY;
    flightTime = seconds;
    elapsed = 0;

    initialVx = (finalx - startX) / flightTime;
    initialVy = (finaly - startY
                 - 0.5 * gravity * flightTime * flightTime) / flightTime;
    flying = true;
}

void bone::parabolicMotion(qreal dt, player& jugador)
{
    if (!flying || dt <= 0)
        return;

    elapsed = std::min(elapsed + dt, flightTime);

    pos_x = startX + initialVx * elapsed;
    pos_y = startY + initialVy * elapsed + 0.5 * gravity * elapsed * elapsed;

    // Hitboxes centradas: hueso de 24x24 y jugador de 80x100.
    const qreal boneHalfWidth = 12.0;
    const qreal boneHalfHeight = 12.0;
    const qreal playerHalfWidth = 40.0;
    const qreal playerHalfHeight = 50.0;

    const qreal dx = jugador.getPos_x() - pos_x;
    const qreal dy = jugador.getPos_y() - pos_y;

    const bool tocaJugador =
        std::abs(dx) <= boneHalfWidth + playerHalfWidth &&
        std::abs(dy) <= boneHalfHeight + playerHalfHeight;

    if (tocaJugador && jugador.getRollRemaining() <= 0) {
        makeDamage(jugador);
        flying = false;
        return;
    }

    if (elapsed >= flightTime) {
        pos_x = finalx;
        pos_y = finaly;
        flying = false;
    }
}

bool bone::isFlying() const
{
    return flying;
}
*/

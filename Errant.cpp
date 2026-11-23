/*
#include "phantom.h"
#include <cmath>

namespace {
const qreal phantomHalfWidth = 40.0; //esto de aca toca moverlo segun el gameplay
const qreal phantomHalfHeight = 50.0;
const qreal playerHalfWidth = 40.0;
const qreal playerHalfHeight = 50.0;
}

phantom::phantom(qreal posPhantomX, qreal posPhantomY)
{
    pos_x = posPhantomX;
    pos_y = posPhantomY;
}

void phantom::persecutePlayer(player& jugador, qreal segundos)
{
    if (segundos <= 0)
        return;

    const qreal dx = jugador.getPos_x() - pos_x;
    const qreal dy = jugador.getPos_y() - pos_y;

    // Las posiciones son el centro de cada personaje.
    const bool tocaJugador =
        std::abs(dx) <= phantomHalfWidth + playerHalfWidth &&
        std::abs(dy) <= phantomHalfHeight + playerHalfHeight;

    if (tocaJugador && jugador.getRollRemaining() <= 0) {
        makeDamage(jugador);
        active = false;
        return;
    }

    // Si el jugador está rodando, no recibe daño y el phantom sigue persiguiéndolo.
    mover(dx, dy, segundos);
}
*/

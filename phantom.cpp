#include "phantom.h"

phantom::phantom(qreal posPhantomX, qreal posPhantomY){}

void phantom::persecutePlayer(player& jugador, qreal segundos)
{
    qreal dx = jugador.getPos_x() - pos_x;
    qreal dy = jugador.getPos_y() - pos_y;

    // cuando ya este en la misma posicion hay que hacer que haga daño y se inactive para no construir
    // otro objeto en la siguiente oleada si no que usar el mismo
    if (dx == 0 && dy == 0) // <--- posiblemente esto haya que cambiarlo a un area de contacto
        if(jugador.getRollRemaining() <= 0){
            this->makeDamage(jugador);
            this->active = false;
        }
    return ;

    mover(dx, dy, segundos);// ya esto normaliza y solo saca la direccion dle movimiento
}
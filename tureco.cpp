#include "tureco.h"
#include "silbon.h"

tureco::tureco() {}

void tureco::persecuteSilbon(silbon & jefe, qreal segundos){
    qreal dx = jefe.getPos_x() - pos_x;
    qreal dy = jefe.getPos_y() - pos_y;

    // cuando ya este en la misma posicion hay que hacer que haga daño y se inactive para no construir
    // otro objeto en la siguiente oleada si no que usar el mismo
    if (dx == 0 && dy == 0){
        // aqui lo que sea que haga cuando este junto al silbon
        return;
    }

    mover(dx, dy, segundos);// ya esto normaliza y solo saca la direccion dle movimiento
}
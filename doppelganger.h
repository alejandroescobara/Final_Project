#pragma once
#ifndef DOPPELGANGER_H
#define DOPPELGANGER_H

#include "character.h"

class player;

class doppelganger : public character
{
public:
    doppelganger(qreal posDoppelX, qreal posDoppelY);// la idea es que en base a la posicion del jugador wavemanager decida donde spawnean los doppelgangers
    static const char * rutaPng;
    void atack(player & jugador); //se basa en mira hacia y busca a entidades en ese espacio
    void roll(); //obtiene invulnerabilidad por un momento y se mueve mas rapido por un momento en la direccion de mira_hacia
    void parry(); //no tengo ni idea de como hacer esto pero asumo sera un undir un boton y despues si se acerta afecte el otro objeto

    bool getParryStatus();
    bool getRollingStatus();

    void setParryStatus();
    void setRollStatus();

private:
    bool isRolling;
    bool isParrying;
};

#endif // DOPPELGANGER_H

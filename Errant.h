/*
#pragma once

#ifndef PHANTOM_H
#define PHANTOM_H

#include <QtGlobal>
#include "player.h"
#include "character.h"

class phantom : public character
{
public:
    phantom(qreal posPhantomX, qreal posPhantomY); // la idea es que en base a la posicion del jugador wavemanager decida donde spawnean los fantasmas
    static const char * rutaPng;
    void atack(character & jugador) override;
    void persecutePlayer(player & jugador, qreal segundos);
};
#endif // PHANTOM_H
*/

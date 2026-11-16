#pragma once

#ifndef PLAYER_H
#define PLAYER_H

#include <QtGlobal>
#include <vector>
#include "character.h"

class phantom;
class doppelganger;
class silbon;
class tureco;

class player : public character
{
public:
    player();
    static const char * rutaPng;
    void atackLevel1();
    void atackLevel2(silbon & jefe, tureco & perro); //se basa en mira_hacia y busca a entidades en ese espacio por lo que necesita acceder a datos de wavemanager
    void roll(); //obtiene invulnerabilidad por un momento y se mueve mas rapido por un momento en la direccion de mira_hacia
    void parry(); //no tengo ni idea de como hacer esto pero asumo sera un undir un boton y despues si se acerta afecte el otro objeto, tambien hacer algo que no permita mantener presiondao, tipo tase de refresco
    qreal getRollRemaining();
    qreal getParryRemaining();
    void decreaseRollRemaining(qreal dt);
    void decreaseParryRemaining(qreal dt);
    void setIsBloquing(bool state);
    bool getIsBloquing();

    void FindToDamage(qreal topLeftCornerX,qreal topLeftCornerY, qreal bottonRigthCornerX,qreal bottonRigthCornerY,
                      std::vector<phantom *>enemigos_phantom, std::vector<doppelganger *>enemigos_doppel, char lookTO);

    void FindToDamage(qreal topLeftCornerX,qreal topLeftCornerY, qreal bottonRigthCornerX,qreal bottonRigthCornerY,
                              silbon & jefe, tureco & perro);


private:
    bool isBlocking;
    qreal rollRemaining;
    qreal parryRemaining; //seria bueno poner que solo se use la primera mitad del tiempo y la segunda sea coldown
    short alcoholInBlood;

};

#endif // PLAYER_H

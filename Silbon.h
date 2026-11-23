/*
#ifndef SILBON_H
#define SILBON_H

#include "character.h"
#include "bone.h"

class player;
class tureco;

class silbon : public character
{
public:
    silbon();

    void interactPlayer(player & jugador, qreal dt);
    void atackSignal();
    void atack(player & jugador);
    void kickTureco(tureco & perro);
    void launchBone(player & jugador);
    void updateBone(qreal dt, player& jugador);
    bone& getBone();
    void decreaseRemainingKick(qreal dt);
    void kickTureco(tureco perro);

private:
    bool boneIsflying;

    bone launchedBone;
    static qreal waitToAtack;//tiempo fijo que espera el silbon hasta atacar otra vez
    static qreal graceTime;  //tiempo fijo que dura el periodo de gracia
    static qreal timeTobone;  //tiempo fijo que espera entre lanzamientos de huesos
    static qreal timeToAtack; //tiempo fijo que se espera entre ataques
    bool atackready;//si ya lanzo el aviso de ataque

    //de aqui pa abajo son temporizadores (se les resta tiempo)
    qreal gracePeriod;//tiempo entre que alza y pega
    qreal remainingKickTureco;
    qreal remainingboneLaunch;
    qreal remainingAtackPlayer;// para que no ataque cada 2x3
};

#endif // SILBON_H
*/

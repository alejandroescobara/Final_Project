/*
#ifndef TURECO_H
#define TURECO_H

#include "character.h"

class player;
class silbon;

class tureco : public character
{
public:
    tureco();

    void persecuteSilbon(silbon& jefe, player& jugador, qreal dt);
    void receiveKick(qreal silbonX);

private:
    enum class State {
        Chasing,
        Launched,
        SlidingWithSilbon,
        Resting
    };

    void runing(silbon& jefe, player& jugador, qreal dt);

    State state;
    bool playerContactLatched;
    qreal oscillationCenterX;
};

#endif // TURECO_H
*/

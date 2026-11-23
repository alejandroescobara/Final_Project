/*
#ifndef BONE_H
#define BONE_H

#include "character.h"

class player;

class bone : public character
{
public:
    bone();

    void launchTo(qreal targetX, qreal targetY, qreal flightTime);
    void launchFromTo(qreal startX, qreal startY,qreal targetX, qreal targetY, qreal flightTime);
    void parabolicMotion(qreal dt, player& jugador);
    bool isFlying() const;

private:
    qreal finalx, finaly;
    qreal startX, startY;
    qreal initialVx, initialVy;
    qreal gravity;
    qreal elapsed, flightTime;
    bool flying;
};

#endif // BONE_H
*/

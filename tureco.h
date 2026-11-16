#ifndef TURECO_H
#define TURECO_H

#include "character.h"

class player;
class silbon;

class tureco : public character
{
public:
    tureco();
    void persecuteSilbon(silbon & jefe, qreal segundos);
    void decreaseInertia();

private:
    qreal kickInertiaRemaining;
    qreal aceleration;
};

#endif // TURECO_H

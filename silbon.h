#ifndef SILBON_H
#define SILBON_H

#include "character.h"

class player;
class tureco;

class silbon : public character
{
public:
    silbon();
    void kickTureco(tureco & perro);
    void launchBones(player & jugador);

private:
    bool atSilbon;    //si esta junto al silbon
};

#endif // SILBON_H

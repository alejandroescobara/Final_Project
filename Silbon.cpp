/*
#include "silbon.h"
#include "player.h"
#include "tureco.h"
#include <algorithm>
#include <cmath>

silbon::silbon()
    : boneIsflying(false), remainingKickTureco(0), remainingboneLaunch(0)
{}

void silbon::atackSignal(){
    //señal que hace el silbon (alza el saco) para qeu nosotros reaccionesmos
    //aqui pues alzaria el costal y pondria un contador
    gracePeriod = graceTime;

}


void silbon::interactPlayer(player & jugador, qreal dt){

    qreal playerX = jugador.getPos_x();

    if ((pos_x < 40) && (remainingAtackPlayer <=0) ){
        this->makeDamage(jugador);
        remainingAtackPlayer = waitToAtack;
        //aqui supongo la activacion de la animacion
        //debe haber un pequeño retraso entre una señal del silbon y el ataque para que el jugador reaccione
        //supongo estara en el metodo de actualizar
    }
    else if(pos_x > 40){
        mover(playerX,0,dt);

    }
}

void silbon::launchBone(player &jugador)
{
    // Solo se permite un hueso en vuelo y se respeta el tiempo entre lanzamientos.
    if (boneIsflying || remainingboneLaunch > 0)
        return;

    const qreal tiempoDeVuelo = 1.2; // segundos de reacción; ajustar al ritmo del juego
    const qreal cooldown = 3.0;      // segundos entre lanzamientos

    launchedBone.launchFromTo(pos_x, pos_y,jugador.getPos_x(), jugador.getPos_y(),tiempoDeVuelo);boneIsflying = launchedBone.isFlying();

    if (boneIsflying)
        remainingboneLaunch = cooldown;
}


void silbon::updateBone(qreal dt, player& jugador)
{
    if (dt <= 0)
        return;

    if (remainingboneLaunch > 0)
        remainingboneLaunch = std::max(qreal(0), remainingboneLaunch - dt);

    if (boneIsflying) {
        launchedBone.parabolicMotion(dt, jugador);
        boneIsflying = launchedBone.isFlying();
    }
}


bone& silbon::getBone()
{
    return launchedBone;
}

void silbon::kickTureco(tureco& perro)
{
    if (remainingKickTureco > 0)
        return;

    const qreal distanceX = std::abs(perro.getPos_x() - pos_x);
    const qreal kickRange = 40.0;

    if (distanceX <= kickRange) {
        perro.receiveKick(pos_x);
        remainingKickTureco = 2.0; // Cooldown en segundos
    }
}


void silbon::decreaseRemainingKick(qreal dt)
{
    remainingKickTureco = std::max(qreal(0), remainingKickTureco - dt);
}
*/

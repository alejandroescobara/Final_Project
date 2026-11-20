#include "character.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QBrush>
#include <QPen>
#include <cmath>

character::character()
    : live_points(0.0),
    mira_hacia('s'),
    pos_x(0.0),
    pos_y(0.0),
    AtackDamage(0.0),
    active(true),
    weight(1.0),
    vx(0.0),
    vy(0.0),
    maxSpeed(180.0),
    friction(6.0)
{}

character::~character(){}

qreal character::getPos_x(){
    return this->pos_x;
}

void character::setPos_x(qreal x)
{
    pos_x = x;
}

qreal character::getPos_y(){
    return this->pos_y;
}

void character::setPos_y(qreal y)
{
    pos_y = y;
}

char character::getMiraHacia(){
    return this->mira_hacia;
}

void character::setMiraHacia(char dir){
    this->mira_hacia = dir;
}

void character::mover(qreal dirX, qreal dirY, qreal segundos)
{
    if (segundos <= 0)
        return;

    const bool hayEntrada = (dirX != 0 || dirY != 0);

    if (hayEntrada) {
        // Actualiza hacia donde mira solo cuando hay entrada.
        if (dirX < 0 && dirY < 0)      mira_hacia = 'q';
        else if (dirX > 0 && dirY < 0) mira_hacia = 'e';
        else if (dirX > 0 && dirY > 0) mira_hacia = 'x';
        else if (dirX < 0 && dirY > 0) mira_hacia = 'z';
        else if (dirY < 0)             mira_hacia = 'w';
        else if (dirX < 0)             mira_hacia = 'a';
        else if (dirY > 0)             mira_hacia = 's';
        else                           mira_hacia = 'd';

        // Normaliza para que la diagonal no acelere mas
        const qreal longitud = std::sqrt(dirX * dirX + dirY * dirY);
        dirX /= longitud;
        dirY /= longitud;

        const qreal aceleracionBase = 500.0; // píxeles/s²; ajústala a tu juego
        const qreal aceleracion = aceleracionBase / weight;

        vx += dirX * aceleracion * segundos;
        vy += dirY * aceleracion * segundos;
    } else {
        //Reduccion de la velocidad
        const qreal factorFriccion = std::exp(-friction * segundos);
        vx *= factorFriccion;
        vy *= factorFriccion;
    }

    // Limitacion de la rapidez máxima
    const qreal rapidez = std::sqrt(vx * vx + vy * vy);
    if (rapidez > maxSpeed && rapidez > 0) {
        const qreal escala = maxSpeed / rapidez;
        vx *= escala;
        vy *= escala;
    }

    pos_x += vx * segundos;
    pos_y += vy * segundos;
}

void character::makeDamage(character& enemy)
{
    qreal vidaNueva = enemy.getVida() - this->AtackDamage;
    if (vidaNueva < 0)
        vidaNueva = 0;
    enemy.setVida(vidaNueva);
}

qreal character::getWeight() const
{
    return weight;
}
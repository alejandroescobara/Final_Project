#include "character.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QBrush>
#include <QPen>
#include <cmath>

character::character(){}
character::~character(){}

qreal character::getPos_x(){
    return this->pos_x;
}

qreal character::getPos_y(){
    return this->pos_y;
}

char character::getMiraHacia(){
    return this->mira_hacia;
}

void character::setMiraHacia(char dir){
    this->mira_hacia = dir;
}

void character::mover(qreal dirX, qreal dirY, qreal segundos)
{
    if (segundos <= 0 || (dirX == 0 && dirY == 0))
        return;

    // Guarda hacia dónde mira, usando las letras que elegiste.
    if (dirX < 0 && dirY < 0)      mira_hacia = 'q'; // arriba-izquierda
    else if (dirX > 0 && dirY < 0) mira_hacia = 'e'; // arriba-derecha
    else if (dirX > 0 && dirY > 0) mira_hacia = 'x'; // abajo-derecha
    else if (dirX < 0 && dirY > 0) mira_hacia = 'z'; // abajo-izquierda
    else if (dirY < 0)             mira_hacia = 'w';
    else if (dirX < 0)             mira_hacia = 'a';
    else if (dirY > 0)             mira_hacia = 's';
    else                           mira_hacia = 'd';

    // Normaliza para que la diagonal no sea más rápida.
    qreal longitud = std::sqrt(dirX * dirX + dirY * dirY);
    dirX /= longitud;
    dirY /= longitud;

    pos_x += dirX * velocity * segundos;
    pos_y += dirY * velocity * segundos;
}

void character::makeDamage(character& enemy)
{
    qreal vidaNueva = enemy.getVida() - this->AtackDamage;
    if (vidaNueva < 0)
        vidaNueva = 0;
    enemy.setVida(vidaNueva);
}
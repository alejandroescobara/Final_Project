#pragma once
#ifndef CHARACTER_H
#define CHARACTER_H
#include <QtGlobal>
class character
{
protected:
    qreal live_points;
    qreal velocity;// esta en pixeles por segundo
    char mira_hacia; //w arriba, a izquierda, s abajo, d Derecha, q diagonal wa, e diagonal wd, x diagonal sd, z diagonal as, se cambia al mover
    qreal pos_x;
    qreal pos_y;
    qreal AtackDamage;
    bool active;

public:
    character();
    ~character();
    void mover(qreal dirX, qreal dirY, qreal segundos);//quiero hacerlo de tal forma que se mueva en las 8 direcciones segun el teclado
    void makeDamage(character& enemy);
    virtual void atack(character & jugador);
    virtual void updateCharacter(qreal dt);

    void setVida(qreal vida);
    qreal getVida();

    char getMiraHacia();
    void setMiraHacia(char dir);

    void setVelocidad(qreal velocidad);
    qreal getVelocity();

    qreal getPos_y();
    qreal getPos_x();
    void setPos_y(qreal y);
    void setPos_x(qreal x);

};

#endif // CHARACTER_H

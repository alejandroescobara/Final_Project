#include "player.h"
#include "doppelganger.h"
#include "phantom.h"
#include "silbon.h"
#include "tureco.h"

player::player() {}

qreal player::getRollRemaining(){
    return this->rollRemaining;
}
qreal player::getParryRemaining(){
    return this->parryRemaining;
}

void player::decreaseRollRemaining(qreal dt){
    this->rollRemaining -= dt;
}
void player::decreaseParryRemaining(qreal dt){
    this->parryRemaining -= dt;
}

void player::setIsBloquing(bool state){
    this->isBlocking = state;
}
bool player::getIsBloquing(){
    return this->isBlocking;
}

void player::FindToDamage(qreal topLeftCornerX,qreal topLeftCornerY, qreal bottonRigthCornerX,qreal bottonRigthCornerY,
                          std::vector<phantom *>enemigos_phantom, std::vector<doppelganger *>enemigos_doppel, char lookTO){
    char direccion_daño = this->getMiraHacia();
    if(direccion_daño == 'w' && direccion_daño == 'a' && direccion_daño == 's' && direccion_daño == 'd'){
        for(auto enemigo :  enemigos_phantom){
            qreal enemyX = enemigo->getPos_x();
            qreal enemyY = enemigo->getPos_y();
            if(enemyX > topLeftCornerX && enemyX < bottonRigthCornerX
                && enemyY > topLeftCornerY && enemyY < bottonRigthCornerY){
                this->makeDamage(*enemigo);
            }
        }
        for(auto enemigo :  enemigos_doppel){
            qreal enemyX = enemigo->getPos_x();
            qreal enemyY = enemigo->getPos_y();
            if(enemyX > topLeftCornerX && enemyX < bottonRigthCornerX
                && enemyY > topLeftCornerY && enemyY < bottonRigthCornerY){
                if(enemigo->getParryStatus()){
                    //aqui se pondria el activador del efecto de que le hagan parry
                }
                else if(!enemigo->getRollingStatus()){
                    this->makeDamage(*enemigo);
                }
            }
        }
    }
    else{

    }
}

void player::FindToDamage(qreal topLeftCornerX,qreal topLeftCornerY, qreal bottonRigthCornerX,qreal bottonRigthCornerY,
    silbon & jefe, tureco & perro){

    qreal enemyX = jefe.getPos_x();
    qreal enemyY = jefe.getPos_y();
    if(enemyX > topLeftCornerX && enemyX < bottonRigthCornerX
        && enemyY > topLeftCornerY && enemyY < bottonRigthCornerY){
        this->makeDamage(jefe);
    }

}

void player::atackLevel2(silbon & jefe, tureco & perro){ //aqui habria que hacer un sistema de busqueda en wavemanager y ps eso aun no esta
    //la idea es que busque pixel por pixel en un rango establecido asumo que por una figura los enemigos que ahi se
    //encuentren y pues les aplique el daño, solo habria inconvenientes si un doppel nos hace parry
    switch (this->mira_hacia) {
    case 'a':
        player::FindToDamage(this->pos_x,this->pos_y + 40,this->pos_x - 40,this->pos_y - 40,jefe, perro);
        break;
    case 'd':
        player::FindToDamage(this->pos_x,this->pos_y + 40,this->pos_x + 40,this->pos_y-40,jefe, perro);
        break;
    default:
        break;
    }
}

void player::atackLevel1(){ //aqui habria que hacer un sistema de busqueda en wavemanager y ps eso aun no esta
    //la idea es que busque pixel por pixel en un rango establecido asumo que por una figura los enemigos que ahi se
    //encuentren y pues les aplique el daño, solo habria inconvenientes si un doppel nos hace parry
    switch (this->mira_hacia) {
    case 'w':
        break;
    case 'a':

        break;
    case 's':

        break;
    case 'd':

        break;
    case 'q':

        break;
    case 'e':

        break;
    case 'x':

        break;
    case 'z':

        break;
    default:

        break;
    }
}
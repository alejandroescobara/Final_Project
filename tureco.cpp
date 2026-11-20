#include "tureco.h"
#include "silbon.h"
#include "player.h"
#include <cmath>

tureco::tureco()
    : state(State::Chasing),
    playerContactLatched(false),
    oscillationCenterX(0)
{
    weight = 1.0;
    maxSpeed = 180.0;
    friction = 5.0;
    vx = 0;
    vy = 0;
}

void tureco::persecuteSilbon(silbon& jefe, player& jugador, qreal dt)
{
    if (dt <= 0)
        return;

    if (state != State::Chasing) {
        runing(jefe, jugador, dt);
        return;
    }

    const qreal dx = jefe.getPos_x() - pos_x;
    const qreal distancia = std::abs(dx);
    const qreal distanciaFrenado =
        friction > 0 ? std::abs(vx) / friction : 0;
    const qreal rangoPatada = 30.0;

    if (distancia <= rangoPatada + distanciaFrenado)
        mover(0, 0, dt);
    else
        mover(dx, 0, dt); // Silbon y Tureco solo se mueven en X
}

void tureco::receiveKick(qreal silbonX)
{
    if (state != State::Chasing && state != State::Resting)
        return;

    // El punto de retorno de la trayectoria es la posición de Silbon.
    oscillationCenterX = silbonX;
    vx = (pos_x < silbonX ? -1.0 : 1.0) * 260.0;
    vy = 0;
    playerContactLatched = false;
    state = State::Launched;
}

void tureco::runing(silbon& jefe, player& jugador, qreal dt)
{
    if (state == State::Resting)
        return;

    // Después del choque inelástico se desplazan juntos y se frenan.
    if (state == State::SlidingWithSilbon) {
        vx *= std::exp(-friction * dt);
        const qreal dx = vx * dt;

        pos_x += dx;
        jefe.setPos_x(jefe.getPos_x() + dx);

        if (std::abs(vx) < 1.0) {
            vx = 0;
            state = State::Resting;
        }
        return;
    }

    // Trayectoria oscilatoria horizontal que ya tenía Tureco.
    const qreal oldX = pos_x;
    const qreal spring = 40.0;
    const qreal damping = 6.0;
    const qreal errorX = pos_x - oscillationCenterX;
    const qreal accelerationX =
        (-spring * errorX - damping * vx) / weight;

    vx += accelerationX * dt;
    pos_x += vx * dt;
    const qreal newX = pos_x;

    // Hitbox horizontal: cada personaje tiene semiancho de 40 px.
    // Se comprueba todo el tramo para no omitir el choque entre fotogramas.
    const qreal playerContact = 80.0;
    const qreal segmentMin = std::min(oldX, newX);
    const qreal segmentMax = std::max(oldX, newX);
    const qreal playerX = jugador.getPos_x();

    const bool cruzaJugador =
        segmentMin <= playerX + playerContact &&
        segmentMax >= playerX - playerContact;

    if (cruzaJugador && !playerContactLatched) {
        if (jugador.getRollRemaining() > 0) {
            // El roll da invulnerabilidad: no daña ni rebota.
        } else if (jugador.getIsBloquing()) {
            vx = -vx; // rebote elástico: misma rapidez, sentido contrario
        } else {
            makeDamage(jugador);
            // No se modifica vx: Tureco atraviesa al jugador.
        }
    }
    playerContactLatched = cruzaJugador;

    // En el regreso, detecta el contacto con Silbon.
    const qreal silbonX = jefe.getPos_x();
    const qreal silbonContact = 80.0;
    const bool seAcercaASilbon =
        (silbonX - oldX) * (newX - oldX) > 0;
    const bool cruzaSilbon =
        segmentMin <= silbonX + silbonContact &&
        segmentMax >= silbonX - silbonContact;

    if (seAcercaASilbon && cruzaSilbon) {
        // Silbon se considera quieto antes del impacto.
        const qreal pesoTotal = weight + jefe.getWeight();
        if (pesoTotal > 0)
            vx = weight * vx / pesoTotal;

        state = State::SlidingWithSilbon;
    }
}

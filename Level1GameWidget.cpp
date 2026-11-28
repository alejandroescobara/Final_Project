#include "Level1GameWidget.h"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QLinearGradient>
#include <QPainterPath>
#include <QtMath>
#include <algorithm>
#include <array>
#include <cmath>

#include "GameplayConfig.h"
#include "Attack.h"

namespace {
  constexpr int FRAME_INTERVAL_MS = 16;
  constexpr float MAX_DELTA_TIME = 0.05f;
  constexpr float ATTACK_FLASH_DURATION = 0.14f;
  constexpr float ACTION_SPRITE_DURATION = 0.30f;
  constexpr int SPRITE_CELL_SIZE = 512;

  int facingAxis(const QVector2D& direction) {
    if (std::abs(direction.x()) > std::abs(direction.y())) {
      return direction.x() < 0.0f ? 3 : 2; // lado: izquierda / derecha
    }
    return direction.y() < 0.0f ? 1 : 0; // espalda / frente
  }

  int attackSpriteRow(const QVector2D& direction) {
    switch (facingAxis(direction)) {
      case 1: return 4; // ataque hacia arriba
      case 2: return 5; // ataque hacia la derecha
      case 0: return 7; // ataque hacia abajo: reutiliza la pose lateral izquierda
      default: return 7; // ataque hacia la izquierda
    }
  }

  void drawActorSprite(QPainter& painter,
                       const QImage& sheet,
                       QPointF position,
                       QVector2D movement,
                       QVector2D facing,
                       QVector2D actionDirection,
                       float animationTime,
                       int actionRow,
                       float actionElapsed,
                       qreal width,
                       qreal height) {
    if (sheet.isNull()) return;

    QVector2D direction = actionRow >= 0
        ? (actionDirection.isNull() ? facing : actionDirection)
        : (movement.isNull() ? facing : movement);
    if (direction.isNull()) direction = QVector2D(0.0f, 1.0f);
    const bool moving = !movement.isNull() && movement.lengthSquared() > 4.0f;
    const int axis = facingAxis(direction);
    int row = 0;
    int column = 0;
    bool flip = false;

    if (actionRow >= 0) {
      row = actionRow;
      if (row == 3) {
        // Inicio y cierre comparten la pose direccional; el cuadro central es la bolita.
        const int rollStartColumn = axis == 0 ? 2 : axis == 1 ? 1 : 0;
        const bool isBallPhase = actionElapsed >= 0.08f && actionElapsed < 0.24f;
        column = isBallPhase ? 3 : rollStartColumn;
        flip = axis == 3 && !isBallPhase;
      } else if (row >= 4 && row <= 7) {
        // Las filas nuevas son preparación y golpe para cada orientación.
        column = actionElapsed < 0.12f ? 0 : 1;
      }
      if (row >= 4 && row <= 7) {
        flip = (row == 5 || row == 7) && axis == 3;
      }
    } else if (moving && axis >= 2) {
      row = 1;
      column = static_cast<int>(animationTime * 9.0f) % 5;
      flip = axis == 3;
    } else if (moving && axis == 1) {
      row = 2;
      column = static_cast<int>(animationTime * 7.0f) % 3;
    } else if (axis == 1) {
      row = 0;
      column = 1;
    } else if (axis >= 2) {
      row = 0;
      column = 2;
      flip = axis == 3;
    }

    const QRect source(column * SPRITE_CELL_SIZE,
                       row * SPRITE_CELL_SIZE,
                       SPRITE_CELL_SIZE,
                       SPRITE_CELL_SIZE);
    const QRectF target(position.x() - width * 0.5,
                        position.y() - height + 15.0,
                        width,
                        height);
    painter.save();
    if (flip) {
      painter.translate(position.x() * 2.0, 0.0);
      painter.scale(-1.0, 1.0);
    }
    painter.drawImage(target, sheet, source);
    painter.restore();
  }
}  // namespace

Level1GameWidget::Level1GameWidget(QWidget* parent)
    : QWidget(parent) {
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);

  backgroundImage.load(QStringLiteral(":/assets/level1_background.png"));
  ghostSpriteSheet.load(QStringLiteral(":/assets/ghost_silbon_bone_spritesheet.png"));
  otonielSpriteSheet.load(QStringLiteral(":/assets/otoniel_level1_spritesheet.png"));
  doppelSpriteSheet.load(QStringLiteral(":/assets/doppel_level1_spritesheet.png"));

  connect(&timer, &QTimer::timeout, this, &Level1GameWidget::updateGame);
  timer.setInterval(FRAME_INTERVAL_MS);

  restartLevel();
}

Level1GameWidget::~Level1GameWidget() {
  world.saveAllKnowledge();
}

void Level1GameWidget::setPaused(bool paused) {
  if (paused) {
    timer.stop();
    clearInputState();
    return;
  }

  if (world.getState() != PlanarWorldState::Playing) {
    restartLevel();
  }

  frameTimer.restart();
  if (!timer.isActive()) {
    timer.start();
  }
}

void Level1GameWidget::restartLevel() {
  world.reset();
  clearInputState();
  attackFlashTimer = 0.0f;
  bossTransitionEmitted = false;
  spriteAnimationTime = 0.0f;
  playerActionAnimationTimer = 0.0f;
  frameTimer.restart();
  update();
}

void Level1GameWidget::clearInputState() {
  moveUp = false;
  moveDown = false;
  moveLeft = false;
  moveRight = false;
  attackQueued = false;
  rollQueued = false;
  parryQueued = false;
}

void Level1GameWidget::updateGame() {
  const float rawDt = frameTimer.isValid()
                          ? static_cast<float>(frameTimer.restart()) / 1000.0f
                          : (static_cast<float>(FRAME_INTERVAL_MS) / 1000.0f);
  const float dt = std::clamp(rawDt, 0.0f, MAX_DELTA_TIME);

  spriteAnimationTime += dt;
  if (attackFlashTimer > 0.0f) {
    attackFlashTimer = std::max(0.0f, attackFlashTimer - dt);
  }
  if (playerActionAnimationTimer > 0.0f) {
    playerActionAnimationTimer = std::max(0.0f, playerActionAnimationTimer - dt);
  }

  if (world.getState() != PlanarWorldState::Playing) {
    update();
    return;
  }

  PlanarInput input;
  const float moveX = (moveRight ? 1.0f : 0.0f) - (moveLeft ? 1.0f : 0.0f);
  const float moveY = (moveDown ? 1.0f : 0.0f) - (moveUp ? 1.0f : 0.0f);
  input.moveDirection = QVector2D(moveX, moveY);

  if (hasMouseAim) {
    const QVector2D toCursor = screenToWorld(lastMousePos) - world.getPlayer().getPosition();
    if (!toCursor.isNull()) {
      input.aimDirection = toCursor.normalized();
    }
  }

  input.attackPressed = attackQueued;
  input.rollPressed = rollQueued;
  input.parryPressed = parryQueued;

  if (input.attackPressed) {
    playerActionAnimationTimer = ACTION_SPRITE_DURATION;
    lastAttackDir = !input.aimDirection.isNull()
                        ? input.aimDirection
                        : (!input.moveDirection.isNull()
                               ? input.moveDirection.normalized()
                               : world.getPlayer().getFacingVector());
    attackFlashTimer = ATTACK_FLASH_DURATION;
  }

  attackQueued = false;
  rollQueued = false;
  parryQueued = false;

  world.update(dt, input);

  if (world.getState() == PlanarWorldState::ReadyForBossFight && !bossTransitionEmitted) {
    bossTransitionEmitted = true;
    setPaused(true);
    emit enterBossFight();
    return;
  }

  update();
}

QPointF Level1GameWidget::computeCameraOrigin() const {
  const float camW = GameplayConfig::level1CameraWidth;
  const float camH = GameplayConfig::level1CameraHeight;
  const float maxCamX = std::max(0.0f, GameplayConfig::level1MapWidth - camW);
  const float maxCamY = std::max(0.0f, GameplayConfig::level1MapHeight - camH);

  const QVector2D playerPos = world.getPlayer().getPosition();
  const float camX = std::clamp(playerPos.x() - camW * 0.5f, 0.0f, maxCamX);
  const float camY = std::clamp(playerPos.y() - camH * 0.5f, 0.0f, maxCamY);

  return QPointF(camX, camY);
}

QVector2D Level1GameWidget::screenToWorld(QPointF screenPos) const {
  if (width() <= 0 || height() <= 0) {
    return world.getPlayer().getPosition();
  }

  const QPointF camOrigin = computeCameraOrigin();
  const double worldX = camOrigin.x() +
                        (screenPos.x() / static_cast<double>(width())) *
                            GameplayConfig::level1CameraWidth;
  const double worldY = camOrigin.y() +
                        (screenPos.y() / static_cast<double>(height())) *
                            GameplayConfig::level1CameraHeight;
  return QVector2D(static_cast<float>(worldX), static_cast<float>(worldY));
}

void Level1GameWidget::paintEvent(QPaintEvent* /*event*/) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.fillRect(rect(), QColor(24, 21, 19));

  const QPointF cameraOrigin = computeCameraOrigin();

  painter.save();
  const double scaleX = static_cast<double>(width()) / GameplayConfig::level1CameraWidth;
  const double scaleY = static_cast<double>(height()) / GameplayConfig::level1CameraHeight;
  painter.scale(scaleX, scaleY);
  painter.translate(-cameraOrigin.x(), -cameraOrigin.y());
  drawWorld(painter, cameraOrigin);
  painter.restore();

  drawHud(painter);

  if (world.getState() == PlanarWorldState::GameOver) {
    drawOverlay(painter,
                QStringLiteral("HAS CAÍDO"),
                QStringLiteral("Presiona R para reintentar o Esc para volver al menú"));
  }
}

void Level1GameWidget::drawWorld(QPainter& painter, QPointF cameraOrigin) const {
  const float mapW = GameplayConfig::level1MapWidth;
  const float mapH = GameplayConfig::level1MapHeight;

  // Fondo del escenario; la imagen ya incluye los muros y el terreno de la arena.
  if (!backgroundImage.isNull()) {
    painter.drawImage(QRectF(0.0, 0.0, mapW, mapH), backgroundImage);
  } else {
    painter.fillRect(QRectF(0.0, 0.0, mapW, mapH), QColor(34, 30, 26));
  }

  // Zona del portal hacia el Nivel 2 (borde superior)
  const QRectF portalRect = world.getBossPortalRect();
  if (world.isBossPortalUnlocked()) {
    painter.fillRect(portalRect, QColor(90, 200, 120, 110));
    painter.setPen(QPen(QColor(150, 255, 170), 3.0));
    painter.drawRect(portalRect);
  } else {
    painter.fillRect(portalRect, QColor(90, 80, 70, 55));
    painter.setPen(QPen(QColor(120, 110, 95), 2.0, Qt::DashLine));
    painter.drawRect(portalRect);
  }

  // Errants
  for (const auto& errant : world.getErrants()) {
    if (!errant->getIsActive()) continue;
    const QPointF pos = errant->getPosition().toPointF();
    if (!ghostSpriteSheet.isNull()) {
      QVector2D moveDirection = errant->getMoveVelocity();
      if (moveDirection.isNull()) {
        moveDirection = errant->getFacingVector();
      }

      int directionCell = 0;  // Abajo
      if (std::abs(moveDirection.x()) > std::abs(moveDirection.y())) {
        directionCell = moveDirection.x() < 0.0f ? 2 : 3;  // Izquierda / derecha
      } else if (moveDirection.y() < 0.0f) {
        directionCell = 1;  // Arriba
      }

      const std::array<QRect, 4> ghostFrames{
          QRect(16, 120, 312, 444), QRect(360, 104, 308, 476),
          QRect(700, 152, 308, 380), QRect(1040, 160, 308, 364)};
      const QRectF ghostRect(pos.x() - 22.0, pos.y() - 30.0, 44.0, 60.0);
      painter.drawImage(ghostRect, ghostSpriteSheet, ghostFrames[directionCell]);
    } else {
      painter.setBrush(QColor(195, 70, 65));
      painter.setPen(QPen(QColor(255, 185, 175), 2.0));
      painter.drawEllipse(pos, 15.0, 15.0);
    }

    drawBar(painter, QRectF(pos.x() - 18.0, pos.y() - 26.0, 36.0, 5.0),
            errant->getHp(), errant->getMaxHp(), QColor(220, 75, 70));
  }

  // Doppels: usan la hoja azul para distinguirlos de Otoniel.
  for (const auto& doppel : world.getDoppels()) {
    if (!doppel->getIsActive()) continue;
    const QPointF pos = doppel->getPosition().toPointF();
    const Attack* doppelAttack = doppel->getPrimaryAttack();
    const bool attackPose = doppelAttack && doppelAttack->getCooldownTimer() > 0.0f &&
                            doppelAttack->getCooldown() - doppelAttack->getCooldownTimer() < ACTION_SPRITE_DURATION;
    const int doppelActionRow = doppel->isRolling() ? 3 :
                                doppel->isParrying() ? 8 :
                                attackPose ? attackSpriteRow(doppel->getFacingVector()) : -1;
    const float doppelActionElapsed = doppel->isRolling()
        ? doppel->getRollElapsedTime()
        : attackPose ? doppelAttack->getCooldown() - doppelAttack->getCooldownTimer()
                     : spriteAnimationTime;
    drawActorSprite(painter, doppelSpriteSheet, pos, doppel->getMoveVelocity(),
                    doppel->getFacingVector(), doppel->getFacingVector(),
                    spriteAnimationTime,
                    doppelActionRow, doppelActionElapsed, 54.0, 66.0);

    drawBar(painter, QRectF(pos.x() - 25.0, pos.y() - 48.0, 50.0, 6.0),
            doppel->getHp(), doppel->getMaxHp(), QColor(65, 145, 225));
  }
  // Jugador
  const PlanarPlayer& player = world.getPlayer();
  const QPointF playerPos = player.getPosition().toPointF();

  // Haz cortante en media luna, orientado hacia el golpe.
  if (attackFlashTimer > 0.0f) {
    const qreal reach = GameplayConfig::planarPlayer.attackRange;
    const qreal fade = std::clamp(attackFlashTimer / ATTACK_FLASH_DURATION, 0.0f, 1.0f);
    const qreal progress = 1.0 - fade;
    const qreal swingScale = 0.82 + 0.18 * progress;

    QPainterPath crescent;
    crescent.moveTo(reach * 0.32, -reach * 0.24);
    crescent.cubicTo(reach * 0.48, -reach * 0.68,
                     reach * 0.91, -reach * 0.60,
                     reach, 0.0);
    crescent.cubicTo(reach * 0.91, reach * 0.60,
                     reach * 0.48, reach * 0.68,
                     reach * 0.32, reach * 0.24);
    crescent.cubicTo(reach * 0.51, reach * 0.13,
                     reach * 0.58, -reach * 0.13,
                     reach * 0.32, -reach * 0.24);
    crescent.closeSubpath();

    painter.save();
    painter.translate(playerPos);
    painter.rotate(qRadiansToDegrees(std::atan2(lastAttackDir.y(), lastAttackDir.x())));
    painter.scale(swingScale, swingScale);
    painter.setOpacity(fade);

    QLinearGradient glow(0.0, -reach * 0.45, reach, reach * 0.45);
    glow.setColorAt(0.0, QColor(255, 246, 190, 235));
    glow.setColorAt(0.48, QColor(255, 204, 91, 220));
    glow.setColorAt(1.0, QColor(229, 116, 45, 205));
    painter.setBrush(glow);
    painter.setPen(QPen(QColor(93, 45, 26, 225), 2.5, Qt::SolidLine,
                        Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(crescent);

    QPainterPath highlight;
    highlight.moveTo(reach * 0.34, -reach * 0.22);
    highlight.cubicTo(reach * 0.50, -reach * 0.60,
                      reach * 0.87, -reach * 0.52,
                      reach * 0.94, 0.0);
    highlight.cubicTo(reach * 0.87, reach * 0.52,
                      reach * 0.50, reach * 0.60,
                      reach * 0.34, reach * 0.22);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(255, 250, 218, 235), 2.0, Qt::SolidLine,
                        Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(highlight);
    painter.restore();
  }

  const int playerActionRow = player.isRolling() ? 3 :
                              player.isParrying() ? 8 :
                              playerActionAnimationTimer > 0.0f
                                  ? attackSpriteRow(lastAttackDir)
                                  : -1;
  const float playerActionElapsed = player.isRolling()
      ? player.getRollElapsedTime()
      : ACTION_SPRITE_DURATION - playerActionAnimationTimer;
  drawActorSprite(painter, otonielSpriteSheet, playerPos, player.getMoveVelocity(),
                  player.getFacingVector(),
                  playerActionRow >= 4 && playerActionRow <= 7
                      ? lastAttackDir : player.getFacingVector(),
                  spriteAnimationTime,
                  playerActionRow, playerActionElapsed, 54.0, 68.0);
}

void Level1GameWidget::drawHud(QPainter& painter) const {
  const PlanarPlayer& player = world.getPlayer();

  // Panel superior izquierdo
  painter.fillRect(QRectF(14.0, 14.0, 270.0, 114.0), QColor(18, 15, 13, 185));
  painter.setPen(QColor(180, 145, 95));
  painter.drawRect(QRectF(14.0, 14.0, 270.0, 114.0));

  painter.setPen(QColor(245, 233, 209));
  painter.drawText(QRectF(24.0, 20.0, 250.0, 18.0),
                   QStringLiteral("HP: %1 / %2   |   Nivel %3")
                       .arg(player.getHp())
                       .arg(player.getMaxHp())
                       .arg(player.getLevel()));
  drawBar(painter, QRectF(24.0, 40.0, 250.0, 12.0),
          player.getHp(), player.getMaxHp(), QColor(85, 195, 105));

  painter.setPen(QColor(215, 205, 185));
  painter.drawText(QRectF(24.0, 58.0, 250.0, 18.0),
                   QStringLiteral("XP: %1 / %2")
                       .arg(player.getXp())
                       .arg(player.getXpToNextLevel()));
  drawBar(painter, QRectF(24.0, 78.0, 250.0, 8.0),
          player.getXp(), player.getXpToNextLevel(), QColor(90, 165, 235));

  painter.setPen(QColor(245, 220, 160));
  painter.drawText(QRectF(24.0, 96.0, 250.0, 20.0),
                   QStringLiteral("Doppels: %1 / %2   |   Errants: %3")
                       .arg(world.getDoppelsDefeated())
                       .arg(GameplayConfig::level1DoppelsForBoss)
                       .arg(world.getErrants().size()));

  const float rosaryRemaining = player.getRosaryRemainingTime();
  if (rosaryRemaining > 0.0f) {
    const QRectF banner(width() * 0.5 - 150.0, 54.0, 300.0, 30.0);
    painter.fillRect(banner, QColor(106, 79, 31, 220));
    painter.setPen(QColor(255, 235, 170));
    painter.drawText(banner, Qt::AlignCenter,
                     QStringLiteral("ROSARIO · Regeneración %1 s")
                         .arg(rosaryRemaining, 0, 'f', 1));
  }

  // Aviso de estado borracho o portal desbloqueado
  if (world.isPlayerDrunk()) {
    const QRectF banner(width() * 0.5 - 150.0, 16.0, 300.0, 32.0);
    painter.fillRect(banner, QColor(120, 75, 25, 200));
    painter.setPen(QColor(255, 230, 175));
    painter.drawText(banner, Qt::AlignCenter,
                     QStringLiteral("EBRIO · %1 s")
                         .arg(world.getDrunkRemainingTime(), 0, 'f', 1));
  } else if (world.isBossPortalUnlocked()) {
    const QRectF banner(width() * 0.5 - 210.0, 16.0, 420.0, 32.0);
    painter.fillRect(banner, QColor(35, 110, 55, 210));
    painter.setPen(QColor(225, 255, 225));
    painter.drawText(banner, Qt::AlignCenter,
                     QStringLiteral("▲ CAMINO AL SILBÓN ABIERTO (BORDE SUPERIOR) ▲"));
  }

  // Barra inferior de controles
  const QRectF controlsRect(14.0, height() - 34.0, width() - 28.0, 24.0);
  painter.fillRect(controlsRect, QColor(18, 15, 13, 160));
  painter.setPen(QColor(210, 198, 178));
  painter.drawText(controlsRect, Qt::AlignCenter,
                   QStringLiteral("WASD: Mover  |  Click Izq / J: Atacar  |  Espacio / K: Roll  |  Click Der / E / L: Parry  |  R: Reiniciar  |  Esc: Menú"));
}

void Level1GameWidget::drawOverlay(QPainter& painter,
                                   const QString& title,
                                   const QString& subtitle) const {
  painter.fillRect(rect(), QColor(0, 0, 0, 155));

  QFont titleFont = painter.font();
  titleFont.setPointSize(28);
  titleFont.setBold(true);
  painter.setFont(titleFont);
  painter.setPen(QColor(235, 195, 125));
  painter.drawText(QRectF(0.0, height() * 0.38, width(), 50.0), Qt::AlignCenter, title);

  QFont subFont = painter.font();
  subFont.setPointSize(13);
  subFont.setBold(false);
  painter.setFont(subFont);
  painter.setPen(QColor(245, 235, 215));
  painter.drawText(QRectF(0.0, height() * 0.48, width(), 36.0), Qt::AlignCenter, subtitle);
}

void Level1GameWidget::drawBar(QPainter& painter,
                               QRectF rect,
                               int value,
                               int maxValue,
                               const QColor& fillColor) const {
  painter.fillRect(rect, QColor(45, 40, 38));
  if (maxValue > 0 && value > 0) {
    const double ratio = std::clamp(static_cast<double>(value) / maxValue, 0.0, 1.0);
    painter.fillRect(QRectF(rect.x(), rect.y(), rect.width() * ratio, rect.height()), fillColor);
  }
  painter.setPen(QColor(15, 12, 10));
  painter.drawRect(rect);
}

void Level1GameWidget::keyPressEvent(QKeyEvent* event) {
  if (event->isAutoRepeat()) return;

  switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_Up:
      moveUp = true;
      break;
    case Qt::Key_S:
    case Qt::Key_Down:
      moveDown = true;
      break;
    case Qt::Key_A:
    case Qt::Key_Left:
      moveLeft = true;
      break;
    case Qt::Key_D:
    case Qt::Key_Right:
      moveRight = true;
      break;
    case Qt::Key_J:
      attackQueued = true;
      break;
    case Qt::Key_Space:
    case Qt::Key_Shift:
    case Qt::Key_K:
      rollQueued = true;
      break;
    case Qt::Key_E:
    case Qt::Key_L:
      parryQueued = true;
      break;
    case Qt::Key_R:
      restartLevel();
      break;
    case Qt::Key_Escape:
      world.saveAllKnowledge();
      setPaused(true);
      emit returnToMenu();
      break;
    default:
      QWidget::keyPressEvent(event);
      break;
  }
}

void Level1GameWidget::keyReleaseEvent(QKeyEvent* event) {
  if (event->isAutoRepeat()) return;

  switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_Up:
      moveUp = false;
      break;
    case Qt::Key_S:
    case Qt::Key_Down:
      moveDown = false;
      break;
    case Qt::Key_A:
    case Qt::Key_Left:
      moveLeft = false;
      break;
    case Qt::Key_D:
    case Qt::Key_Right:
      moveRight = false;
      break;
    default:
      QWidget::keyReleaseEvent(event);
      break;
  }
}

void Level1GameWidget::mouseMoveEvent(QMouseEvent* event) {
  lastMousePos = event->position();
  hasMouseAim = true;
  QWidget::mouseMoveEvent(event);
}

void Level1GameWidget::mousePressEvent(QMouseEvent* event) {
  lastMousePos = event->position();
  hasMouseAim = true;

  if (event->button() == Qt::LeftButton) {
    attackQueued = true;
  } else if (event->button() == Qt::RightButton) {
    parryQueued = true;
  }

  QWidget::mousePressEvent(event);
}

void Level1GameWidget::focusOutEvent(QFocusEvent* event) {
  clearInputState();
  QWidget::focusOutEvent(event);
}





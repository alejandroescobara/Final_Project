#include "Level1GameWidget.h"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QtMath>
#include <algorithm>
#include <array>
#include <cmath>

#include "GameplayConfig.h"

namespace {
  constexpr int FRAME_INTERVAL_MS = 16;
  constexpr float MAX_DELTA_TIME = 0.05f;
  constexpr float ATTACK_FLASH_DURATION = 0.14f;
}  // namespace

Level1GameWidget::Level1GameWidget(QWidget* parent)
    : QWidget(parent) {
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);

  backgroundImage.load(QStringLiteral(":/assets/level1_background.png"));
  ghostSpriteSheet.load(QStringLiteral(":/assets/ghost_silbon_bone_spritesheet.png"));

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

  if (attackFlashTimer > 0.0f) {
    attackFlashTimer = std::max(0.0f, attackFlashTimer - dt);
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

  // Doppels
  for (const auto& doppel : world.getDoppels()) {
    if (!doppel->getIsActive()) continue;
    const QPointF pos = doppel->getPosition().toPointF();
    const QPointF dirEnd = (doppel->getPosition() + doppel->getFacingVector() * 28.0f).toPointF();

    painter.setBrush(QColor(175, 80, 210));
    painter.setPen(QPen(QColor(240, 195, 255), 2.5));
    painter.drawEllipse(pos, 20.0, 20.0);
    painter.drawLine(pos, dirEnd);

    drawBar(painter, QRectF(pos.x() - 24.0, pos.y() - 32.0, 48.0, 6.0),
            doppel->getHp(), doppel->getMaxHp(), QColor(200, 95, 235));
  }

  // Jugador
  const PlanarPlayer& player = world.getPlayer();
  const QPointF playerPos = player.getPosition().toPointF();

  // Arco visual temporal al atacar
  if (attackFlashTimer > 0.0f) {
    const float radius = GameplayConfig::planarPlayer.attackRange;
    const double angleRad = std::atan2(-lastAttackDir.y(), lastAttackDir.x());
    const double centerDeg = qRadiansToDegrees(angleRad);
    const double spanDeg = GameplayConfig::planarPlayerAttackAngle;

    painter.setBrush(QColor(245, 220, 130, 85));
    painter.setPen(QPen(QColor(255, 235, 160, 180), 2.0));
    painter.drawPie(QRectF(playerPos.x() - radius, playerPos.y() - radius,
                           radius * 2.0f, radius * 2.0f),
                    static_cast<int>((centerDeg - spanDeg * 0.5) * 16.0),
                    static_cast<int>(spanDeg * 16.0));
  }

  const QColor playerFill = world.isPlayerDrunk() ? QColor(215, 160, 70) : QColor(75, 165, 220);
  painter.setBrush(playerFill);
  painter.setPen(QPen(QColor(245, 240, 225), 2.5));
  painter.drawEllipse(playerPos, 18.0, 18.0);

  const QPointF facingEnd = (player.getPosition() + player.getFacingVector() * 28.0f).toPointF();
  painter.setPen(QPen(QColor(255, 245, 210), 3.0));
  painter.drawLine(playerPos, facingEnd);
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





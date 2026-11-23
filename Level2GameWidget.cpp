#include "Level2GameWidget.h"

#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QtMath>

#include <algorithm>
#include <memory>

#include "BoxAttack.h"
#include "GameplayConfig.h"
#include "LateralPlayer.h"
#include "Projectile.h"
#include "Silbon.h"
#include "Tureco.h"

Level2GameWidget::Level2GameWidget(QWidget* parent)
    : QWidget(parent),
      background(QStringLiteral(":/assets/level2_background.png")),
      playerSpriteSheet(QStringLiteral(":/assets/player_spritesheet.png")) {
  setFocusPolicy(Qt::StrongFocus);
  setMinimumSize(900, 560);
  timer.setTimerType(Qt::PreciseTimer);
  timer.setInterval(16);
  connect(&timer, &QTimer::timeout, this, &Level2GameWidget::updateGame);
  restartLevel();
  timer.start();
}

Level2GameWidget::~Level2GameWidget() = default;

void Level2GameWidget::setPaused(bool paused) {
  if (paused) timer.stop();
  else if (!timer.isActive()) timer.start();
}

void Level2GameWidget::restartLevel() {
  projectiles.clear();
  gameFinished = false;
  playerWon = false;
  moveLeft = false;
  moveRight = false;

  player = std::make_unique<LateralPlayer>(
      QVector2D(170.0f, GameplayConfig::levelGroundY),
      GameplayConfig::playerHealth,
      GameplayConfig::playerMoveSpeed,
      GameplayConfig::playerWeight,
      GameplayConfig::playerJumpSpeed,
      GameplayConfig::levelGroundY,
      Roll(GameplayConfig::playerRollDistance / GameplayConfig::playerRollDuration,
           GameplayConfig::playerRollDuration,
           GameplayConfig::playerRollCooldown),
      Parry(GameplayConfig::playerParryWindow,
            GameplayConfig::parryRecovery,
            GameplayConfig::parryCooldown,
            0.7f,
            GameplayConfig::meleeKnockback),
      GameplayConfig::lateralGravity,
      GameplayConfig::movementFriction);
  player->setPrimaryAttack(std::make_unique<BoxAttack>(
      GameplayConfig::playerMeleeDamage,
      GameplayConfig::playerMeleeCooldown,
      GameplayConfig::playerMeleeRange,
      GameplayConfig::playerMeleeHalfWidth,
      GameplayConfig::meleeKnockback));

  silbon = std::make_unique<Silbon>(
      QVector2D(870.0f, GameplayConfig::levelGroundY),
      [this](Projectile projectile) { projectiles.push_back(std::move(projectile)); });
  tureco = std::make_unique<Tureco>(
      QVector2D(870.0f + GameplayConfig::turecoAttachedOffset,
                GameplayConfig::levelGroundY));
}

void Level2GameWidget::updateGame() {
  if (gameFinished || !player || !silbon || !tureco) return;

  const float dt = GameplayConfig::gameTickSeconds;
  const std::vector<Character*> targets{player.get(), silbon.get(), tureco.get()};

  QVector2D movement;
  if (moveLeft) movement.setX(movement.x() - 1.0f);
  if (moveRight) movement.setX(movement.x() + 1.0f);
  player->move(movement);

  silbon->update(dt, targets, player->getPosition(), *tureco);
  tureco->update(dt, *silbon, *player, targets);
  player->update(dt, targets);

  const float playerX = std::clamp(player->getPosition().x(), 42.0f,
                                   static_cast<float>(width()) - 42.0f);
  player->setPosition(QVector2D(playerX, player->getPosition().y()));
  const float silbonX = std::clamp(silbon->getPosition().x(), 42.0f,
                                   static_cast<float>(width()) - 42.0f);
  silbon->setPosition(QVector2D(silbonX, silbon->getPosition().y()));

  for (Projectile& projectile : projectiles) projectile.update(dt, targets);
  std::erase_if(projectiles, [](const Projectile& projectile) { return !projectile.isActive(); });

  if (!silbon->getIsActive()) {
    gameFinished = true;
    playerWon = true;
  } else if (!player->getIsActive() || !tureco->getIsActive()) {
    gameFinished = true;
    playerWon = false;
  }

  update();
}

void Level2GameWidget::drawHealthBar(QPainter& painter,
                                     QRectF rect,
                                     int health,
                                     int maxHealth,
                                     const QColor& color) const {
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(15, 14, 18, 220));
  painter.drawRoundedRect(rect, 4.0, 4.0);
  const qreal fraction = maxHealth > 0
      ? std::clamp(static_cast<qreal>(health) / maxHealth, 0.0, 1.0)
      : 0.0;
  rect.adjust(3.0, 3.0, -3.0, -3.0);
  rect.setWidth(rect.width() * fraction);
  painter.setBrush(color);
  painter.drawRoundedRect(rect, 3.0, 3.0);
}

void Level2GameWidget::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.fillRect(rect(), QColor(27, 25, 27));

  if (!background.isNull()) {
    painter.drawImage(rect(), background);
  }

  const qreal scaleX = width() / 1280.0;
  const qreal scaleY = height() / 720.0;
  const auto screenPoint = [scaleX, scaleY](QVector2D pos) {
    return QPointF(pos.x() * scaleX, pos.y() * scaleY);
  };
  const auto screenRect = [scaleX, scaleY](QVector2D pos, qreal w, qreal h) {
    return QRectF(pos.x() * scaleX - w * scaleX / 2.0,
                  pos.y() * scaleY - h * scaleY,
                  w * scaleX,
                  h * scaleY);
  };

  const QRectF floorLine(0.0, GameplayConfig::levelGroundY * scaleY,
                         width(), 4.0 * scaleY);
  painter.fillRect(floorLine, QColor(42, 30, 22, 200));

  if (silbon && silbon->getIsActive()) {
    const QPointF point = screenPoint(silbon->getPosition());
    const QRectF body = screenRect(silbon->getPosition(), 58.0,
                                   GameplayConfig::silbonBodyHeight);
    painter.setPen(QPen(QColor(35, 16, 16), 3.0));
    painter.setBrush(QColor(171, 58, 42));
    painter.drawRect(body);
    painter.setPen(Qt::white);
    painter.drawText(QPointF(point.x() - 28.0, body.top() - 12.0), QStringLiteral("Silbón"));
    drawHealthBar(painter, QRectF(body.left(), body.top() - 8.0, body.width(), 6.0),
                  silbon->getHp(), silbon->getMaxHp(), QColor(199, 58, 48));
    painter.setPen(Qt::white);
    painter.drawText(QPointF(body.left(), body.top() - 16.0),
                     QStringLiteral("%1 / %2").arg(silbon->getHp()).arg(silbon->getMaxHp()));
  }

  if (tureco && tureco->getIsActive()) {
    const QPointF point = screenPoint(tureco->getPosition());
    const QRectF body = screenRect(tureco->getPosition(), 44.0, 58.0);
    painter.setPen(QPen(QColor(27, 31, 25), 3.0));
    painter.setBrush(QColor(111, 142, 69));
    painter.drawRect(body);
    painter.setPen(Qt::white);
    painter.drawText(QPointF(point.x() - 25.0, body.top() - 10.0), QStringLiteral("Tureco"));
    drawHealthBar(painter, QRectF(body.left(), body.top() - 6.0, body.width(), 5.0),
                  tureco->getHp(), tureco->getMaxHp(), QColor(133, 177, 78));
  }

  if (player && player->getIsActive()) {
    const QRectF body = screenRect(player->getPosition(), 76.0, 112.0);
    painter.save();
    if (!playerSpriteSheet.isNull()) {
      const QRect source(320, 0, 320, 256);
      if (player->getFacingVector().x() < 0.0f) {
        painter.translate(body.center().x() * 2.0, 0.0);
        painter.scale(-1.0, 1.0);
      }
      painter.drawImage(body, playerSpriteSheet, source);
    } else {
      painter.setPen(QPen(Qt::black, 2.0));
      painter.setBrush(QColor(198, 139, 78));
      painter.drawRect(body);
    }
    painter.restore();
    drawHealthBar(painter,
                  QRectF(26.0, height() - 34.0, 230.0, 18.0),
                  player->getHp(), player->getMaxHp(), QColor(180, 61, 48));
  }

  for (const Projectile& projectile : projectiles) {
    if (!projectile.isActive()) continue;
    const QPointF point = screenPoint(projectile.getPosition());
    const qreal radius = projectile.getRadius() * scaleX;
    painter.setPen(QPen(QColor(43, 26, 19), 2.0));
    painter.setBrush(QColor(219, 190, 128));
    painter.drawEllipse(point, radius, radius);
  }

  painter.setPen(Qt::white);
  painter.setFont(QFont(QStringLiteral("Arial"), 12, QFont::Bold));
  painter.drawText(QPointF(24.0, 30.0),
                   QStringLiteral("NIVEL 2  |  A/D mover  Espacio saltar  L atacar  K parry  I roll  Esc menú"));

  if (gameFinished) {
    painter.fillRect(rect(), QColor(10, 10, 13, 190));
    painter.setPen(playerWon ? QColor(235, 207, 138) : QColor(233, 121, 98));
    painter.setFont(QFont(QStringLiteral("Arial"), 28, QFont::Bold));
    painter.drawText(rect(), Qt::AlignCenter,
                     playerWon ? QStringLiteral("¡VICTORIA!\nEnter para volver a jugar · Esc para menú")
                               : QStringLiteral("DERROTA\nEnter para reintentar · Esc para menú"));
  }
}

void Level2GameWidget::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_Escape) {
    emit returnToMenu();
    return;
  }
  if (gameFinished) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) restartLevel();
    return;
  }

  switch (event->key()) {
    case Qt::Key_A: moveLeft = true; break;
    case Qt::Key_D: moveRight = true; break;
    case Qt::Key_Space: if (!event->isAutoRepeat()) player->jump(); break;
    case Qt::Key_L: {
      if (event->isAutoRepeat()) break;
      if (tureco->isKicked()) {
        player->attack(player->getFacingVector(), {player.get(), silbon.get(), tureco.get()});
      } else {
        const int silbonHealthBefore = silbon->getHp();
        const bool attackStarted = player->attack(player->getFacingVector(),
                                                   {player.get(), silbon.get()});
        const int sharedDamage = silbonHealthBefore - silbon->getHp();
        if (attackStarted && sharedDamage > 0 && tureco->getIsActive()) {
          tureco->takeDamage(sharedDamage, player.get(), true);
        }
      }
      break;
    }
    case Qt::Key_K: if (!event->isAutoRepeat()) player->executeParry(); break;
    case Qt::Key_I: if (!event->isAutoRepeat()) player->executeRoll(player->getFacingVector()); break;
    default: QWidget::keyPressEvent(event); return;
  }
  event->accept();
}

void Level2GameWidget::keyReleaseEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_A) moveLeft = false;
  else if (event->key() == Qt::Key_D) moveRight = false;
  else QWidget::keyReleaseEvent(event);
}

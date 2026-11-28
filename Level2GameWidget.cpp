#include "Level2GameWidget.h"

#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRectF>
#include <QPen>
#include <QtMath>

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <vector>

#include "BoxAttack.h"
#include "GameplayConfig.h"
#include "LateralPhysicsMotor.h"
#include "LateralPlayer.h"
#include "Projectile.h"
#include "Silbon.h"
#include "Tureco.h"

namespace {
const std::array<QRectF, 2> LEVEL2_PLATFORMS{
    QRectF(GameplayConfig::level2PlatformOneX, GameplayConfig::level2PlatformOneY,
           GameplayConfig::level2PlatformOneWidth, GameplayConfig::level2PlatformThickness),
    QRectF(GameplayConfig::level2PlatformTwoX, GameplayConfig::level2PlatformTwoY,
           GameplayConfig::level2PlatformTwoWidth, GameplayConfig::level2PlatformThickness)};
}

Level2GameWidget::Level2GameWidget(QWidget* parent)
    : QWidget(parent),
      background(QStringLiteral(":/assets/level2_background.png")),
      playerSpriteSheet(QStringLiteral(":/assets/player_spritesheet.png")),
      jumpSprite(QStringLiteral(":/assets/otoniel_jump.png")),
      ghostSilbonBoneSpriteSheet(QStringLiteral(":/assets/ghost_silbon_bone_spritesheet.png")),
      turecoSpriteSheet(QStringLiteral(":/assets/tureco_spritesheet.png")) {
  setFocusPolicy(Qt::StrongFocus);
  setMinimumSize(900, 560);
  timer.setTimerType(Qt::PreciseTimer);
  timer.setInterval(16);
  connect(&timer, &QTimer::timeout, this, &Level2GameWidget::updateGame);
  restartLevel();
  timer.start();
}

Level2GameWidget::~Level2GameWidget() = default;

void Level2GameWidget::setDifficulty(Level2Difficulty newDifficulty) {
  if (difficulty == newDifficulty) return;
  difficulty = newDifficulty;
  restartLevel();
}

void Level2GameWidget::setPaused(bool paused) {
  if (paused) timer.stop();
  else if (!timer.isActive()) timer.start();
}

void Level2GameWidget::restartLevel() {
  projectiles.clear();
  gameFinished = false;
  playerWon = false;
  playerAnimation = PlayerAnimation::Idle;
  playerAnimationTime = 0.0f;
  attackAnimationTimer = 0.0f;
  silbonAnimationTime = 0.0f;
  turecoAnimationTime = 0.0f;
  turecoFacing = 1.0f;
  turecoIsMoving = false;
  moveLeft = false;
  moveRight = false;
  blockHeld = false;

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
  player->getLateralPhysics()->setPlatforms(
      std::vector<QRectF>(LEVEL2_PLATFORMS.begin(), LEVEL2_PLATFORMS.end()));
  player->setPrimaryAttack(std::make_unique<BoxAttack>(
      GameplayConfig::playerMeleeDamage,
      GameplayConfig::playerMeleeCooldown,
      GameplayConfig::playerMeleeRange,
      GameplayConfig::level2PlayerMeleeHalfWidth,
      GameplayConfig::meleeKnockback));

  const bool hardMode = difficulty == Level2Difficulty::Hard;
  silbon = std::make_unique<Silbon>(
      QVector2D(870.0f, GameplayConfig::levelGroundY),
      [this](Projectile projectile) { projectiles.push_back(std::move(projectile)); },
      hardMode ? GameplayConfig::hardSilbonMeleeDamage : GameplayConfig::silbonMeleeDamage,
      hardMode ? GameplayConfig::hardBoneDamage : GameplayConfig::boneDamage,
      hardMode ? 3 : 1);
  tureco = std::make_unique<Tureco>(
      QVector2D(870.0f + GameplayConfig::turecoAttachedOffset,
                GameplayConfig::levelGroundY),
      hardMode ? GameplayConfig::hardTurecoDamage : GameplayConfig::turecoDamage);
}

void Level2GameWidget::updateGame() {
  if (gameFinished || !player || !silbon || !tureco) return;

  const float dt = GameplayConfig::gameTickSeconds;
  const std::vector<Character*> targets{player.get(), silbon.get(), tureco.get()};

  player->setBlocking(blockHeld);
  QVector2D movement;
  if (moveLeft) movement.setX(movement.x() - 1.0f);
  if (moveRight) movement.setX(movement.x() + 1.0f);
  player->move(movement);

  silbon->update(dt, targets, player->getPosition(), *tureco);
  silbonAnimationTime += dt;
  const float previousTurecoX = tureco->getPosition().x();
  tureco->update(dt, *silbon, *player, targets);
  const float turecoDeltaX = tureco->getPosition().x() - previousTurecoX;
  turecoIsMoving = std::abs(turecoDeltaX) > 0.1f;
  if (turecoIsMoving) {
    turecoFacing = turecoDeltaX < 0.0f ? -1.0f : 1.0f;
  }
  turecoAnimationTime += dt;
  player->update(dt, targets);
  updatePlayerAnimation(dt);

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

void Level2GameWidget::updatePlayerAnimation(float dt) {
  if (!player) return;

  attackAnimationTimer = std::max(0.0f, attackAnimationTimer - dt);

  PlayerAnimation nextAnimation = PlayerAnimation::Idle;
  if (player->isRolling()) {
    nextAnimation = PlayerAnimation::Roll;
  } else if (player->isBlocking() || player->isParryBusy()) {
    nextAnimation = PlayerAnimation::Parry;
  } else if (attackAnimationTimer > 0.0f) {
    nextAnimation = PlayerAnimation::Attack;
  } else if (!player->isGrounded()) {
    nextAnimation = PlayerAnimation::Jump;
  } else if (moveLeft || moveRight) {
    nextAnimation = PlayerAnimation::Run;
  }

  if (nextAnimation != playerAnimation) {
    playerAnimation = nextAnimation;
    playerAnimationTime = 0.0f;
  } else {
    playerAnimationTime += dt;
  }
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

  if (silbon && silbon->getIsActive()) {
    const QRectF bossBarRect(width() - 310.0, 16.0, 286.0, 18.0);
    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Arial"), 11, QFont::Bold));
    painter.drawText(QRectF(bossBarRect.x(), 0.0, bossBarRect.width(), 15.0),
                     Qt::AlignRight | Qt::AlignVCenter,
                     QStringLiteral("SILBÓN  %1 / %2")
                         .arg(silbon->getHp()).arg(silbon->getMaxHp()));
    drawHealthBar(painter, bossBarRect, silbon->getHp(), silbon->getMaxHp(),
                  QColor(199, 58, 48));
  }
  const QRectF floorLine(0.0, GameplayConfig::levelGroundY * scaleY,
                         width(), 4.0 * scaleY);
  painter.fillRect(floorLine, QColor(42, 30, 22, 200));

  painter.save();
  painter.setPen(QPen(QColor(39, 27, 20), 3.0));
  for (const QRectF& platform : LEVEL2_PLATFORMS) {
    const QRectF ledge(platform.x() * scaleX, platform.y() * scaleY,
                       platform.width() * scaleX, platform.height() * scaleY);
    painter.setBrush(QColor(92, 66, 43, 245));
    painter.drawRoundedRect(ledge, 4.0, 4.0);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(151, 111, 68, 245));
    painter.drawRect(QRectF(ledge.left() + 2.0, ledge.top() + 2.0,
                            ledge.width() - 4.0, std::max(3.0, ledge.height() * 0.28)));
    painter.setPen(QPen(QColor(39, 27, 20), 3.0));
  }
  painter.restore();

  if (silbon && silbon->getIsActive()) {
    const QRectF hitbox = screenRect(silbon->getPosition(), 58.0,
                                     GameplayConfig::silbonBodyHeight);
    const std::array<QRect, 4> silbonFrames{
        QRect(28, 704, 288, 644), QRect(376, 704, 292, 644),
        QRect(704, 704, 284, 644), QRect(1052, 704, 288, 644)};
    const bool silbonMoving = std::abs(silbon->getMoveVelocity().x()) > 1.0f;
    const int frame = silbonMoving
        ? static_cast<int>(silbonAnimationTime / 0.14f) %
              static_cast<int>(silbonFrames.size())
        : 0;
    const QRectF spriteRect = screenRect(silbon->getPosition(), 116.0, 260.0);

    painter.save();
    if (ghostSilbonBoneSpriteSheet.isNull()) {
      painter.setPen(QPen(QColor(35, 16, 16), 3.0));
      painter.setBrush(QColor(171, 58, 42));
      painter.drawRect(hitbox);
    } else {
      if (silbon->getFacingVector().x() < 0.0f) {
        painter.translate(spriteRect.center().x() * 2.0, 0.0);
        painter.scale(-1.0, 1.0);
      }
      painter.drawImage(spriteRect, ghostSilbonBoneSpriteSheet, silbonFrames[frame]);
    }
    painter.restore();
  }

  if (tureco && tureco->getIsActive()) {
    const QPointF point = screenPoint(tureco->getPosition());
    const QRectF body = screenRect(tureco->getPosition(), 120.0, 82.0);
    painter.save();
    if (turecoSpriteSheet.isNull()) {
      painter.setPen(QPen(QColor(27, 31, 25), 3.0));
      painter.setBrush(QColor(111, 142, 69));
      painter.drawRect(body);
    } else {
      const bool charging = tureco->isKicked();
      const int row = charging ? 1 : 0;
      const bool walking = turecoIsMoving && !charging;
      const int frame = (charging || walking)
          ? static_cast<int>(turecoAnimationTime / 0.09f) % 4
          : 0;
      const QRect source(frame * 512, row * 340, 512, 340);
      const bool spriteFacesRight = !charging;
      const bool shouldFlip = spriteFacesRight ? turecoFacing < 0.0f
                                               : turecoFacing > 0.0f;
      if (shouldFlip) {
        painter.translate(body.center().x() * 2.0, 0.0);
        painter.scale(-1.0, 1.0);
      }
      painter.drawImage(body, turecoSpriteSheet, source);
    }
    painter.restore();
    painter.setPen(Qt::white);
    painter.drawText(QPointF(point.x() - 25.0, body.top() - 10.0), QStringLiteral("Tureco"));
    drawHealthBar(painter, QRectF(body.left(), body.top() - 6.0, body.width(), 5.0),
                  tureco->getHp(), tureco->getMaxHp(), QColor(133, 177, 78));
  }

  if (player && player->getIsActive()) {
    const QRectF body = screenRect(player->getPosition(), 76.0, 112.0);
    painter.save();
    if (!playerSpriteSheet.isNull()) {
      constexpr int cellWidth = 320;
      constexpr int cellHeight = 256;
      int row = 0;
      int column = 1;

      switch (playerAnimation) {
        case PlayerAnimation::Idle:
          row = 0;
          column = 1;
          break;
        case PlayerAnimation::Jump:
          row = 0;
          column = 1;
          break;
        case PlayerAnimation::Run:
          row = 7;
          column = static_cast<int>(playerAnimationTime / 0.10f) % 6;
          break;
        case PlayerAnimation::Attack:
          row = 1;
          column = 1 + (playerAnimationTime >= 0.12f ? 1 : 0);
          break;
        case PlayerAnimation::Parry:
          row = 6;
          column = 0;
          break;
        case PlayerAnimation::Roll:
          row = 5;
          column = std::clamp(static_cast<int>(playerAnimationTime /
                                               GameplayConfig::playerRollDuration * 6.0f),
                              0, 5);
          break;
      }

      const QRect source(column * cellWidth, row * cellHeight, cellWidth, cellHeight);
      const qreal spriteHeight = body.height();
      const qreal spriteWidth = spriteHeight * cellWidth / cellHeight;
      const QRectF spriteRect(body.center().x() - spriteWidth * 0.5,
                              body.bottom() - spriteHeight, spriteWidth, spriteHeight);
      if (player->getFacingVector().x() < 0.0f) {
        painter.translate(body.center().x() * 2.0, 0.0);
        painter.scale(-1.0, 1.0);
      }
      if (playerAnimation == PlayerAnimation::Jump && !jumpSprite.isNull()) {
        const qreal jumpHeight = body.height();
        const qreal jumpWidth = jumpHeight * jumpSprite.width() / jumpSprite.height();
        const QRectF jumpRect(body.center().x() - jumpWidth * 0.5,
                              body.bottom() - jumpHeight, jumpWidth, jumpHeight);
        painter.drawImage(jumpRect, jumpSprite);
      } else {
        painter.drawImage(spriteRect, playerSpriteSheet, source);
      }
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
    if (!ghostSilbonBoneSpriteSheet.isNull()) {
      painter.save();
      painter.translate(point);
      painter.rotate(qRadiansToDegrees(qAtan2(projectile.getVelocity().y(),
                                               projectile.getVelocity().x())));
      painter.drawImage(QRectF(-19.0 * scaleX, -19.0 * scaleY,
                               38.0 * scaleX, 38.0 * scaleY),
                        ghostSilbonBoneSpriteSheet, QRect(20, 1552, 308, 316));
      painter.restore();
    } else {
      const qreal radius = projectile.getRadius() * scaleX;
      painter.setPen(QPen(QColor(43, 26, 19), 2.0));
      painter.setBrush(QColor(219, 190, 128));
      painter.drawEllipse(point, radius, radius);
    }
  }

  painter.setPen(Qt::white);
  painter.setFont(QFont(QStringLiteral("Arial"), 12, QFont::Bold));
  painter.drawText(QPointF(24.0, 30.0),
                   QStringLiteral("NIVEL 2  |  A/D mover  Espacio saltar  L atacar  K bloquear (mantener)  I roll  Esc menú"));

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
      bool attackStarted = false;
      if (tureco->isKicked()) {
        attackStarted = player->attack(player->getFacingVector(),
                                       {player.get(), silbon.get(), tureco.get()});
      } else {
        const int silbonHealthBefore = silbon->getHp();
        attackStarted = player->attack(player->getFacingVector(),
                                       {player.get(), silbon.get()});
        const int sharedDamage = silbonHealthBefore - silbon->getHp();
        if (attackStarted && sharedDamage > 0 && tureco->getIsActive()) {
          tureco->takeDamage(sharedDamage, player.get(), true);
        }
      }
      if (attackStarted) attackAnimationTimer = 0.24f;
      break;
    }
    case Qt::Key_K: if (!event->isAutoRepeat()) blockHeld = true; break;
    case Qt::Key_I: if (!event->isAutoRepeat()) player->executeRoll(player->getFacingVector()); break;
    default: QWidget::keyPressEvent(event); return;
  }
  event->accept();
}

void Level2GameWidget::keyReleaseEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key_A) moveLeft = false;
  else if (event->key() == Qt::Key_D) moveRight = false;
  else if (event->key() == Qt::Key_K) blockHeld = false;
  else QWidget::keyReleaseEvent(event);
}











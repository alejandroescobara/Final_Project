#pragma once

#include <QImage>
#include <QPainter>
#include <QTimer>
#include <QWidget>

#include <memory>
#include <vector>

class LateralPlayer;
class Projectile;
class Silbon;
class Tureco;

class Level2GameWidget final : public QWidget {
  Q_OBJECT

  private:
    QTimer timer;
    QImage background;
    QImage playerSpriteSheet;
    std::unique_ptr<LateralPlayer> player;
    std::unique_ptr<Silbon> silbon;
    std::unique_ptr<Tureco> tureco;
    std::vector<Projectile> projectiles;
    bool moveLeft = false;
    bool moveRight = false;
    bool gameFinished = false;
    bool playerWon = false;

    void restartLevel();
    void updateGame();
    void drawHealthBar(QPainter& painter, QRectF rect, int health, int maxHealth,
                       const QColor& color) const;

  signals:
    void returnToMenu();

  protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

  public:
    explicit Level2GameWidget(QWidget* parent = nullptr);
    ~Level2GameWidget() override;
    void setPaused(bool paused);
};

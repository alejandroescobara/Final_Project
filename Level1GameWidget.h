#pragma once

#include <QElapsedTimer>
#include <QPainter>
#include <QPointF>
#include <QTimer>
#include <QVector2D>
#include <QWidget>

#include "PlanarWorld.h"

class Level1GameWidget final : public QWidget {
  Q_OBJECT

  private:
    QTimer timer;
    QElapsedTimer frameTimer;
    PlanarWorld world;

    bool moveUp = false;
    bool moveDown = false;
    bool moveLeft = false;
    bool moveRight = false;

    bool attackQueued = false;
    bool rollQueued = false;
    bool parryQueued = false;

    QPointF lastMousePos;
    bool hasMouseAim = false;

    float attackFlashTimer = 0.0f;
    QVector2D lastAttackDir{1.0f, 0.0f};
    bool bossTransitionEmitted = false;

    void clearInputState();
    void updateGame();

    QPointF computeCameraOrigin() const;
    QVector2D screenToWorld(QPointF screenPos) const;

    void drawWorld(QPainter& painter, QPointF cameraOrigin) const;
    void drawHud(QPainter& painter) const;
    void drawOverlay(QPainter& painter, const QString& title, const QString& subtitle) const;
    void drawBar(QPainter& painter,
                 QRectF rect,
                 int value,
                 int maxValue,
                 const QColor& fillColor) const;

  signals:
    void returnToMenu();
    void enterBossFight();

  protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

  public:
    explicit Level1GameWidget(QWidget* parent = nullptr);
    ~Level1GameWidget() override;

    void setPaused(bool paused);
    void restartLevel();
};

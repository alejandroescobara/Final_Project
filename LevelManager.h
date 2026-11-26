#pragma once

#include <QWidget>

class QStackedWidget;
class Level1GameWidget;
class Level2GameWidget;

enum class GameLevel { LevelOne, LevelTwo };

class LevelManager final : public QWidget {
  Q_OBJECT

  private:
    QStackedWidget* pages;
    QWidget* menuPage;
    Level1GameWidget* levelOnePage;
    Level2GameWidget* levelTwoPage;

    QWidget* createMenuPage();
    void launchLevel(GameLevel level);
    void showMenu();

  public:
    explicit LevelManager(QWidget* parent = nullptr);
};

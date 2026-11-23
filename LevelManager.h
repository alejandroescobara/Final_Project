#pragma once

#include <QWidget>

class QStackedWidget;
class Level2GameWidget;

enum class GameLevel { LevelOne, LevelTwo };

class LevelManager final : public QWidget {
  Q_OBJECT

  private:
    QStackedWidget* pages;
    QWidget* menuPage;
    QWidget* levelOnePage;
    Level2GameWidget* levelTwoPage;

    QWidget* createMenuPage();
    QWidget* createLevelOnePage();
    void launchLevel(GameLevel level);
    void showMenu();

  public:
    explicit LevelManager(QWidget* parent = nullptr);
};

#include "LevelManager.h"

#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "Level2GameWidget.h"

LevelManager::LevelManager(QWidget* parent)
    : QWidget(parent),
      pages(new QStackedWidget(this)),
      menuPage(createMenuPage()),
      levelOnePage(createLevelOnePage()),
      levelTwoPage(new Level2GameWidget(this)) {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(pages);
  pages->addWidget(menuPage);
  pages->addWidget(levelOnePage);
  pages->addWidget(levelTwoPage);
  connect(levelTwoPage, &Level2GameWidget::returnToMenu, this, &LevelManager::showMenu);
  showMenu();
}

QWidget* LevelManager::createMenuPage() {
  auto* page = new QWidget(this);
  page->setStyleSheet(QStringLiteral("background:#211d1b; color:#f5e9d1;"));
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(120, 70, 120, 70);
  layout->setSpacing(18);

  auto* title = new QLabel(QStringLiteral("EL CAMINO DEL ERRANTE"), page);
  title->setAlignment(Qt::AlignCenter);
  title->setStyleSheet(QStringLiteral("font-size:34px; font-weight:700; color:#e7c783;"));
  auto* subtitle = new QLabel(QStringLiteral("Selecciona un nivel"), page);
  subtitle->setAlignment(Qt::AlignCenter);
  subtitle->setStyleSheet(QStringLiteral("font-size:18px;"));

  auto* levelOneButton = new QPushButton(QStringLiteral("Nivel 1 · Vista en plano"), page);
  auto* levelTwoButton = new QPushButton(QStringLiteral("Nivel 2 · Silbón y Tureco"), page);
  const QString buttonStyle = QStringLiteral(
      "QPushButton{background:#70432f;color:#fff3df;border:2px solid #c3995d;"
      "border-radius:8px;padding:14px;font-size:18px;}"
      "QPushButton:hover{background:#8c5638;}");
  levelOneButton->setStyleSheet(buttonStyle);
  levelTwoButton->setStyleSheet(buttonStyle);

  auto* note = new QLabel(QStringLiteral(
      "El Nivel 2 ya incluye movimiento lateral, combate y el jefe.\n"
      "El Nivel 1 queda enlazado al sistema; su escena Qt aún no está en este proyecto."), page);
  note->setAlignment(Qt::AlignCenter);
  note->setWordWrap(true);
  note->setStyleSheet(QStringLiteral("color:#c9bca8;font-size:14px;"));

  layout->addStretch();
  layout->addWidget(title);
  layout->addWidget(subtitle);
  layout->addSpacing(20);
  layout->addWidget(levelOneButton);
  layout->addWidget(levelTwoButton);
  layout->addWidget(note);
  layout->addStretch();

  connect(levelOneButton, &QPushButton::clicked, this,
          [this] { launchLevel(GameLevel::LevelOne); });
  connect(levelTwoButton, &QPushButton::clicked, this,
          [this] { launchLevel(GameLevel::LevelTwo); });
  return page;
}

QWidget* LevelManager::createLevelOnePage() {
  auto* page = new QWidget(this);
  page->setStyleSheet(QStringLiteral("background:#211d1b; color:#f5e9d1;"));
  auto* layout = new QVBoxLayout(page);
  auto* title = new QLabel(QStringLiteral("Nivel 1 · Vista en plano"), page);
  title->setAlignment(Qt::AlignCenter);
  title->setStyleSheet(QStringLiteral("font-size:28px;font-weight:700;color:#e7c783;"));
  auto* details = new QLabel(QStringLiteral(
      "La base del nivel en plano existe en las clases de física y del agente,\n"
      "pero este repositorio todavía no tiene su escena de juego conectada a Qt."), page);
  details->setAlignment(Qt::AlignCenter);
  details->setWordWrap(true);
  auto* backButton = new QPushButton(QStringLiteral("Volver al menú"), page);
  layout->addStretch();
  layout->addWidget(title);
  layout->addWidget(details);
  layout->addWidget(backButton, 0, Qt::AlignHCenter);
  layout->addStretch();
  connect(backButton, &QPushButton::clicked, this, &LevelManager::showMenu);
  return page;
}

void LevelManager::launchLevel(GameLevel level) {
  if (level == GameLevel::LevelOne) {
    levelTwoPage->setPaused(true);
    pages->setCurrentWidget(levelOnePage);
  } else {
    levelTwoPage->setPaused(false);
    pages->setCurrentWidget(levelTwoPage);
    levelTwoPage->setFocus();
  }
}

void LevelManager::showMenu() {
  levelTwoPage->setPaused(true);
  pages->setCurrentWidget(menuPage);
}

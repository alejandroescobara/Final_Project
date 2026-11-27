#include "GameAudio.h"

#include <QMediaPlayer>
#include <QUrl>
#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioOutput>
#else
#include <QMediaContent>
#endif

GameAudio::GameAudio() {
  initializeVoices(damageVoices, "qrc:/assets/player_damage.mp3", 0.60f);
  initializeVoices(attackVoices, "qrc:/assets/player_attack.mp3", 0.48f);
}

GameAudio::~GameAudio() = default;

void GameAudio::initializeVoices(std::array<Voice, voiceCount>& voices,
                                 const char* resourcePath,
                                 float volume) {
  for (Voice& voice : voices) {
    voice.player = std::make_unique<QMediaPlayer>();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    voice.output = std::make_unique<QAudioOutput>();
    voice.output->setVolume(volume);
    voice.player->setAudioOutput(voice.output.get());
    voice.player->setSource(QUrl(QString::fromUtf8(resourcePath)));
#else
    voice.player->setVolume(static_cast<int>(volume * 100.0f));
    voice.player->setMedia(QMediaContent(QUrl(QString::fromUtf8(resourcePath))));
#endif
  }
}

void GameAudio::play(std::array<Voice, voiceCount>& voices, int& nextVoice) {
  Voice& voice = voices[nextVoice];
  nextVoice = (nextVoice + 1) % voiceCount;
  voice.player->setPosition(0);
  voice.player->play();
}

void GameAudio::playDamage() {
  play(damageVoices, nextDamageVoice);
}

void GameAudio::playAttack() {
  play(attackVoices, nextAttackVoice);
}

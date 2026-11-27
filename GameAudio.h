#pragma once

#include <array>
#include <memory>

#include <QtGlobal>

class QMediaPlayer;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class QAudioOutput;
#endif

class GameAudio final {
  private:
    struct Voice {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
      std::unique_ptr<QAudioOutput> output;
#endif
      std::unique_ptr<QMediaPlayer> player;
    };

    static constexpr int voiceCount = 4;
    std::array<Voice, voiceCount> damageVoices;
    std::array<Voice, voiceCount> attackVoices;
    int nextDamageVoice = 0;
    int nextAttackVoice = 0;

    static void initializeVoices(std::array<Voice, voiceCount>& voices,
                                 const char* resourcePath,
                                 float volume);
    static void play(std::array<Voice, voiceCount>& voices, int& nextVoice);

  public:
    GameAudio();
    ~GameAudio();

    void playDamage();
    void playAttack();
};

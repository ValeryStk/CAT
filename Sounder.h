#ifndef SOUNDER_H
#define SOUNDER_H

#include <QObject>
#include <Windows.h>
#include <QMediaPlayer>
#include <QMediaPlaylist>

enum SoundIndex{

    blackFounded,
    keyPressed,
    problemMessage,
    shortAlert,
    Myau
};



class Sounder:public QObject
{
    Q_OBJECT
public:
    Sounder();

private slots:
  void playSound(QString sampleName);
  void changeSoundState(bool isOn);

private:
  bool isSoundOn;
  QMediaPlayer        *m_player;
  QMediaPlaylist      *m_playlist;
  QStringList mySounds;

};

#endif // SOUNDER_H

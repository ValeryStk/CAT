#include "Sounder.h"
#include "QDir"

Sounder::Sounder() {
  isSoundOn = false;
  m_player = new QMediaPlayer(this);
  m_playlist = new QMediaPlaylist(m_player);
  m_player->setPlaylist(m_playlist);
  m_player->setVolume(70);
  m_playlist->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);

  QDir dir(":/sounds");
  mySounds = dir.entryList();
  for (int i = 0; i < mySounds.count(); ++i) {
    QString url = "qrc:/sounds/" + mySounds.at(i);
    m_playlist->addMedia(QUrl(url));
    qDebug() << "Sounds at 0:" << ":/sounds/" + mySounds.at(i);
  }

}

void Sounder::playSound(QString sampleName) {
  if (!isSoundOn && sampleName != "noSound.mp3")
    return;
  int index;
  index = std::distance(mySounds.begin(), std::find(mySounds.begin(), mySounds.end(), sampleName));
  m_playlist->setCurrentIndex(index);
  m_player->play();
}

void Sounder::changeSoundState(bool isOn) {
  isSoundOn = isOn;
}


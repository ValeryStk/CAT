#ifndef ENVIMODULE_H
#define ENVIMODULE_H
#include <QtQuick>
#include <QObject>
#include <QSettings>
#include <QQuickPaintedItem>
#include <QVector>
#include <EnviReader.h>
#include <QStringListModel>
#include <Version.h>
#include <QPoint>
#include <Sounder.h>
#include "common_types.h"


//!
//!\brief Класса, отвечающий за взаимодействие QML и С++
//!


class EnviModule: public QQuickPaintedItem {
  Q_OBJECT
  Q_PROPERTY(QImage m_image MEMBER m_image NOTIFY pictureWasLoaded)    //!< Свойство для отрисовки изображения в QML
  Q_PROPERTY(int widthImage MEMBER widthImage NOTIFY sizeChanged)      //!< Свойство ширины изображения
  Q_PROPERTY(int heightImage MEMBER heightImage NOTIFY sizeChanged)    //!< Свойство высоты изображения
  Q_PROPERTY(QString homePath MEMBER homePath NOTIFY pathChanged)
  Q_PROPERTY(QString message MEMBER message NOTIFY messageShow)
  Q_PROPERTY(int progress MEMBER progress NOTIFY progressPixelsChanged)
  Q_PROPERTY(int maxProgress MEMBER maxProgress NOTIFY maxProgressChanged)
  Q_PROPERTY(QPoint lTpoint MEMBER lTpoint NOTIFY lTpointChanged)
  Q_PROPERTY(QPoint rBpoint MEMBER rBpoint NOTIFY rBpointChanged)

  Q_PROPERTY(QPoint startPolygonPoint MEMBER startPolygonPoint NOTIFY startPolPointChanged)
  Q_PROPERTY(QPoint lastPolygonPoint MEMBER lastPolygonPoint NOTIFY lastPolPointChanged)

  Q_PROPERTY(float scaleFactor MEMBER scaleFactor NOTIFY scaleWasChanged)
  Q_PROPERTY(bool isFileAbsent MEMBER isFileAbsent NOTIFY fileImageDataAbsent)
  Q_PROPERTY(bool isSelectMode MEMBER isSelectMode NOTIFY selectModeChanged)
  Q_PROPERTY(QImage zoomedImage MEMBER zoomedImage NOTIFY zoomedImageChanged)
  Q_PROPERTY(QStringList channelsNames MEMBER channelsNames NOTIFY channelsNamesChanged)
  Q_PROPERTY(QStringList headerInfo MEMBER headerInfo NOTIFY headerInfoUpdated)
  Q_PROPERTY(QString version MEMBER version NOTIFY versionChanged)
  Q_PROPERTY(bool isBandsUpdated MEMBER isBandsUpdated NOTIFY bandsWereChanged)
  Q_PROPERTY(float multic MEMBER multic NOTIFY multicWasChanged)
  Q_PROPERTY(QString pathDark MEMBER pathDark NOTIFY darkPathWasChanged)

 public:
  enum Pages { //!< Перечисление страниц интерфейса пользователя
    StartPage, SettingsPage, EnviPage, AboutPage
  };
  Q_ENUM(Pages)

  enum Channel { //!< Перечисление названий основных каналов
    RED, GREEN, BLUE, IR
  };
  Q_ENUM(Channel)


  Q_ENUM(SoundIndex)

  enum ModeChooser {
    View, SelectLomanArea, SelectRectArea
  };
  Q_ENUM(ModeChooser)

  EnviModule();           //!<Конструктор класса
  ~EnviModule() override; //!<Деструктор класса


 public:
  void paint(QPainter* painter)override;
  Q_INVOKABLE QString homeEnviPath() const;
  Q_INVOKABLE void silentClearAreas();
  Q_INVOKABLE QPoint dp() const;
  Q_INVOKABLE int getChannelsNumber() const;
  Q_INVOKABLE QString darkPointInfo() const;
  Q_INVOKABLE void setMode(const EnviModule::ModeChooser mode);
  Q_INVOKABLE EnviModule::ModeChooser getMode() const;
  Q_INVOKABLE bool getIsLomanStarted() const;
  Q_INVOKABLE QPoint beginPolPoint() const;

 private:
  QString version = VER_PRODUCTVERSION_STR;
  ModeChooser m_mode;
  QSettings* m_settings;
  QString homePath;
  QString message;
  QString pathDark;
  QStringList headerInfo;
  EnviReader m_envi;
  QVector <QRect>* m_areas;
  QVector <QPolygon>m_ignorePolygons;
  QVector <QLine>* m_polygon;
  QImage m_image;
  QImage zoomedImage;
  int widthImage;
  int heightImage;
  float scaleFactor;
  float multic;
  int progress;
  int maxProgress;
  int channelsNumber;
  bool isSelectMode;
  bool isFileAbsent;
  bool isDarkMarker;
  bool isBandsUpdated;
  bool isLomanStarted;
  bool isIgnorePolygonReady;
  QPoint lTpoint;
  QPoint rBpoint;
  QPoint startPolygonPoint;
  QPoint lastPolygonPoint;
  DarkPoint m_dp;
  result_values m_calculeted_values;
  QTimer progressTimer;
  QThread* mThread;
  QThread beeperThread;
  Sounder m_sounder;
  QStringList channelsNames;
  void initializeVariables();
  QRect findBoundedRect();
  QRect pointRect;
  QVector<QPoint> m_points;
  QRect lomanBoundedRect;
  QPolygon ignorePolygon;


 private slots:
  void parseHeaderFinished(bool result, QString message);
  void imageWasLoaded(QString path, bool result);
  void imageChannelsChanged(bool result);
  void initProgress(int max);
  void progressChanged();
  void addArea();
  void clearAreas();
  void startSearchingDarkestPixel();
  void taskForDarkSearchingFinished(bool result, DarkPoint dp, QString path);
  void imageWasSavedAsBMP();
  void zoomImageRedy();
  void acceptStartSignalCorrection(float correctMultic);
  void selectAllImageForDarkSearching();
  void addLineToLomanArea();
  void polygonWasCreated();


 signals:
  Q_INVOKABLE void openEnvi(QString path);
  Q_INVOKABLE void changeBands(QVector<bool>);
  Q_INVOKABLE void changeChannels(QVector<QString>);
  Q_INVOKABLE void addAreaToList();
  Q_INVOKABLE void clearAllAreas();
  Q_INVOKABLE void setMultiplicator(float);
  Q_INVOKABLE void findDarkestPixel();
  Q_INVOKABLE void savePicture();
  Q_INVOKABLE void openImagesFolder();
  Q_INVOKABLE void playSound(QString);
  Q_INVOKABLE void changeSoundState(bool);
  Q_INVOKABLE void zoomScaleFactor(float scaleFactor);
  Q_INVOKABLE void selectAllArea();
  Q_INVOKABLE void ignoreTheDarkPixelAndFindNew();
  Q_INVOKABLE void addLine();
  Q_INVOKABLE void endPolygon();

  void pathChanged();
  void pictureWasLoaded();
  void sizeChanged();
  void messageShow();
  void maxProgressChanged();
  void progressPixelsChanged();
  void lTpointChanged();
  void rBpointChanged();
  void startPolPointChanged();
  void lastPolPointChanged();
  void selectModeChanged();
  void zoomedImageChanged();
  void fileImageDataAbsent();
  void channelsNamesChanged();
  void headerInfoUpdated();
  void versionChanged();
  void giveMeTheDarkestPixel(QVector<QRect>*);
  void showDarkPoint();
  void bandsWereChanged();
  void scaleWasChanged();
  void multicWasChanged();
  void darkPathWasChanged();

};

#endif // ENVIMODULE_H

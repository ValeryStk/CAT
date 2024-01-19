#ifndef ENVIREADER_H
#define ENVIREADER_H

#include<QObject>
#include <limits>
#include <QString>
#include <time.h>
#include <QDate>
#include <QTime>
#include <QImage>
#include <QByteArray>
#include <QPoint>
#include <QProcess>
#include <UniversalImageReader.h>
#include "UniversalDarkPixelFinder.h"


enum ReadHeaderResult {

  NoFileExists,
  FileOpeningProblem,
  NoRequairedField,
  ParsingProblem,
  OK
};

enum OpenEnviDataResult {

  HEADER_PROBLEM,
  DATA_FILE_ABSENT,
  DATA_SIZE_PROBLEM
};

enum DataType {
  ByteEnvi = 1,
  IntegerEnvi = 2,
  LongEnvi = 3,
  FloatEnvi = 4,
  DoubleEnvi = 5,
  PairFloatEnvi = 6,
  PairDoubleEnvi = 9,
  UnsignIntegerEnvi = 12,
  UnsignLongIntegerEnvi = 13,
  LongInteger64Envi = 14,
  UnsignLongInteger64Envi = 15
};

enum ByteOrder {

  LSF,
  MSF
};

const QStringList interLeave = {

  "bip",
  "bsq",
  "bil"
};

const QStringList waveLengthUnits = {

  "Micrometers",
  "Nanometers",
  "Millimeters",
  "Centimeters",
  "Meters",
  "Wavenumber",
  "Angstroms",
  "GHz",
  "MHz",
  "Index",
  "Unknown"
};

const QStringList headerParams{

  "acquisition time",
  "band names",
  "bands",
  "bbl",
  "byte order",
  "class lookup",
  "class names",
  "classes",
  "cloud cover",
  "complex function",
  "coordinate system string",
  "data gain values",
  "data ignore value",
  "data offset values",
  "data reflectance gain values",
  "data reflectance offset values",
  "data type",
  "default stretch",
  "dem band",
  "dem file",
  "description",
  "file type",
  "fwhm",
  "geo points",
  "header offset",
  "interleave",
  "lines",
  "map info",
  "pixel size",
  "projection info",
  "read procedures",
  "reflectance scale factor",
  "rpc info",
  "samples",
  "security tag",
  "sensor type",
  "solar irradiance",
  "spectra names",
  "sun azimuth",
  "sun elevation",
  "wavelength",
  "wavelength units",
  "x start",
  "y start",
  "z plot average",
  "z plot range",
  "z plot titles"
};

const QStringList requiredParams{

  "bands",
  "byte order",
  "data type",
  "header offset",
  "interleave",
  "lines",
  "samples"
  //"sun elevation"
};

struct EnviHeaderStruct {

  QString description;
  int samples;
  int lines;
  int bands;
  int headerOffset;
  QString fileType;
  DataType dataType;
  QString interLeave;
  QString sensorType;
  ByteOrder byteOrder;
  QString mapInfo;
  QString coordSystem;
  QString waveLengthUnit;
  QStringList bandNames;
  QVector <double> waveLength;
  QVector <double> fwHm;
  QVector <double> dataGainValues;
  double sunElevationAngle;
};

struct DarkPoint {
  QPoint originPoint;
  QVector<QString>chanelsValues;
  QString info;
  QString solution;
  QString errors;
};
Q_DECLARE_METATYPE(DarkPoint)

class EnviReader : public QObject {
  Q_OBJECT

 public:
  explicit EnviReader();
  ~EnviReader()override;

  bool makeImageWithChannelsSet(QVector<int> channelsSet);
  QImage getImage() const;
  void clearHeader();
  void clearImage();
  int getLoadedPixels() const;
  EnviHeaderStruct getEnviHeader() const;
  QStringList getHdrList() const;
  QImage getDarkImage() const;
  QImage getZoomedImage() const;
  int getScaleFactor() const;
  void setIgnorePolygons(const QVector<QPolygon> value);
  void clearAreasAndIgnores();

 private:
  bool isHeaderOk;
  bool isFileOk;
  int loadedPixels;
  float scale;
  EnviHeaderStruct m_enviHeader;
  QStringList m_hdrList;
  QVector <QPoint> pointIgnoreList;
  QVector <QPolygon> ignorePolygons;
  QVector<QRect>* lastlistRects;
  QPoint lastDarkPoint;

  QByteArray* m_byteArray;
  char* m_enviImageChar;
  float* m_enviImageFloat32;
  uint16_t* m_enviImageUint16;

  QImage m_Image;
  QImage m_darkImage;
  QImage m_zoomedImage;
  float multic;
  bool checkRequiredParameters(QStringList paramsList);
  bool parseParamsHeader(EnviHeaderStruct& enviHdr, QStringList paramsList);
  void singleParamPars(QString& param, QString remove);
  void complexParam(QString& param, QString remove, QStringList& list);
  void complexParam(QString& param, QString remove, QVector<double>& list);
  ReadHeaderResult loadEnviHeader(EnviHeaderStruct& enviHdr, QString filePath);
  int getSizeOfEnviType(DataType dt);
  void saveAnyImage(QImage img, QString folderName);
  void drawAimDarkPointer(QImage img, QPoint darkPoint, QString folderName, QString& file);
  QString darkReport(double minValue, QPoint darkPoint, int offX, int offY, QVector<QString> values);
  void clearIgnoreList();

 private slots:
  bool loadEnviImageData(QString path);
  bool makeImage(int samples, int lines, int bands);
  void setMultiplicator(float multic);
  void findDarkestPixel(QVector<QRect>*);
  void savePicture();
  void openImagesFolder();
  void changeChannelsSet(QVector<QString> channelsInfo);
  void changeZoomForImage(float scaleFactor);
  void addPointToIgnoreListAndSearchAgain();

 signals:
  void headerIsReady(bool, QString);
  void imageReady(QString, bool);
  void channelsChanged(bool);
  void startProgressState(int max);
  void darkPixelTaskFinished(bool, DarkPoint, QString);
  void imageWasSavedAsBmp();
  void zoomWasChanged();
  void correctStartSignal(float);
};

#endif // ENVIREADER_H

#include "EnviReader.h"
#include <algorithm>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QString>
#include <QTextStream>
#include <QTextCodec>
#include <QFileInfo>
#include <QDateTime>
#include <QPainter>

EnviReader::EnviReader() {

  isHeaderOk = false;
  isFileOk = false;
  m_byteArray = new QByteArray;
  m_enviImageChar = nullptr;
  m_enviImageFloat32 = nullptr;
  m_enviImageUint16 = nullptr;
  lastlistRects = nullptr;
  multic = 1;
  scale = 1.0;

}

EnviReader::~EnviReader() {
  delete m_byteArray;
}


ReadHeaderResult EnviReader::loadEnviHeader(EnviHeaderStruct& enviHdr, QString filePath) {
  QFileInfo inf(filePath);
  if (!inf.exists()) {
    return NoFileExists;
  }
  QFile enviHeaderFile(filePath);
  QStringList myParamsList;
  QString complexParam;

  if (enviHeaderFile.open(QIODevice::ReadOnly)) {
    QTextStream in(&enviHeaderFile);

    while (!in.atEnd()) {
      QString line = in.readLine();

      for (int i = 0; i < headerParams.count(); ++i) {
        if (line.contains(headerParams.at(i)) && !line.contains("{")) {

          myParamsList.append(line);
          line.clear();
          break;

        } else if (line.contains("{")) {

          complexParam.append(line);
          while (!complexParam.contains("}"))
            complexParam.append(in.readLine());
          myParamsList.append(complexParam);
          complexParam.clear();
          break;
        }

      }

    }

  } else {
    return FileOpeningProblem;
  }

  if (!checkRequiredParameters(myParamsList)) {
    return NoRequairedField;
  }

  if (!parseParamsHeader(enviHdr, myParamsList)) {
    return ParsingProblem;
  }
  m_hdrList = myParamsList;
  return OK;

}

bool EnviReader::loadEnviImageData(QString path) {
  QString filePath = path;
  filePath.remove("file:///");
  bool result = false;
  ReadHeaderResult resultHeader;
  clearHeader();
  resultHeader = loadEnviHeader(m_enviHeader, filePath);

  isHeaderOk = false;
  QString message = "ok";
  switch (resultHeader) {
    case NoFileExists: message =       "Файл заголовка не найден";
      break;
    case FileOpeningProblem: message = "Проблема открытия файла заголовка";
      break;
    case NoRequairedField: message =   "Отсутствует Обязательное поле в файле заголовка";
      break;
    case ParsingProblem: message =     "Ошибка разбора заголовочного файла";
      break;
    case OK: isHeaderOk = true; break;
  }
  if (!isHeaderOk) {
    emit headerIsReady(false, message);
    return false;
  }


  int samples =   m_enviHeader.samples;
  int lines =     m_enviHeader.lines;
  int bands =     m_enviHeader.bands;
  DataType type = m_enviHeader.dataType;
  int sizeType =  getSizeOfEnviType(type);
  quint64 sizeInBytes = static_cast<quint64>(samples * lines * bands * sizeType);
  qDebug() << "Size:" << sizeInBytes << sizeType;

  if (type != FloatEnvi && type != UnsignIntegerEnvi) {
    message = "Тип данных указанный в заголовке не поддерживается.";
    emit headerIsReady(false, message);
    return result;
  }
  if (type == FloatEnvi) {
    emit correctStartSignal(1);
    setMultiplicator(1);
  }
  if (type == UnsignIntegerEnvi) {
    emit correctStartSignal(0.006);
    setMultiplicator(0.006);
  }
  emit headerIsReady(true, message);

  QString pathData = filePath.remove(".hdr", Qt::CaseInsensitive);
  QFile* file = new QFile(pathData);
  qDebug() << pathData;
  if (!file->exists()) { //Проверка на случай если файл данных изображения содержит расширение dat
    file->setFileName(pathData.append(".dat"));
    if (!file->exists()) {
      isFileOk = false;
      emit imageReady("Файл данных не найден!", result);
      return false;
    }
  }

  if (!file->open(QIODevice::ReadOnly)) {

    isFileOk = false;
    emit imageReady("Проблема открытия файла данных: " + file->errorString(), result);
    return false;
  }
  quint64 fsize = static_cast<quint64>(file->size());
  if (fsize != sizeInBytes) {
    isFileOk = false;
    qInfo() << "File size:" << fsize;
    emit imageReady("Размер файла данных не соответствует размеру, указанному в заголовке", result);
    return false;
  }
  emit startProgressState(lines * samples);
  m_Image = QImage(samples, lines, QImage::Format_RGB888);

  *m_byteArray = file->readAll();
  if (type == FloatEnvi)
    m_enviImageFloat32 = (float*)m_byteArray->data();
  if (type == UnsignIntegerEnvi)
    m_enviImageUint16  = (uint16_t*)m_byteArray->data();
  makeImage(samples, lines, bands);
  pointIgnoreList.clear();
  //m_Image = m_Image.scaled(samples,1000,Qt::IgnoreAspectRatio);

  file->close();
  isFileOk = true;
  result = true;
  emit imageReady(filePath, result);
  return result;
}

bool EnviReader::makeImage(int samples, int lines, int bands) {
  bool result = false;
  loadedPixels = 0;
  QVector <int> channelsSet = {0, 1, 2};
  QVector<uint16_t> channelsUint16;
  channelsUint16.resize(bands);

  if (m_enviHeader.interLeave.contains("bip", Qt::CaseInsensitive)) {

    if (m_enviHeader.dataType == FloatEnvi)  {
      UniversalImageReader::makeImageForBip(m_Image,
                                            channelsSet,
                                            loadedPixels,
                                            m_enviImageFloat32,
                                            samples,
                                            lines,
                                            bands,
                                            multic);
    }

    if (m_enviHeader.dataType == UnsignIntegerEnvi) {

      UniversalImageReader::makeImageForBip(m_Image,
                                            channelsSet,
                                            loadedPixels,
                                            m_enviImageUint16,
                                            samples,
                                            lines,
                                            bands,
                                            multic);
    }
  }

  if (m_enviHeader.interLeave.contains("bil", Qt::CaseInsensitive)) {

    if (m_enviHeader.dataType == FloatEnvi)  {
      UniversalImageReader::makeImageForBil(m_Image,
                                            channelsSet,
                                            loadedPixels,
                                            m_enviImageFloat32,
                                            samples,
                                            lines,
                                            bands,
                                            multic);
    }

    if (m_enviHeader.dataType == UnsignIntegerEnvi) {

      UniversalImageReader::makeImageForBil(m_Image,
                                            channelsSet,
                                            loadedPixels,
                                            m_enviImageUint16,
                                            samples,
                                            lines,
                                            bands,
                                            multic);
    }

  }

  if (m_enviHeader.interLeave.contains("bsq", Qt::CaseInsensitive)) {

    if (m_enviHeader.dataType == FloatEnvi)  {
      result = UniversalImageReader::makeImageForBsq(m_Image,
                                                     channelsSet,
                                                     loadedPixels,
                                                     m_enviImageFloat32,
                                                     samples,
                                                     lines,
                                                     multic);
    }

    if (m_enviHeader.dataType == UnsignIntegerEnvi) {
      result = UniversalImageReader::makeImageForBsq(m_Image,
                                                     channelsSet,
                                                     loadedPixels,
                                                     m_enviImageUint16,
                                                     samples,
                                                     lines,
                                                     multic);


    }

  }

  return result;
}

void EnviReader::changeChannelsSet(QVector<QString> channelsInfo) {

  bool result = false;
  if (channelsInfo.count() != m_enviHeader.bands) {
    qCritical() << "Bands and channels problem!";
    qCritical() << "Bands = " << m_enviHeader.bands;
    qCritical() << "channelsInfo.count = " << channelsInfo.count();
  }

  QVector<int>indexedChannels(3);
  indexedChannels.fill(-1);

  for (int i = 0; i < channelsInfo.count(); ++i) {

    if (channelsInfo.at(i) == "red")  {

      indexedChannels[2] = i;
      qInfo() << "Indexing" << indexedChannels[2] << "red";
      continue;
    } else if (channelsInfo.at(i) == "green") {

      indexedChannels[1] = i;
      qInfo() << "Indexing" << indexedChannels[1] << "green";
      continue;
    } else if (channelsInfo.at(i) == "blue") {

      indexedChannels[0] = i;
      qInfo() << "Indexing" << indexedChannels[0] << "blue";
      continue;
    }

  }


  result = makeImageWithChannelsSet(indexedChannels);
  emit channelsChanged(result);
}

void EnviReader::changeZoomForImage(float scaleFactor) {
  int width = m_Image.width() * scaleFactor;
  int height = m_Image.height() * scaleFactor;
  m_zoomedImage = m_Image.scaled(width, height);
  scale = scaleFactor;
  qDebug() << "scaleFactor in enviReader:" << scale;
  emit zoomWasChanged();
}

void EnviReader::addPointToIgnoreListAndSearchAgain() {
  pointIgnoreList.append(lastDarkPoint);
  findDarkestPixel(lastlistRects);
  qInfo() << "Проверка пополнения Ignore Lista: " << pointIgnoreList.count();
}

bool EnviReader::makeImageWithChannelsSet(QVector<int> channelsSet) {
  bool result = false;
  if (isHeaderOk == false || isFileOk == false)
    return result;

  int samples = m_enviHeader.samples;
  int lines = m_enviHeader.lines;
  int bands = m_enviHeader.bands;
  emit startProgressState(lines * samples);
  QString interLeave = m_enviHeader.interLeave;
  m_Image = QImage(samples, lines, QImage::Format_RGB888);

  loadedPixels = 0;

  if (interLeave == "bip") {
    if (m_enviHeader.dataType == FloatEnvi)  {
      UniversalImageReader::makeImageForBip(
          m_Image,
          channelsSet,
          loadedPixels,
          m_enviImageFloat32,
          samples,
          lines,
          bands,
          multic);
    }
    if (m_enviHeader.dataType ==  UnsignIntegerEnvi) {

      UniversalImageReader::makeImageForBip(
          m_Image,
          channelsSet,
          loadedPixels,
          m_enviImageUint16,
          samples,
          lines,
          bands,
          multic);
    }
  }

  if (interLeave == "bil") {

    if (m_enviHeader.dataType == FloatEnvi)  {
      UniversalImageReader::makeImageForBil(
          m_Image,
          channelsSet,
          loadedPixels,
          m_enviImageFloat32,
          samples,
          lines,
          bands,
          multic);
    }

    if (m_enviHeader.dataType ==  UnsignIntegerEnvi) {

      UniversalImageReader::makeImageForBil(
          m_Image,
          channelsSet,
          loadedPixels,
          m_enviImageUint16,
          samples,
          lines,
          bands,
          multic);
    }

  }

  if (interLeave == "bsq") {
    if (m_enviHeader.dataType == FloatEnvi)  {
      UniversalImageReader::makeImageForBsq(
          m_Image,
          channelsSet,
          loadedPixels,
          m_enviImageFloat32,
          samples,
          lines,
          multic);

    }

    if (m_enviHeader.dataType ==  UnsignIntegerEnvi) {

      UniversalImageReader::makeImageForBsq(
          m_Image,
          channelsSet,
          loadedPixels,
          m_enviImageUint16,
          samples,
          lines,
          multic);

    }

  }

  if (scale != 1.0) {
    int width = m_Image.width() * scale;
    int height = m_Image.height() * scale;
    qDebug() << "Scale factor channels set:" << this->scale;
    m_zoomedImage = m_Image.scaled(width, height);
  } else {

    m_zoomedImage = m_Image;
  }


  result = true;//Заглушка может быть имеет смысл добавить дополнительные проверки
  return result;
}

void EnviReader::findDarkestPixel(QVector<QRect>* listRect) {
  qInfo() << "Вход в функцию для нахождения координат тёмного пикселя....\n";
  lastlistRects = listRect;
  if (!isFileOk && isHeaderOk) {
    qWarning() << "Перед поиском темного пикселя нужно загрузить файл данных\n";
    return;
  }

  for (int i = 0; i < listRect->count(); ++i) {
    if (m_Image.width() <= listRect->at(i).bottomRight().x() ||
        m_Image.height() <= listRect->at(i).bottomRight().y()
       ) {

      qWarning() << "Размер выделенной области больше оригинала изображения!!! Удаление области.";
      listRect->removeAt(i);
      continue;
    }
  }

  if (listRect->empty()) {
    qInfo() << "Нет выделенных областей для поиска\n";
    return;
  }

  //int samples =   m_enviHeader.samples;
  //int lines =     m_enviHeader.lines;
  //int bands =     m_enviHeader.bands;
  int areaWidth  = 0;
  int areaHeight = 0;
  int offSetX =    0;
  int offSetY =    0;
  int darkestOffSetX = 0;
  int darkestOffSetY = 0;

  QVector<float> darkValues;
  QVector<float> theDarkestValues;
  QVector<uint16_t>darkUint16;
  QVector<uint16_t>theDarkUint16;
  QPoint darkPoint;
  QPoint darkOfTheDarkPoint;
  int minOfTheMinIndex = 0;
  QImage theDarkestImage;
  double minOfTheMin = std::numeric_limits<double>::max();
  double min = 0;

  darkArguments <float> daF;
  darkArguments <uint16_t> daUint16;

  for (int j = 0; j < listRect->count(); ++j) {

    double min = std::numeric_limits<double>::max();
    areaWidth  = listRect->at(j).width();
    areaHeight = listRect->at(j).height();
    offSetX =    listRect->at(j).topLeft().x();
    offSetY =    listRect->at(j).topLeft().y();


    m_darkImage = QImage(areaWidth, areaHeight, QImage::Format_RGB888);
    if (m_enviHeader.dataType == FloatEnvi) {
      daF.darkValues.clear();

      daF.m_darkImage = m_darkImage;
      daF.offSetX = offSetX;
      daF.offSetY = offSetY;
      daF.samples = m_enviHeader.samples;
      daF.lines = m_enviHeader.lines;
      daF.bands = m_enviHeader.bands;
      daF.areaHeight = areaHeight;
      daF.areaWidth = areaWidth;
      daF.pointer = m_enviImageFloat32;
      daF.darkValues = darkValues;
      daF.min = min;
      daF.darkPoint = darkPoint;
      daF.multic = multic;
      daF.ignorePixels = pointIgnoreList;
      daF.ignorePolygons = ignorePolygons;
    }
    if (m_enviHeader.dataType == UnsignIntegerEnvi) {
      daUint16.darkValues.clear();

      daUint16.m_darkImage = m_darkImage;
      daUint16.offSetX = offSetX;
      daUint16.offSetY = offSetY;
      daUint16.samples = m_enviHeader.samples;
      daUint16.lines = m_enviHeader.lines;
      daUint16.bands = m_enviHeader.bands;
      daUint16.areaHeight = areaHeight;
      daUint16.areaWidth = areaWidth;
      daUint16.pointer = m_enviImageUint16;
      daUint16.darkValues = darkUint16;
      daUint16.min = min;
      daUint16.darkPoint = darkPoint;
      daUint16.multic = multic;
      daUint16.ignorePixels = pointIgnoreList;
      daUint16.ignorePolygons = ignorePolygons;

    }

    if (m_enviHeader.interLeave.contains("bip", Qt::CaseInsensitive)) {

      if (m_enviHeader.dataType == FloatEnvi) {
        UniversalDarkPixelFinder::findDarkPixelInBip(daF);
      }
      if (m_enviHeader.dataType == UnsignIntegerEnvi) {
        UniversalDarkPixelFinder::findDarkPixelInBip(daUint16);
      }

    }

    if (m_enviHeader.interLeave.contains("bil", Qt::CaseInsensitive)) {


      if (m_enviHeader.dataType == FloatEnvi) {
        UniversalDarkPixelFinder::findDarkPixelInBil(daF);
      }
      if (m_enviHeader.dataType == UnsignIntegerEnvi) {

        UniversalDarkPixelFinder::findDarkPixelInBil(daUint16);

      }

    }

    if (m_enviHeader.interLeave.contains("bsq", Qt::CaseInsensitive)) {

      if (m_enviHeader.dataType == FloatEnvi) {
        UniversalDarkPixelFinder::findDarkPixelInBsq(daF);
      }
      if (m_enviHeader.dataType == UnsignIntegerEnvi) {

        UniversalDarkPixelFinder::findDarkPixelInBsq(daUint16);

      }

    }

    //        qDebug()<<"min"<<min;
    //        qDebug()<<"MinOfTheMin"<<minOfTheMin;
    if (m_enviHeader.dataType == FloatEnvi)
      min = daF.min;
    if (m_enviHeader.dataType == UnsignIntegerEnvi)
      min = daUint16.min;

    if (minOfTheMin > min) {
      minOfTheMin = min;
      minOfTheMinIndex = j;

      if (m_enviHeader.dataType == FloatEnvi) {
        darkOfTheDarkPoint = daF.darkPoint;
        theDarkestImage = daF.m_darkImage;
        darkestOffSetX = daF.offSetX;
        darkestOffSetY = daF.offSetY;
        theDarkestValues = daF.darkValues;
      }
      if (m_enviHeader.dataType == UnsignIntegerEnvi) {
        darkOfTheDarkPoint = daUint16.darkPoint;
        theDarkestImage = daUint16.m_darkImage;
        darkestOffSetX = daUint16.offSetX;
        darkestOffSetY = daUint16.offSetY;
        theDarkUint16 = daUint16.darkValues;
      }
      qDebug() << "Dark: " << j;

    }

  }
  qInfo() << "Наитемнейший пиксель был найден в области" << minOfTheMinIndex;
  min = minOfTheMin;
  m_darkImage = theDarkestImage;
  darkPoint = darkOfTheDarkPoint;
  offSetX = darkestOffSetX;
  offSetY = darkestOffSetY;
  if (m_enviHeader.dataType == FloatEnvi)
    darkValues = theDarkestValues;
  if (m_enviHeader.dataType == UnsignIntegerEnvi)
    darkUint16 = theDarkUint16;
  theDarkestImage = QImage();

  QPoint croupedDarkPoint;
  croupImage(m_darkImage, darkPoint, croupedDarkPoint);
  QString pathDarkArea;
  drawAimDarkPointer(m_darkImage, croupedDarkPoint, "DarkAreas", pathDarkArea);

  DarkPoint dp;
  dp.originPoint.setX(darkPoint.x() + offSetX);
  dp.originPoint.setY(darkPoint.y() + offSetY);
  lastDarkPoint = darkPoint;

  if (m_enviHeader.dataType == FloatEnvi) {
    for (int i = 0; i < darkValues.count(); ++i) {
      dp.chanelsValues.append(QString::number(darkValues.at(i)));
    }
  }

  if (m_enviHeader.dataType == UnsignIntegerEnvi) {
    for (int i = 0; i < darkUint16.count(); ++i) {
      dp.chanelsValues.append(QString::number(darkUint16.at(i)));
    }
  }
  dp.info = darkReport(min, darkPoint, offSetX, offSetY, dp.chanelsValues);
  pathDarkArea.prepend("file:///");

  emit darkPixelTaskFinished(true, dp, pathDarkArea);

}



//Вспомогательные функции

QString EnviReader::darkReport(double minValue, QPoint darkPoint, int offX, int offY, QVector<QString> values) {
  QString resultStr;
  resultStr.append("Cумма всех каналов наитемнейшего пикселя:")
           .append(QString::number(minValue))
           .append("\n")
           .append("Координаты тёмного пикселя в полигоне:   ")
           .append("X:")
           .append(QString::number(darkPoint.x()))
           .append(" Y:")
           .append(QString::number(darkPoint.y()))
           .append("\n")
           .append("Координаты тёмного пикселя в оригинале: ")
           .append("X:")
           .append(QString::number(darkPoint.x() + offX))
           .append(" Y:")
           .append(QString::number(darkPoint.y() + offY))
           .append("\n")
           .append("Значения каналов для тёмного пикселя:\n");
  for (int i = 0; i < values.count(); ++i) {
    resultStr.append(values.at(i)).append(" ; ");
  }

  return resultStr;
}

void EnviReader::clearIgnoreList() {
  pointIgnoreList.clear();
}

void EnviReader::drawAimDarkPointer(QImage img, QPoint darkPoint, QString folderName, QString& file) {

  int scaleFactor = 2;
  QDir dir;
  QString checkDir = QDir::currentPath() + folderName.prepend("/");
  dir.setPath(checkDir);
  if (!dir.exists())
    dir.mkdir(checkDir);
  QString fileName = QDateTime::currentDateTime().toString("yyyyMMdd__hh_mm_ss");
  fileName.append(".bmp");
  checkDir.append("/");
  checkDir.append(fileName);
  QImage scaledImage = img.scaled(img.width() * scaleFactor, img.height() * scaleFactor);
  QPoint scaledPoint;
  scaledPoint.setX(darkPoint.x()*scaleFactor);
  scaledPoint.setY(darkPoint.y()*scaleFactor);
  QPixmap pixmap = QPixmap::fromImage(scaledImage);
  QPainter painter(&pixmap);
  QPen pen;
  pen.setColor(Qt::yellow);
  painter.setPen(pen);
  painter.drawEllipse(scaledPoint, 2 * scaleFactor, 2 * scaleFactor);
  painter.end();
  pixmap.save(checkDir, "BMP");
  file = checkDir;

}

void EnviReader::saveAnyImage(QImage img, QString folderName) {
  QDir dir;
  QString checkDir = QDir::currentPath() + folderName.prepend("/");
  dir.setPath(checkDir);
  if (!dir.exists())
    dir.mkdir(checkDir);
  QString fileName = QDateTime::currentDateTime().toString("yyyyMMdd__hh_mm_ss");
  fileName.append(".bmp");
  checkDir.append("/");
  checkDir.append(fileName);
  img.save(checkDir, "BMP");
  emit imageWasSavedAsBmp();
}

int EnviReader::getSizeOfEnviType(DataType dt) {
  switch (dt) {

    case ByteEnvi: return 1;
    case IntegerEnvi: return 2;
    case LongEnvi: return 4;
    case FloatEnvi: return 4;
    case DoubleEnvi: return 8;
    case PairFloatEnvi: return 8;
    case PairDoubleEnvi: return 16;
    case UnsignIntegerEnvi: return 2;
    case UnsignLongIntegerEnvi: return 4;
    case LongInteger64Envi: return 8;
    case UnsignLongInteger64Envi: return 8;
    default: return 0;
  }
}

void EnviReader::openImagesFolder() {
  QString openExplorer = "c:/windows/explorer.exe /n,";
  QString dir = QDir::currentPath();
  dir.append("/DarkAreas");
  QString path = QDir::toNativeSeparators(dir);
  openExplorer.append(path);
  QProcess::startDetached(openExplorer);
}

void EnviReader::savePicture() {
  saveAnyImage(m_Image, "BMPs");
}

void EnviReader::clearHeader() {
  m_enviHeader.bandNames.clear();
  m_enviHeader.bands = 0;
  m_enviHeader.byteOrder = LSF;
  m_enviHeader.coordSystem.clear();
  m_enviHeader.dataGainValues.clear();
  m_enviHeader.dataType = FloatEnvi;
  m_enviHeader.description.clear();
  m_enviHeader.fileType.clear();
  m_enviHeader.fwHm.clear();
  m_enviHeader.headerOffset = 0;
  m_enviHeader.interLeave.clear();
  m_enviHeader.lines = 0;
  m_enviHeader.mapInfo.clear();
  m_enviHeader.samples = 0;
  m_enviHeader.sensorType.clear();
  m_enviHeader.waveLength.clear();
  m_enviHeader.waveLengthUnit.clear();
}

void EnviReader::clearImage() {
  m_Image = QImage();
}

int EnviReader::getLoadedPixels() const {
  return loadedPixels;
}

QImage EnviReader::getImage() const {
  return m_Image;
}

void EnviReader::setMultiplicator(float multic) {
  this->multic = multic;
  qInfo() << "Устанавливаем мультик..." << multic;
}

EnviHeaderStruct EnviReader::getEnviHeader()const {
  return m_enviHeader;
}

QStringList EnviReader::getHdrList() const {
  return m_hdrList;
}

QImage EnviReader::getDarkImage() const {
  return m_darkImage;
}

QImage EnviReader::getZoomedImage() const {
  return m_zoomedImage;
}

int EnviReader::getScaleFactor() const {
  return scale;
}

void EnviReader::setIgnorePolygons(const QVector<QPolygon> value) {
  ignorePolygons = value;
}

void EnviReader::clearAreasAndIgnores() {
  if (!ignorePolygons.isEmpty())
    ignorePolygons.clear();
  if (!pointIgnoreList.isEmpty())
    pointIgnoreList.clear();
  if (lastlistRects)
    lastlistRects->clear();
}

bool EnviReader::checkRequiredParameters(QStringList paramsList) {
  bool result = false;
  int counter = 0;

  for (int i = 0; i < requiredParams.count(); ++i) {

    for (int j = 0; j < paramsList.count(); ++j) {

      if (paramsList.at(j).contains(requiredParams.at(i))) {

        if (paramsList.at(j).contains("default"))
          continue;
        ++counter;
      }

    }
  }
  if (requiredParams.count() == counter) {
    result = true;
  } else {
    result = false;
    qWarning() << "No required fields are in the header!!!";
  }

  return result;
}

bool EnviReader::parseParamsHeader(EnviHeaderStruct& enviHdr, QStringList paramsList) {

  bool result = false;
  bool isBandNamesExists = false;
  qInfo() << "\nРазбор заголовка:\n";
  for (int i = 0; i < paramsList.count(); ++i) {

    QString pars = paramsList.at(i);

    //Необходимые параметры (обязательно должны быть)
    if (paramsList.at(i).contains("samples")) {
      singleParamPars(pars, "samples");
      enviHdr.samples = pars.toInt(&result);
      qInfo() << "samples:" << enviHdr.samples << "\n";
      continue;
    }
    if (paramsList.at(i).contains("lines")) {
      singleParamPars(pars, "lines");
      enviHdr.lines = pars.toInt(&result);
      qInfo() << "Lines:" << enviHdr.lines << "\n";
      continue;
    }
    if (paramsList.at(i).contains("bands") && (!paramsList.at(i).contains("default"))) {
      singleParamPars(pars, "bands");
      enviHdr.bands = pars.toInt(&result);
      qInfo() << "Bands:" << enviHdr.bands << "\n";
      continue;
    }
    if (paramsList.at(i).contains("header offset")) {
      singleParamPars(pars, "header offset");
      enviHdr.headerOffset = pars.toInt(&result);
      qInfo() << "Header Offset:" << enviHdr.headerOffset << "\n";
      continue;
    }
    if (paramsList.at(i).contains("file type")) {
      singleParamPars(pars, "file type");
      enviHdr.fileType = pars;
      qInfo() << "File type:" << enviHdr.fileType << "\n";
      continue;
    }
    if (paramsList.at(i).contains("byte order")) {
      singleParamPars(pars, "byte order");
      enviHdr.byteOrder = (ByteOrder)pars.toInt(&result);
      qInfo() << "Byte order:" << enviHdr.byteOrder << "\n";
      continue;
    }
    if (paramsList.at(i).contains("interleave")) {
      singleParamPars(pars, "interleave");
      enviHdr.interLeave = pars;
      qInfo() << "interleave:" << enviHdr.interLeave << "\n";
      continue;
    }

    //Дополнительные параметры (в примере)
    if (paramsList.at(i).contains("description")) {
      enviHdr.description = pars;
      continue;
    }

    if (paramsList.at(i).contains("data type")) {

      singleParamPars(pars, "data type");
      enviHdr.dataType = (DataType)pars.toInt();
      qInfo() << "data type:" << enviHdr.dataType << "\n";
      continue;
    }

    if (paramsList.at(i).contains("sensor type")) {
      continue;
    }
    if (paramsList.at(i).contains("map info")) {
      continue;
    }
    if (paramsList.at(i).contains("coordinate system string")) {
      continue;
    }

    if (paramsList.at(i).contains("wavelength units")) {
      singleParamPars(pars, "wavelength units");
      enviHdr.waveLengthUnit = pars;
      qInfo() << "wavelength units" << enviHdr.waveLengthUnit << "\n";
      continue;
    }

    if (paramsList.at(i).contains("band names")) {
      complexParam(pars, "band names", enviHdr.bandNames);
      qInfo() << "band names:\n";
      for (int i = 0; i < enviHdr.bandNames.count(); ++i) {
        qInfo() << enviHdr.bandNames.at(i) << "\n";
      }
      isBandNamesExists = true;
      continue;
    }

    if (paramsList.at(i).contains("wavelength") && (!paramsList.at(i).contains("units"))) {
      complexParam(pars, "wavelength", enviHdr.waveLength);
      qInfo() << "wavelength:\n";
      for (int i = 0; i < enviHdr.waveLength.count(); ++i) {
        qInfo() << enviHdr.waveLength.at(i) << "\n";
      }
      continue;
    }

    if (paramsList.at(i).contains("fwhm")) {
      complexParam(pars, "fwhm", enviHdr.fwHm);
      qInfo() << "fwhm:\n";
      for (int i = 0; i < enviHdr.fwHm.count(); ++i) {
        qInfo() << enviHdr.fwHm.at(i) << "\n";
      }
      continue;
    }

    if (paramsList.at(i).contains("data gain values")) {
      complexParam(pars, "data gain values\n", enviHdr.dataGainValues);
      qInfo() << "data gain values:\n";
      for (int i = 0; i < enviHdr.dataGainValues.at(i); ++i) {
        qInfo() << enviHdr.dataGainValues.at(i) << "\n";
      }
      continue;
    }

    if (paramsList.at(i).contains("sun elevation")) {
      singleParamPars(pars, "sun elevation");
      enviHdr.sunElevationAngle = pars.toDouble();
      continue;
    }


    if (paramsList.at(i).contains("acquisition time")) {
      continue;
    }
    if (paramsList.at(i).contains("bbl")) {
      continue;
    }
    if (paramsList.at(i).contains("class lookup")) {
      continue;
    }
    if (paramsList.at(i).contains("class names")) {
      continue;
    }
    if (paramsList.at(i).contains("classes")) {
      continue;
    }
    if (paramsList.at(i).contains("cloud cover")) {
      continue;
    }
    if (paramsList.at(i).contains("complex function")) {
      continue;
    }
    if (paramsList.at(i).contains("data ignore value")) {
      continue;
    }
    if (paramsList.at(i).contains("data offset values")) {
      continue;
    }
    if (paramsList.at(i).contains("data reflectance gain values")) {
      continue;
    }
    if (paramsList.at(i).contains("data reflectance offset values")) {
      continue;
    }
    if (paramsList.at(i).contains("default stretch")) {
      continue;
    }
    if (paramsList.at(i).contains("dem band")) {
      continue;
    }
    if (paramsList.at(i).contains("dem file")) {
      continue;
    }
    if (paramsList.at(i).contains("geo points")) {
      continue;
    }
    if (paramsList.at(i).contains("pixel size")) {
      continue;
    }
    if (paramsList.at(i).contains("projection info")) {
      continue;
    }
    if (paramsList.at(i).contains("read procedures")) {
      continue;
    }
    if (paramsList.at(i).contains("reflectance scale factor")) {
      continue;
    }
    if (paramsList.at(i).contains("rpc info")) {
      continue;
    }
    if (paramsList.at(i).contains("security tag")) {
      continue;
    }
    if (paramsList.at(i).contains("solar irradiance")) {
      continue;
    }
    if (paramsList.at(i).contains("spectra names")) {
      continue;
    }
    if (paramsList.at(i).contains("sun azimuth")) {
      continue;
    }
    if (paramsList.at(i).contains("sun elevation")) {
      continue;
    }
    if (paramsList.at(i).contains("x start")) {
      continue;
    }
    if (paramsList.at(i).contains("y start")) {
      continue;
    }
    if (paramsList.at(i).contains("z plot average")) {
      continue;
    }
    if (paramsList.at(i).contains("z plot range")) {
      continue;
    }
    if (paramsList.at(i).contains("z plot titles")) {
      continue;
    }

  }

  if (!isBandNamesExists) { //Не все hdr имеют имена каналов (заполняем порядковый номер)
    QStringList unknownBandsNamesList;

    for (int i = 0; i < m_enviHeader.bands; ++i) {
      QString name = "Неизвестный канал ";
      name.append(QString::number(i));
      unknownBandsNamesList.append(name);

    }
    m_enviHeader.bandNames = unknownBandsNamesList;
  }
  qInfo() << "\n___________________________________________\nРезультат разбора hdr:" << result;
  qInfo() << "\n___________________________________________\n\n";
  return result;

}

void EnviReader::singleParamPars(QString& param, QString remove) {

  param.remove(remove);
  param.replace(" ", "");
  param.remove("=");
}

void EnviReader::complexParam(QString& param, QString remove, QStringList& list) {
  singleParamPars(param, remove);
  param.remove("{");
  param.remove("\n");
  param.remove("}");
  param.replace(" ", "");
  list = param.split(",");
}

void EnviReader::complexParam(QString& param, QString remove, QVector<double>& list) {
  singleParamPars(param, remove);
  param.remove("{");
  param.remove("\n");
  param.remove("}");

  QStringList params = param.split(",");

  for (int i = 0; i < params.count(); ++i) {

    list.append(params.at(i).toDouble());
  }

}


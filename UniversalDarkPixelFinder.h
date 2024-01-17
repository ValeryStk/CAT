#ifndef UNIVERSALDARKPIXELFINDER_H
#define UNIVERSALDARKPIXELFINDER_H
#include <QDebug>
#include <QImage>
#include <QPoint>
#include "PolygonCollisionChecker.h"

template<class T>
struct darkArguments {

  QImage m_darkImage;
  int offSetY;
  int offSetX;
  int samples;
  int lines;
  int bands;
  int areaHeight;
  int areaWidth;
  T* pointer;
  QVector<T>darkValues;
  double min;
  QPoint darkPoint;
  float multic;
  QVector<QPoint>ignorePixels;
  QVector<QPolygon>ignorePolygons;
};

class UniversalDarkPixelFinder {


 public:
  template<class T>
  static bool findDarkPixelInBip(darkArguments<T>& da) {

    QRgb value;
    QVector<T> channels;
    channels.resize(da.bands);
    int startPoint = da.offSetY * da.samples * da.bands + da.offSetX * da.bands;
    int endPoint = startPoint + da.samples * da.areaHeight * da.bands;
    int count = 0;
    int line =  0;

    for (int i = startPoint; i < endPoint; i += da.bands * da.samples) {

      for (int k = 0; k < da.areaWidth * da.bands; k += da.bands) {

        float sum = 0;
        for (int j = 0; j < channels.count(); ++j) {

          channels[j] = da.pointer[i + j + k];
          sum += channels[j];
        }

        int chR  = 0;
        int chG  = 0;
        int chB  = 0;

        if (channels.count() > 2)
          chR = channels[2] * da.multic;
        if (channels.count() > 1)
          chG = channels[1] * da.multic;
        if (channels.count() > 0)
          chB = channels[0] * da.multic;

        value = qRgb(chR, chG, chB);

        bool isIgnore = false;
        for (int l = 0; l < da.ignorePixels.count(); ++l) {

          if (da.ignorePixels.at(l).x() == count && da.ignorePixels.at(l).y() == line) {
            isIgnore = true;
            qInfo() << "****************** bip ignoring *************************************";
          }
        }
        if (!isIgnore) {
          if (da.min > sum && sum != 0 && sum > 0) { //Проверка суммы всех каналов на 0

            QPoint point = QPoint(count + da.offSetX, line + da.offSetY);
            int index = -1;
            bool result = false;
            std::tie(result, index) = PolygonCollisionChecker::isPointInPolygons(point, da.ignorePolygons);
            if (result) {
              qDebug() << "Bip point is in ignore polygon..." << da.ignorePolygons.count();
            } else {

              da.min = sum;
              da.darkPoint.setX(count);
              da.darkPoint.setY(line);
              da.darkValues = channels;
            }
          }
        }

        da.m_darkImage.setPixel(count, line, value);
        ++count;
        if (count == da.areaWidth) {
          ++line;
          count = 0;
        }
      }

    }

    return true;
  }


  template<class T>
  static bool findDarkPixelInBil(darkArguments<T>& da) {

    QRgb value;
    QVector<T> channels;
    channels.resize(da.bands);
    int xCounter = 0;
    int yCounter = 0;
    int startPoint = da.offSetY * da.samples * da.bands + da.offSetX;
    int endPoint   = startPoint + da.samples * da.areaHeight * da.bands;

    for (int j = startPoint; j < endPoint; j = j + da.bands * da.samples) {


      for (int i = 0; i < da.areaWidth; ++i) {

        float sum = 0;
        for (int k = 0; k < channels.count(); ++k) {

          channels[k] = da.pointer[i + j + da.samples * k];
          sum += channels[k];

        }

        uint chR = 0;
        uint chG = 0;
        uint chB = 0;

        if (channels.count() > 2)
          chR = channels[2] * da.multic;
        if (channels.count() > 1)
          chG = channels[1] * da.multic;
        if (channels.count() > 0)
          chB = channels[0] * da.multic;

        value = qRgb(chR, chG, chB);

        bool isIgnore = false;
        for (int l = 0; l < da.ignorePixels.count(); ++l) {

          if (da.ignorePixels.at(l).x() == xCounter && da.ignorePixels.at(l).y() == yCounter) {
            isIgnore = true;
            qInfo() << "******************* bil ignoring ************************************";
          }
        }

        if (!isIgnore) {
          if (da.min > sum && sum != 0 && sum > 0) { //Проверка суммы всех каналов на чистый 0 (Может работать некорректно)


            QPoint point = QPoint(xCounter + da.offSetX, yCounter + da.offSetY);
            int index = -1;
            bool result = false;
            std::tie(result, index) = PolygonCollisionChecker::isPointInPolygons(point, da.ignorePolygons);
            if (result) {
              qDebug() << "Bil point is in ignore polygon..." << da.ignorePolygons.count();
            } else {

              da.min = sum;
              da.darkPoint.setX(xCounter);
              da.darkPoint.setY(yCounter);
              da.darkValues = channels;

            }
          }
        }

        da.m_darkImage.setPixel(xCounter, yCounter, value);
        ++xCounter;
      }
      xCounter = 0;
      ++yCounter;
    }
    return true;
  }


  template<class T>
  static bool findDarkPixelInBsq(darkArguments<T>& da) {

    QRgb value;
    QVector<T> channels;
    channels.resize(da.bands);
    int xCounter = 0;
    int yCounter = 0;

    for (int j = da.offSetY; j < da.offSetY + da.areaHeight; ++j) {

      for (int i = da.offSetX; i < da.offSetX + da.areaWidth; ++i) {

        float sum = 0;
        bool isOk = true;
        for (int k = 0; k < channels.count(); ++k) {

          channels[k] = da.pointer[i + j * da.samples + (da.samples * da.lines) * k];
          sum += channels[k];
          if (channels[k] < 0)
            isOk = false;
        }

        unsigned int chR  = 0;
        unsigned int chG  = 0;
        unsigned int chB  = 0;

        if (channels.count() > 2)
          chR = channels[2] * da.multic;
        if (channels.count() > 1)
          chG = channels[1] * da.multic;
        if (channels.count() > 0)
          chB = channels[0] * da.multic;

        value = qRgb(static_cast<int>(chR), static_cast<int>(chG), static_cast<int>(chB));

        bool isIgnore = false;
        for (int l = 0; l < da.ignorePixels.count(); ++l) {

          if (da.ignorePixels.at(l).x() == xCounter && da.ignorePixels.at(l).y() == yCounter) {
            isIgnore = true;
            qInfo() << "********************* bsq ignoring **********************************";
            break;
          }
        }
        if (!isIgnore) {
          if (da.min > sum && sum != 0 && sum > 0 && isOk) { //Проверка суммы всех каналов на чистый 0 (Может работать некорректно)

            QPoint point = QPoint(xCounter + da.offSetX, yCounter + da.offSetY);
            int index = -1;
            bool result = false;
            std::tie(result, index) = PolygonCollisionChecker::isPointInPolygons(point, da.ignorePolygons);
            if (result) {
              qDebug() << "Bsq point is in ignore polygon..." << da.ignorePolygons.count();
            } else {

              da.min = sum;
              da.darkPoint.setX(xCounter);
              da.darkPoint.setY(yCounter);
              da.darkValues = channels;
            }
          }
        }
        da.m_darkImage.setPixel(xCounter, yCounter, value);
        ++xCounter;
      }
      xCounter = 0;
      ++yCounter;

    }
    return true;
  }
};


static void croupImage(QImage& img, QPoint dp, QPoint& croupedPoint) {

  int widthImage = img.width();
  int height = img.height();
  int darkPointX = dp.x();
  int darkPointY = dp.y();
  qDebug() << "Размеры изображения:" << widthImage << height;
  qDebug() << "Тёмный пиксель: " << "dPx" << dp.x() << "dPy" << dp.y();

  int desSize = 200;
  int deltaTopY = dp.y() - desSize / 2;
  if (deltaTopY < 0)
    deltaTopY = 0;
  else {
    darkPointY = darkPointY - desSize / 2;
  }

  int deltaBottomY = dp.y() + desSize / 2;
  if (deltaBottomY > img.height())
    deltaBottomY = img.height() - 1 - deltaTopY;
  else
    deltaBottomY = desSize;

  int deltaXleft = dp.x() - desSize / 2;
  if (deltaXleft < 0)
    deltaXleft = 0;
  else {
    darkPointX = darkPointX + desSize / 2;
  }

  int deltaXright = dp.x() + desSize / 2;
  if (deltaXright > img.width())
    deltaXright = img.width() - 1 - deltaXleft;
  else
    deltaXright = desSize;

  qDebug()
      << "xL" << deltaXleft
      << "yT" << deltaTopY
      << "xR" << deltaXright
      << "yB" << deltaBottomY;
  croupedPoint.setX(dp.x() - deltaXleft);
  croupedPoint.setY(dp.y() - deltaTopY);
  QImage cropped = img.copy(deltaXleft, deltaTopY, deltaXright, deltaBottomY);
  img = cropped;

}

#endif // UNIVERSALDARKPIXELFINDER_H

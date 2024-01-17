#ifndef UNIVERSALIMAGEREADER_H
#define UNIVERSALIMAGEREADER_H

#include <QImage>
class UniversalImageReader {
 public:
  template<class T>
  static bool makeImageForBip(QImage& image,
                              QVector<int>channelsSet,
                              int& loadedPixels,
                              T* pointer,
                              int samples,
                              int lines,
                              int bands,
                              float multic) {

    if (channelsSet.count() < 3)
      return false;
    int xCounter = 0;
    int yCounter = 0;
    QRgb value;
    uint chR  = 0;
    uint chG  = 0;
    uint chB  = 0;

    for (int i = 0; i < samples * lines * bands; i += bands) {

      for (int j = 0; j < channelsSet.count(); ++j) {

        if (channelsSet[2] != -1)
          chR = pointer[i + channelsSet[2]] * multic;

        if (channelsSet[1] != -1)
          chG = pointer[i + channelsSet[1]] * multic;

        if (channelsSet[0] != -1)
          chB = pointer[i + channelsSet[0]] * multic;

      }

      value = qRgb(chR, chG, chB);
      image.setPixel(xCounter, yCounter, value);
      ++xCounter;
      if (xCounter == samples) {
        ++yCounter; loadedPixels = yCounter * samples;
        xCounter = 0;
      }

    }
    return true;
  }


  template<class T>
  static bool makeImageForBil(QImage& image,
                              QVector<int>channelsSet,
                              int& loadedPixels,
                              T* pointer,
                              int samples,
                              int lines,
                              int bands,
                              float multic) {

    if (channelsSet.count() < 3)
      return false;
    QRgb value;
    uint chR  = 0;
    uint chG  = 0;
    uint chB  = 0;
    int xCounter = 0;
    int yCounter = 0;

    for (int j = 0; j < samples * lines * bands; j = j + bands * samples) {

      for (int i = 0; i < samples; ++i) {

        for (int k = 0; k < channelsSet.count(); ++k) {

          if (channelsSet[2] != -1)
            chR = pointer[i + j + (samples) * channelsSet[2]] * multic;
          if (channelsSet[1] != -1)
            chG = pointer[i + j + (samples) * channelsSet[1]] * multic;
          if (channelsSet[0] != -1)
            chB = pointer[i + j + (samples) * channelsSet[0]] * multic;

        }

        value = qRgb(chR, chG, chB);
        image.setPixel(xCounter, yCounter, value);
        ++xCounter;
      }

      xCounter = 0;
      ++yCounter;
      loadedPixels = yCounter * samples;

    }

    return true;
  }



  template<class T>
  static bool makeImageForBsq(QImage& m_Image,
                              QVector<int>channelsSet,
                              int& loadedPixels,
                              T* pointer,
                              int samples,
                              int lines,
                              float multic) {

    if (channelsSet.count() < 3)
      return false;
    QRgb value;
    uint chR  = 0;
    uint chG  = 0;
    uint chB  = 0;

    int xCounter = 0;
    int yCounter = 0;


    for (int j = 0; j < lines; ++j) {

      for (int i = 0; i < samples; ++i) {


        if (channelsSet[2] != -1)
          chR = pointer[i + j * samples + (samples * lines) * channelsSet[2]] * multic;

        if (channelsSet[1] != -1)
          chG = pointer[i + j * samples + (samples * lines) * channelsSet[1]] * multic;

        if (channelsSet[0] != -1)
          chB = pointer[i + j * samples + (samples * lines) * channelsSet[0]] * multic;


        value = qRgb(chR, chG, chB);
        m_Image.setPixel(xCounter, yCounter, value);
        ++xCounter;

      }
      xCounter = 0;
      ++yCounter;
      loadedPixels = yCounter * samples;

    }


    return true;
  }

};

#endif // UNIVERSALIMAGEREADER_H



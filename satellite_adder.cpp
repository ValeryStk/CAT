#include "satellite_adder.h"
#include <QString>
#include <QDir>
#include <QFile>
#include "DBJson.h"
#include "vector"
#include <QJsonArray>
#include <QJsonObject>

namespace cat {

using std::vector;

void loadList(QString path, std::vector<double>& list) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    return;
  }
  QTextStream qts(&file);
  QString line;
  while (qts.readLineInto(&line)) {
    double var = line.toDouble();
    list.push_back(var);
  }
  file.close();
  qDebug() << "List " << path << " " << list.size();
}

bool add_new_satellite(const QString& pathToFolder) {
  bool result = false;
  QDir dir(pathToFolder);
  if (!dir.exists()) {
    return result;
  }
  QString satellite_name;
  satellite_name = dir.dirName();

  QStringList files;
  vector<vector<double>> channels;
  files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
  for (int i = 0; i < files.size(); ++i) {
    vector<double> band_data;
    loadList(satellite_name + "\\" + files[i], band_data);
    channels.push_back(band_data);
  }
  QJsonArray root_obj;
  db_json::getJsonArrayFromFile("sdb.json", root_obj);
  for (size_t k = 0; k < channels[0].size(); ++k) {
    QJsonArray jarr;
    for (size_t j = 0; j < channels.size(); ++j) {
      jarr.append(channels[j][k]);
    }
    auto obj = root_obj[k].toObject();
    if (satellite_name.length() > 0) {
      if (satellite_name[0] != '_')
        satellite_name.prepend("_");
    }
    obj[satellite_name] = jarr;
    root_obj[k] = obj;
  }
  db_json::saveJsonArrayToFile("sdb.json", root_obj, QJsonDocument::Compact);
  return result;
}
}

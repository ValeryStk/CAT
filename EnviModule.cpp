#include "EnviModule.h"
#include "QFile"
#include "QByteArray"
#include <QDataStream>
#include <QFileDialog>
#include <QVector>
#include "LeastSquareSolver.cpp"
using namespace std;

EnviModule::EnviModule()

{

    m_settings = new QSettings(QDir::currentPath()+"/atmoc.ini",QSettings::IniFormat);
    homePath = m_settings->value("Dirs/defaultEnviFolder").toString();
    m_areas = new QVector <QRect>();
    m_polygon = new QVector<QLine>();
    initializeVariables();
    QDir dir;
    dir.setPath(homePath);
    if(!dir.exists()){
        homePath = "C:/";
        m_settings->setValue("Dirs/defaultEnviFolder",homePath);
    }
    qInfo()<<"Путь к папке с данными из файла atmoc.ini"<<homePath<<"\n";
    mThread = new QThread();
    m_envi.moveToThread(mThread);
    mThread->start();
    progressTimer.setInterval(50);

    //m_sounder.moveToThread(&beeperThread);
    //beeperThread.start();

    qRegisterMetaType<DarkPoint>();
    qRegisterMetaType<ModeChooser>();
    qRegisterMetaType<SoundIndex>("SoundIndex");

    connect(this,SIGNAL(openEnvi(QString)),&m_envi,SLOT(loadEnviImageData(QString)));
    connect(this,SIGNAL(addAreaToList()),SLOT(addArea()));
    connect(this,SIGNAL(clearAllAreas()),SLOT(clearAreas()));
    connect(this,SIGNAL(findDarkestPixel()),SLOT(startSearchingDarkestPixel()));
    connect(this,SIGNAL(ignoreTheDarkPixelAndFindNew()),&m_envi,SLOT(addPointToIgnoreListAndSearchAgain()));
    connect(this,SIGNAL(playSound(QString)),&m_sounder,SLOT(playSound(QString)));
    connect(this,SIGNAL(changeSoundState(bool)),&m_sounder,SLOT(changeSoundState(bool)));

    connect(&m_envi,SIGNAL(headerIsReady(bool,QString)),this,SLOT(parseHeaderFinished(bool,QString)));
    connect(&m_envi,SIGNAL(imageReady(QString,bool)),this,SLOT(imageWasLoaded(QString,bool)));
    connect(&m_envi,SIGNAL(channelsChanged(bool)),this,SLOT(imageChannelsChanged(bool)));
    connect(&m_envi,SIGNAL(startProgressState(int)),this,SLOT(initProgress(int)));
    connect(&m_envi,SIGNAL(darkPixelTaskFinished(bool,DarkPoint,QString)),this,SLOT(taskForDarkSearchingFinished(bool,DarkPoint,QString)));
    connect(&m_envi,SIGNAL(imageWasSavedAsBmp()),this,SLOT(imageWasSavedAsBMP()));
    connect(this,SIGNAL(zoomScaleFactor(float)),&m_envi,SLOT(changeZoomForImage(float)));
    connect(&m_envi,SIGNAL(zoomWasChanged()),this,SLOT(zoomImageRedy()));
    connect(&m_envi,SIGNAL(correctStartSignal(float)),this,SLOT(acceptStartSignalCorrection(float)));

    connect(&progressTimer,SIGNAL(timeout()),this,SLOT(progressChanged()));
    connect(this,SIGNAL(setMultiplicator(float)),&m_envi,SLOT(setMultiplicator(float)));
    connect(this,SIGNAL(giveMeTheDarkestPixel(QVector<QRect>*)),&m_envi,SLOT(findDarkestPixel(QVector<QRect>*)));
    connect(this,SIGNAL(savePicture()),&m_envi,SLOT(savePicture()));
    connect(this,SIGNAL(openImagesFolder()),&m_envi,SLOT(openImagesFolder()));
    connect(this,SIGNAL(changeChannels(QVector<QString>)),&m_envi,SLOT(changeChannelsSet(QVector<QString>)));
    connect(this,SIGNAL(selectAllArea()),this,SLOT(selectAllImageForDarkSearching()));
    connect(this,SIGNAL(addLine()),this,SLOT(addLineToLomanArea()));
    connect(this,SIGNAL(endPolygon()),this,SLOT(polygonWasCreated()));

    //auto result = lss::optimize({0.1, 2, 0.01, 0.01});
    //qDebug()<<result.albedo;
}

EnviModule::~EnviModule()
{
    mThread->quit();
    beeperThread.quit();
}

void EnviModule::paint(QPainter *painter)
{

    painter->drawImage(0,0,m_image);
    QPen pen;
    pen.setColor(QColor(Qt::yellow));
    pen.setWidth(1);
    painter->setPen(pen);

    if(isDarkMarker){

        painter->drawEllipse(m_dp.originPoint/(1/scaleFactor),3,3);
    }

    if(isSelectMode){

        if(m_mode==SelectRectArea){
            QRect rect(lTpoint,rBpoint);
            painter->drawRect(rect);
        }
        if(m_mode==SelectLomanArea){

           //painter->drawEllipse(pointRect);
        }

    }
    if(!m_areas->isEmpty()){
        pen.setColor(QColor("orange"));
        pen.setWidth(2);
        painter->setPen(pen);
        for(int i=0;i<m_areas->count();++i){

            QRect rect(m_areas->at(i).topLeft()/(1/scaleFactor),m_areas->at(i).bottomRight()/(1/scaleFactor));
            painter->drawRect(rect);
        }
    }
    if(!m_points.isEmpty()){
        pen.setColor(QColor("red"));
        pen.setWidth(2);
        painter->setPen(pen);
        for(int j=0;j<m_points.count();++j){
            painter->drawEllipse(m_points[j]/(1/scaleFactor),2,2);
        }
    }
    if(isIgnorePolygonReady){
        pen.setColor(QColor("red"));
        pen.setWidth(2);
        painter->setPen(pen);
        painter->setBrush(QColor(255,0,0,56));


        QVector<QPolygon>::Iterator it = m_ignorePolygons.begin();
        for(;it<m_ignorePolygons.end();++it){

            QPolygon polygon;
            for(int i=0;i<it->count();++i){

                QPoint point = it->at(i)/(1/scaleFactor);
                polygon.append(point);
            }
            painter->drawPolygon(polygon);
        }

    }

}

QString EnviModule::homeEnviPath() const
{
    QString urlPath = homePath;
    urlPath.prepend("file:///");
    qInfo()<<"UrlPath: "<<urlPath<<"\n";
    return urlPath;
}

void EnviModule::silentClearAreas()
{
    if(!m_areas->isEmpty())m_areas->clear();
    isDarkMarker = false;
}

QPoint EnviModule::dp() const
{
    return m_dp.originPoint;
}

Q_INVOKABLE int EnviModule::getChannelsNumber() const
{
    return channelsNumber;
}

QString EnviModule::darkPointInfo() const
{
    return m_dp.info;
}

void EnviModule::setMode(const EnviModule::ModeChooser mode)
{
    m_mode = mode;
    switch (m_mode) {
    case View:qInfo()<<"View mode ***************************";
        break;
    case SelectLomanArea:qInfo()<<"Loman mode ***************************";
        break;
    case SelectRectArea:qInfo()<<"Rect mode ********************************";
        break;

    }

}

EnviModule::ModeChooser EnviModule::getMode() const
{
    return m_mode;
}

bool EnviModule::getIsLomanStarted() const
{
    return isLomanStarted;
}

QPoint EnviModule::beginPolPoint() const
{
    if(!m_polygon->isEmpty())return m_polygon->at(0).p1();
    return QPoint(0,0);
}


void EnviModule::initializeVariables()
{
    widthImage  = 0;
    heightImage = 0;
    scaleFactor = 1;
    progress    = 0;
    maxProgress = 0;
    multic = 1.0;
    channelsNumber = 0;
    lTpoint = QPoint(0,0);
    rBpoint = QPoint(0,0);
    isSelectMode = false;
    isDarkMarker = false;
    isIgnorePolygonReady = false;
    zoomedImage = QImage(200,200,QImage::Format_RGB32);
    zoomedImage.fill(Qt::red);
    channelsNames = QStringList();
}

QRect EnviModule::findBoundedRect()
{
    QRect rect;
    if (m_polygon->isEmpty())return rect;

    int topLeftX = numeric_limits<int>::max();
    int topLeftY = numeric_limits<int>::max();
    int bottomRightX = 0;
    int bottomRightY = 0;

    for(int i=0;i<m_polygon->count();++i){

        if(m_polygon->at(i).p1().x()<topLeftX)topLeftX = m_polygon->at(i).p1().x();
        if(m_polygon->at(i).p1().y()<topLeftY)topLeftY = m_polygon->at(i).p1().y();
        if(m_polygon->at(i).p1().x()>bottomRightX)bottomRightX = m_polygon->at(i).p1().x();
        if(m_polygon->at(i).p1().y()>bottomRightY)bottomRightY = m_polygon->at(i).p1().y();
    }

    if(topLeftX>lastPolygonPoint.x())topLeftX = lastPolygonPoint.x();
    if(topLeftY>lastPolygonPoint.y())topLeftY = lastPolygonPoint.y();
    if(bottomRightX<lastPolygonPoint.x())bottomRightX = lastPolygonPoint.x();
    if(bottomRightY<lastPolygonPoint.y())bottomRightY = lastPolygonPoint.y();

    rect.setTopLeft(QPoint(topLeftX,topLeftY));
    rect.setBottomRight(QPoint(bottomRightX,bottomRightY));
    lomanBoundedRect = rect;
    return rect;
}

void EnviModule::parseHeaderFinished(bool result, QString message)
{
    if(result){
        channelsNames = m_envi.getEnviHeader().bandNames;
        channelsNumber = channelsNames.count();
        emit channelsNamesChanged();
        headerInfo = m_envi.getHdrList();
        emit headerInfoUpdated();
    }
    else{
        this->message = message;
        emit messageShow();
    }
}

void EnviModule::imageWasLoaded(QString path, bool result)
{

    progressTimer.stop();
    if(!result){
        message = path;
        isFileAbsent = true;
        emit fileImageDataAbsent();
        qInfo()<<"Ошибка загрузки данных"<<message;
        emit messageShow();
        progressTimer.stop();
        return;
    }

    if(m_settings->value("Dirs/saveLastOpenedFolderAsDefault").toBool()){

        bool check = false;
        homePath = path;
        qInfo()<<"Сохранение последней открытой директории (если файл был корректно открыт):\n";
        qInfo()<<homePath<<"\n";
        qInfo()<<m_settings->value("Dirs/defaultEnviFolder").toString()<<"\n";
        check = homePath != m_settings->value("Dirs/defaultEnviFolder").toString();
        qInfo()<<check<<"\n";
        if(check){
            m_settings->setValue("Dirs/defaultEnviFolder",path.remove(path.lastIndexOf("/"),path.count()));
            homePath = path.remove(path.lastIndexOf("/"),path.count());
            qInfo()<<"Директория по усмолчанию изменена.";
            emit pathChanged();
        }
    }


    m_image =     m_envi.getImage();
    widthImage =  m_image.width();
    heightImage = m_image.height();
    QPoint start(0,0);
    QPoint end(widthImage-2,heightImage-2);
    lTpoint = start;
    rBpoint = end;
    qInfo()<<"Ширина загруженой картинки:"<<m_image.width()<<"\n";
    qInfo()<<"Высота загруженой картинки:"<<m_image.height()<<"\n";
    this->update();
    emit sizeChanged();
    emit pictureWasLoaded();
}

void EnviModule::imageChannelsChanged(bool result)
{
    qInfo()<<"Результат изменения отображения подключеных каналов:"<<result;
    if(result){
        m_image = m_envi.getZoomedImage();
        qDebug()<<"zoomed Width image: "<<m_image.width();
        qDebug()<<"zoomed Height image: "<<m_image.height();
        this->update();
        emit bandsWereChanged();
    }else{
        emit fileImageDataAbsent();
        message = "Данные не загружены";
        emit messageShow();
    }
}

void EnviModule::initProgress(int max)
{
    maxProgress = max;
    emit maxProgressChanged();
    progressTimer.start();
}

void EnviModule::progressChanged()
{
    progress = m_envi.getLoadedPixels();
    emit progressPixelsChanged();
}

void EnviModule::addArea()
{

    if(rBpoint.x()>m_image.width()-1||rBpoint.y()>m_image.height()-1){
        message = "Выделенная область выходит за границы изображения";
        emit messageShow();
        return;}

    m_areas->append(QRect(lTpoint*(1/scaleFactor),rBpoint*(1/scaleFactor)));
    qDebug()<<"Area was added with scaleFactor: "<<scaleFactor;
    playSound("areaForSearchingBlackPixelWasCreated.wav");


    QPoint marginPoint;
    marginPoint.setX(lTpoint.x()+50);
    marginPoint.setY(lTpoint.y()+50);
    lTpoint = marginPoint;

    marginPoint.setX(rBpoint.x()+50);
    marginPoint.setY(rBpoint.y()+50);
    rBpoint = marginPoint;

    this->update();
    //emit pictureWasLoaded();
}

void EnviModule::clearAreas()
{

    //if(!m_ignorePolygons.isEmpty())m_ignorePolygons.clear();
    if(m_areas->isEmpty()&&m_ignorePolygons.isEmpty()&&m_points.isEmpty()){
        message = "Всё удалено. Что вы хотите удалить?";
        emit messageShow();
        playSound("nothingToDelete.wav");
        return;
    }
    m_envi.clearAreasAndIgnores();
    isDarkMarker = false;

    if(!m_areas->isEmpty()){

        m_areas->clear();
    }

    if(!m_ignorePolygons.isEmpty()){

        m_ignorePolygons.clear();
        isIgnorePolygonReady = false;

    }
    message = "Все области для поиска удалены.";
    emit messageShow();
    playSound("allAreasCleared.wav");
    //update();

}

void EnviModule::startSearchingDarkestPixel()
{

    if(m_areas->isEmpty()){

        message = "Области для поиска не добавлены!";
        emit messageShow();
        playSound("noAreasForSearchingDarkPixel.wav");
        return;
    }
    emit giveMeTheDarkestPixel(m_areas);
}

void EnviModule::taskForDarkSearchingFinished(bool result, DarkPoint dp,QString path)
{
    qInfo()<<"Поиск тёмного пикселя завершён"<<result;
    pathDark = path;
    emit darkPathWasChanged();
    m_dp = dp;
    lss::setElevationAngle(m_envi.getEnviHeader().sunElevationAngle);
    m_calculeted_values = lss::optimize({dp.chanelsValues[0].toDouble(),
                                         dp.chanelsValues[1].toDouble(),
                                         dp.chanelsValues[2].toDouble(),
                                         dp.chanelsValues[3].toDouble()});
    isDarkMarker = true;
    this->update();
    emit showDarkPoint();
}

void EnviModule::imageWasSavedAsBMP()
{
    QString msg = "Изображение сохранено.";
    qInfo()<<msg;
    message = msg;
    emit messageShow();
    playSound("imageSaved.wav");

}

void EnviModule::zoomImageRedy()
{
    //scaleFactor = m_envi.getScaleFactor();
    m_image =     m_envi.getZoomedImage();
    widthImage =  m_image.width();
    heightImage = m_image.height();
    qInfo()<<"Ширина загруженой картинки:"<<m_image.width()<<"\n";
    qInfo()<<"Высота загруженой картинки:"<<m_image.height()<<"\n";
    this->update();
    emit sizeChanged();
    //emit pictureWasLoaded();
    qDebug()<<"ZOOM scheme works!"<<scaleFactor;
}

void EnviModule::acceptStartSignalCorrection(float correctMultic)
{
    multic = correctMultic;
    qDebug()<<"Скорректированный коэффициент усиления: "<<multic;
    emit multicWasChanged();
}

void EnviModule::selectAllImageForDarkSearching()
{
    QPoint start(0,0);
    QPoint end(widthImage-2,heightImage-2);
    lTpoint = start;
    emit lTpointChanged();
    rBpoint = end;
    emit rBpointChanged();
}

void EnviModule::addLineToLomanArea()
{

    QPoint pointLT;
    QPoint pointBR;
    int xT,yT,xB,yB;
    xT = startPolygonPoint.x()-2;
    yT = startPolygonPoint.y()-2;
    xB = startPolygonPoint.x()+2;
    yB = startPolygonPoint.y()+2;
    pointRect = QRect(QPoint(xT,yT),QPoint(xB,yB));

    pointLT.setX(startPolygonPoint.x()-5);
    pointLT.setY(startPolygonPoint.y()-5);
    pointBR.setX(startPolygonPoint.x()+5);
    pointBR.setY(startPolygonPoint.y()+5);
    m_points.append(startPolygonPoint*(1/scaleFactor));
    playSound("pointIgnorePolygonAdded.aif");
    update();
}

void EnviModule::polygonWasCreated()
{
    qDebug()<<"Polygon is ready....";
    if(m_points.count()<3){
        message = "Для создания полигона нужно минимум 3 точки";
        emit messageShow();
        playSound("minimumThreePointsNeed.wav");
        return;
    }
    for(int i=0;i<m_points.count();++i){
        ignorePolygon.append(m_points[i]);
    }
    ignorePolygon.append(m_points[0]);
    isIgnorePolygonReady = true;
    m_ignorePolygons.append(ignorePolygon);
    m_envi.setIgnorePolygons(m_ignorePolygons);

    ignorePolygon.clear();
    m_points.clear();
    playSound("ignorePolygonAdded.wav");
}

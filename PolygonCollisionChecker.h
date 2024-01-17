#ifndef POLYGONCOLLISIONCHECKER_H
#define POLYGONCOLLISIONCHECKER_H
#include <QPolygon>
#include <QDebug>
#include "tuple"

std::tuple<bool,int>isPointInPolygons(QPoint &checkPoint,QVector<QPolygon> &polygons);

class PolygonCollisionChecker{

public:
    static PolygonCollisionChecker& I(){
       return polyChecker;
    }
    static std::tuple<bool,int> isPointInPolygons(QPoint &checkPoint,QVector<QPolygon> &polygons){

        if(polygons.isEmpty()){
            //qDebug()<<"No Ignore Polygons!";
            return {false,-1};
        };
        //qDebug()<<"Check point "<<checkPoint.x()<<" "<<checkPoint.y();
        QVector<QPolygon>::Iterator it = polygons.begin();
        for(;it<polygons.end();++it){

            //qDebug()<<"IGNORE START POINT: "<<it->at(0).x()<<" "<<it->at(0).y();
            if(it->containsPoint(checkPoint,Qt::WindingFill)){

                return {true,std::distance(it,polygons.begin())};
            }
        }
        return {false,-1};
    }

private:
    PolygonCollisionChecker(){}
    PolygonCollisionChecker(const PolygonCollisionChecker&);
    PolygonCollisionChecker& operator =(const PolygonCollisionChecker&);
    static PolygonCollisionChecker polyChecker;

};



#endif // POLYGONCOLLISIONCHECKER_H

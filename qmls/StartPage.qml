import QtQuick 2.11
import EnviModule 1.0
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.2

Rectangle {

    id:_startPage
    property string pathToFile;
    property color fullOpacity:"#00000000"
    property int widthBorder:2
    property color colorBorder:"#40000000"
    property int imageMargin:20
    property int radiusButtons:10
    property int mainFrameRadius:10
    property bool isRemont:false
    height:_MainWindow.height
    width:_MainWindow.width
    color:"#5b6a75";

    Rectangle{

        id:_frame
        width: _MainWindow.minimumWidth
        height: _MainWindow.minimumHeight
        anchors.horizontalCenter: _startPage.horizontalCenter
        anchors.verticalCenter: _startPage.verticalCenter
        color:"#80095074"
        radius:mainFrameRadius
        border.color: colorBorder
        border.width: widthBorder

        Column{
            id:_column
            anchors.verticalCenter: _frame.verticalCenter
            anchors.horizontalCenter: _frame.horizontalCenter
            spacing:50

            ComboBox{
                id:_satellites_list
                width:400
                model:_enviPage.satelliteList
            }

            CustomButton{

                id:_loadDataButton
                anchors.horizontalCenter: _column.horizontalCenter
                width:400
                height:50
                radius:radiusButtons
                border.color: colorBorder
                border.width: widthBorder
                textButton: "Загрузить данные ENVI"
                textToolTip: "Открыть данные формата ENVI"
                focus: true

                onClicked: {

                    _enviPage.updateCurrentSatellite(_satellites_list.currentText)
                    _MainWindow.currentPage = Envi.EnviPage
                    _enviPage.playSound("buttonPressed.mp3")

                }
            }

            CustomButton{

                id:_settingsButton
                anchors.horizontalCenter: _column.horizontalCenter
                width:400
                height:50
                border.color: colorBorder
                border.width: widthBorder
                radius:radiusButtons
                textButton: "Настройки пользователя"
                isToolTip: false
                focus: true

                onClicked: {
                    _MainWindow.currentPage = Envi.SettingsPage
                    _enviPage.playSound("buttonPressed.mp3")
                }
            }

            CustomButton{

                id:_aboutSoftButton
                anchors.horizontalCenter: _column.horizontalCenter
                width:400
                height:50
                border.color: colorBorder
                border.width: widthBorder
                radius:radiusButtons
                textButton: "О программе"
                isToolTip: false

                onClicked: {
                    _enviPage.playSound("buttonPressed.mp3")
                    _MainWindow.currentPage = Envi.AboutPage
                }
            }

            CustomButton{

                id:_exitButton
                anchors.horizontalCenter: _column.horizontalCenter
                width:400
                height:50
                border.color: colorBorder
                border.width: widthBorder
                radius:radiusButtons
                textButton: "Завершить работу"
                isToolTip: false

                onClicked: {

                    _MainWindow.close()
                    _enviPage.playSound("buttonPressed.mp3")
                }
            }

        }

        Rectangle{
            id:_leftTop
            anchors.top: parent.top
            anchors.left: parent.left
            width:100
            height:100
            radius:50
            color:fullOpacity
            visible:isRemont
            Image{
                anchors.margins: imageMargin
                anchors.fill:parent
                source:"qrc:/pictures/bolt.svg"
                rotation: 15
            }
        }
        Rectangle{
            id:_rightTop
            anchors.top: parent.top
            anchors.right: parent.right
            width:100
            height:100
            radius:50
            color:fullOpacity
            visible:isRemont
            Image{
                anchors.margins: imageMargin
                anchors.fill:parent
                source:"qrc:/pictures/bolt.svg"
                rotation: 30
            }
        }
        Rectangle{
            id:_leftBottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            width:100
            height:100
            radius:50
            color:fullOpacity
            visible:isRemont
            Image{
                anchors.margins: imageMargin
                anchors.fill:parent
                source:"qrc:/pictures/bolt.svg"
                rotation: 45
            }
        }
        Rectangle{
            id:rightBottom
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            width:100
            height:100
            radius:50
            color:fullOpacity
            visible:isRemont
            Image{
                anchors.margins: imageMargin
                anchors.fill:parent
                source:"qrc:/pictures/bolt.svg"
                rotation: 80
            }
        }


        Rectangle{
            id:_intrnalFrame
            anchors.margins: 100
            anchors.fill: _frame
            color: fullOpacity
            border.color: "#40000000"
            border.width: 6
            radius:50
            visible:isRemont

            Image{
                width: 300*1.3
                height:300
                source:"qrc:/pictures/remont.svg"
            }

            Image{
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                width: 300
                height:300
                z:2
                source:"qrc:/pictures/masterRemonta.svg"
            }


            Image{
                anchors.top: parent.top
                anchors.right: parent.right
                width: 300
                height:500
                source:"qrc:/pictures/colorRenovate.svg"
            }

        }

    }
}

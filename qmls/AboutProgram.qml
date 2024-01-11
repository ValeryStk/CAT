import QtQuick 2.0
import EnviModule 1.0
Rectangle {
    id:_aboutSoftInfo
    width:_MainWindow.width
    height:_MainWindow.height
    color:"#5b6a75"

    Image{
        id:_imageLogo
        anchors.horizontalCenter: _aboutSoftInfo.horizontalCenter
        width:256
        height:256
        source:"qrc:/pictures/cat.svg"

        MouseArea{
            anchors.fill: parent
            cursorShape: "PointingHandCursor"
            onClicked: {
                _enviPage.playSound("z_myau.mp3")
            }
        }

    }

    Image{
        id:_leftLapa
        anchors.right: _labelMaster.left
        anchors.verticalCenter: _labelMaster.verticalCenter
        width:256
        height:256
        rotation: -45
        source:"qrc:/pictures/lapa.svg"
    }

    Image{
        id:_rightLapa
        anchors.left: _labelMaster.right
        anchors.verticalCenter: _labelMaster.verticalCenter
        width:256
        height:256
        rotation: 45
        source:"qrc:/pictures/lapa.svg"
    }

    Rectangle{
        id:_labelMaster
        width:600
        height:100
        radius:60
        anchors.horizontalCenter: _imageLogo.horizontalCenter
        anchors.top: _imageLogo.bottom
        color:"#80000080"
        Text{
            height: 100
            width:600
            text:"Я мастер атмосферной коррекции..."
            font.pointSize: 16
            horizontalAlignment: Text.Center
            verticalAlignment: Text.AlignVCenter
        }
    }

    CustomButton{
        id:_mainMenuButton
        anchors.centerIn: _aboutSoftInfo
        textButton: "Главное меню"
        height:50
        radius:5
        isToolTip: false
        onClicked: {
            _MainWindow.currentPage = Envi.StartPage
            _enviPage.playSound("buttonPressed.mp3")

        }

    }

}

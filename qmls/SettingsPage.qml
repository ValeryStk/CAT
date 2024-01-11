import QtQuick 2.0
import EnviModule 1.0

Rectangle {

    id: _SettingsPage
    width:_MainWindow.width
    height:_MainWindow.height
    color:"#5b6a75"

    CustomButton{

        anchors.centerIn: parent
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

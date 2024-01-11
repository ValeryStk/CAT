import QtQml 2.0
import QtQuick 2.11
import QtQuick.Controls 2.3
import QtQuick.Window 2.11
import QtQuick.Layouts 1.3
import EnviModule 1.0


ApplicationWindow {

    id:_MainWindow
    property int currentPage:0
    property int scrWidth: Screen.width
    property int scrHeight: Screen.height
    width: minimumWidth
    height: minimumHeight
    minimumWidth: 1600
    minimumHeight: 900
    visible: true
    title: "CAT "+_enviPage.version;
    //flags: Qt.FramelessWindowHint
    onScreenChanged: {console.log(""+_MainWindow.width)}

    Rectangle{
        id:_PagesContainer
        anchors.fill:parent
        color:"yellow"


        StackLayout{
            id:_stackLayout
            currentIndex: _MainWindow.currentPage
            anchors.fill: parent

            StartPage{
                id:_startPage

            }

            SettingsPage{
                id:_settingsPage
            }

            EnviPage{
                id:_enviPage
                Keys.onPressed: {
                    if (event.key === Qt.Key_Alt) {
                        altPressed();
                        event.accepted = true;
                    }
                }

            }
            AboutProgram{
                id:_aboutPage
            }

            onCurrentIndexChanged: {

                switch(currentIndex){

                case Envi.StartPage:forceActiveFocus(_startPage);
                    break;

                case Envi.SettingsPage:forceActiveFocus(_settingsPage);
                    break;

                case Envi.EnviPage:forceActiveFocus(_enviPage);
                    _enviPage.pageOpen();
                    break;

                case Envi.AboutPage:forceActiveFocus(_aboutPage);
                    break;

                }
            }

        }

    }

}



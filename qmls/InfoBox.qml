import QtQuick 2.0
import QtQuick.Controls 2.3

ApplicationWindow{
    id:_appRectangle
    minimumHeight: 620
    maximumHeight: 620
    maximumWidth: 420
    minimumWidth: 420
    property int fontSize:9
    title: "Тёмный кандидат"
    property string text:""
    property string url:"qrc:/pictures/information.svg"
    Rectangle{

        id:_infoBox
        width:_appRectangle.width
        height:_appRectangle.height
        color:"gray"
        visible:true

        Rectangle{
            id:_textBox
            anchors.top: _infoBox.top
            width:_infoBox.width
            height:150
            color:"gray"
            Text{
                anchors.topMargin: 5
                anchors.leftMargin: 10
                anchors.fill: parent
                text:_appRectangle.text
                wrapMode: Text.Wrap
                antialiasing: true
                font.pointSize: fontSize

            }
        }

        Image{
            id:_imageDarkArea
            anchors.topMargin: 100
            anchors.centerIn:_infoBox
            source: _appRectangle.url
        }

        Rectangle{
            id:_chooseDarkVariant
            anchors.bottom:_infoBox.bottom
            width: parent.width
            height: 70
            color: "gray"
            CustomButton{
                id:_ignoreButton
                anchors.leftMargin: 10
                anchors.left: _chooseDarkVariant.left
                anchors.verticalCenter: _chooseDarkVariant.verticalCenter
                width:200
                height:40
                fontSize: _appRectangle.fontSize
                textButton: "Игнорировать и искать"
                onClicked: ignoreAndSearchAgain()
            }
            CustomButton{
                id:_AcceptButton
                anchors.leftMargin: 10
                anchors.left: _ignoreButton.right
                anchors.verticalCenter: _chooseDarkVariant.verticalCenter
                width:90
                height:40
                fontSize: _appRectangle.fontSize
                textButton: "Принять"
            }
            CustomButton{
                id:_CancelButton
                anchors.rightMargin: 10
                anchors.right: _chooseDarkVariant.right
                anchors.verticalCenter: _chooseDarkVariant.verticalCenter
                width:90
                height:40
                fontSize: _appRectangle.fontSize
                textButton: "Отмена"
            }

        }

    }

    signal ignoreAndSearchAgain();
}

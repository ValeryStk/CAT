import QtQuick 2.0
import QtQuick.Controls 2.3

Rectangle {

    id:_customButton
    property color hoveredColor:"#145475"
    property color normalColor:"#046291"
    property color startGradientColor:"#ffffff"
    property string textButton:""
    property string textToolTip:""
    property int delayToolTipTime:1200
    property int fontSize:20
    property bool isToolTip:false
    opacity: 0.8
    width:300
    height:150
    //gradient:_mouseArea.containsMouse ? _hoverGradient:_normalGradient
    color:_mouseArea.containsMouse ? hoveredColor:normalColor
    radius:0
    border.color: "black"
    border.width: 2
    ToolTip{
        visible: _mouseArea.containsMouse&&isToolTip
        delay: delayToolTipTime
        text:textToolTip
        font.pointSize: 12
    }



    Text{

        id:_text
        anchors.centerIn: _customButton
        text: textButton
        font.pointSize: fontSize

    }


    MouseArea{

        id:_mouseArea
        anchors.fill: _customButton
        hoverEnabled: true
        onPressed: {

            hoveredColor = "#8cfffb"
        }

        onReleased: {
            hoveredColor = "#145475"
            if(_mouseArea.containsMouse)_customButton.clicked()
        }

    }
    signal clicked();



    Gradient {
        id:_normalGradient
        GradientStop {
            position: 0.0; color: startGradientColor
        }
        GradientStop {
            position: 1.0; color: normalColor
        }
    }

    Gradient {
        id:_hoverGradient
        GradientStop {
            position: 0.0; color: "#0e5599"
        }
        GradientStop {
            position: 1.0; color: "#8cd9ff"
        }
    }

    Gradient {
        id:_pressedGradient
        GradientStop {
            position: 0.0; color: "#0e5599"
        }
        GradientStop {
            position: 1.0; color: "#8cd9ff"
        }
    }
}

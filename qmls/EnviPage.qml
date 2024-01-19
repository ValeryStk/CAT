import QtQuick 2.0
import QtQuick 2.6
import QtQml 2.3
import QtQuick 2.11
import EnviModule 1.0
import QtQuick.Controls 2.4
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.3
import QtQuick.Extras 1.4
import QtQuick.Controls.Styles 1.4


Rectangle {

    id:_enviPage
    color: "#5b6a75"
    readonly property int  delayCloseMessage: 2000
    readonly property string version: _envi.version
    property string path:_envi.homeEnviPath()
    property bool   isInitiazed:false
    property real   zoomFactor:zoomSpinBox.value


    Flickable   {
        id: _flickArea
        anchors.centerIn: _enviPage
        implicitWidth:{contentWidth<_MainWindow.scrWidth-100?contentWidth:_MainWindow.scrWidth-100}//1600
        implicitHeight:{contentHeight<_MainWindow.scrHeight-100?contentHeight:_MainWindow.scrHeight-200}// //900
        focus: true
        clip: true
        contentWidth:_envi.widthImage
        contentHeight:_envi.heightImage
        boundsBehavior: Flickable.StopAtBounds
        onFlickStarted: {myMouseArea.startFlick(Qt.point(contentX, contentY))}
        onFlickEnded: {myMouseArea.endFlick(Qt.point(contentX, contentY))}

        MouseArea{

            id: myMouseArea
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            property point cursorPos
            property bool cursorPosActive : false

            // new code to handle flicking
            property point startFlickContentRef: Qt.point(0, 0)
            property point startFlickMousePos: Qt.point(mouseX, mouseY)
            property bool flicking: false
            function setStartFlickMousePos(x,y) {
                // "prepare" for flicking whenever we have a legitimate mouse pos...
                if (!flicking)  // ...but are not actually flicking!
                    startFlickMousePos = Qt.point(x,y)
            }
            function startFlick(refPt) {
                // Note: startFlick() might be called multiple times before endFlick() is called...
                if (!flicking) {  // ...so this is an important check!

                    startFlickContentRef = refPt
                    flicking = true
                }
            }
            function endFlick(refPt) {
                if (flicking) {
                    var diffX = refPt.x - startFlickContentRef.x
                    var diffY = refPt.y - startFlickContentRef.y
                    //statBar.cursorPos =Qt.point(startFlickMousePos.x + diffX,startFlickMousePos.y + diffY)
                    flicking = false
                }
            }

            anchors.fill: parent
            hoverEnabled: true
            cursorShape:{
                if(_chooserControl.isSelectMode){

                    return Qt.CrossCursor
                }else{
                    if(pressed){
                        return Qt.ClosedHandCursor

                    }else{

                        return Qt.OpenHandCursor
                    }

                }
            }
            onClicked: {

                if(!_envi.isSelectMode){return}// MouseEvent OK
                cursorPos = Qt.point(mouse.x, mouse.y)
                setStartFlickMousePos(mouse.x, mouse.y)
                //_labelX.text = "Координаты (x,y): "+cursorPos.x+" "+cursorPos.y

                if(mouse.button & Qt.RightButton) {
                    if(_envi.getMode()===Envi.SelectRectArea){
                        _envi.rBpoint.x = cursorPos.x
                        _envi.rBpoint.y = cursorPos.y
                        _envi.update();
                    }else{
                        if(!_envi.getIsLomanStarted()){
                            _envi.lastPolygonPoint.x = cursorPos.x
                            _envi.lastPolygonPoint.y = cursorPos.y
                        }else{
                            _envi.lastPolygonPoint = _envi.beginPolPoint()

                        }
                        _envi.endPolygon();
                        _envi.update()
                    }
                }

                if(mouse.button & Qt.LeftButton){
                    if(_envi.getMode()===Envi.SelectRectArea){

                        _envi.lTpoint.x = cursorPos.x;
                        _envi.lTpoint.y = cursorPos.y;
                        _envi.update();

                    }else{

                        _envi.startPolygonPoint.x = cursorPos.x;
                        _envi.startPolygonPoint.y = cursorPos.y;
                        _envi.addLine();
                        _envi.update();
                    }
                }

            }
            onEntered: {
                // here: mouseX/Y OK?
                cursorPos = Qt.point(mouseX, mouseY)
                setStartFlickMousePos(mouseX, mouseY)
                cursorPosActive = true
            }
            onExited: { cursorPosActive = false }
            onPositionChanged: {  // MouseEvent OK
                cursorPos = Qt.point(mouse.x, mouse.y)
                setStartFlickMousePos(mouse.x, mouse.y)
            }
            onWheel: {  // WheelEvent OK
                cursorPos = Qt.point(wheel.x, wheel.y)
                setStartFlickMousePos(wheel.x, wheel.y)
                wheel.accepted = false; // pass to Flickable
            }


        }

        Envi{

            id:_envi
            width: _envi.widthImage/scale
            height: _envi.heightImage/scale
            visible: true
            lTpoint.x: 0
            lTpoint.y: 0
            rBpoint.x: _envi.widthImage-2
            rBpoint.y: _envi.heightImage-2

            onChannelsNamesChanged: {

                _channelsView.update()

            }

            onMulticChanged: {

                _signalControl.refreshTextValue()
                _signalControl.update()
                console.info("Multic v qml: "+_envi.multic)
            }

            onPictureWasLoaded: {

                initChannels();
                _envi.visible = true;
                _channelsButton.isChanged = false;
                _progressBar.value = _envi.maxProgress;
                _progressBar.textTask = "Изображение загружено.";
                _envi.playSound("dataWasSuccessfulyLoaded.wav");
                _closeTimer.start();
                isInitiazed = true;

            }

            onBandsWereChanged: {

                _envi.visible = true
                _progressBar.value = _envi.maxProgress
                _progressBar.textTask = "Каналы изменены."
                _closeTimer.start()
            }

            onProgressPixelsChanged: {

                _progressBar.value = _envi.progress
            }

            onMaxProgressChanged:{

                _progressBar.to = _envi.maxProgress
            }

            onMessageShow: {

                messageDialog.open()
            }

            onFileImageDataAbsent: {
                _progressBar.visible = false
                _envi.playSound("dataLoadedError.wav");
            }

            onShowDarkPoint: {

                _iBox.url = _envi.pathDark
                _iBox.text = _envi.darkPointInfo()
                _iBox.dark_pixels_solution_text = _envi.darkPointSolution()
                _iBox.dark_pixels_errors = _envi.darkPointErrors()
                _iBox.visible = true
                _iBox.show()
                console.info("\nСценарий после нахождения тёмного пикселя\n")
                _envi.playSound("blackFounded.wav");
                //var x = {flickX:0}
                //var y = {flickY:0}
                //flickCalculator(x,y)
                //console.info("X and Y flicking:"+x.flickX+" "+y.flickY)
                //_flickArea.flick(x.flickX,y.flickY);
            }

            Timer{
                id:_closeTimer
                running: false
                interval: delayCloseMessage
                repeat: false

                onTriggered: {
                    _progressBar.visible = false
                    _progressBar.value = 0
                    _progressBar.textTask = ""
                }
            }

        }

    }
    CustomButton{
        id:_menuButton
        anchors.top: _enviPage.top
        anchors.left:_enviPage.left
        normalColor: "#797979"
        width:50
        height:50
        radius:0
        opacity: 0.5
        fontSize: 30
        isToolTip: true
        textToolTip: "Главное меню"
        Image{
            anchors.margins: 10
            anchors.fill: parent
            source: "qrc:/pictures/menu.svg"
        }

        onClicked: {
            _filesPopUp.open()
        }

        Popup{
            id:_filesPopUp
            x: 0
            y: 0
            width: 200
            height: 300
            modal: true
            focus: true
            leftMargin: 0
            leftPadding: 5
            rightPadding: 0
            rightMargin: 0
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
            background: Rectangle{color:"#5b6a75";}

            ColumnLayout {
                anchors.fill: parent

                CustomButton {
                    normalColor: "#797979";
                    width:_filesPopUp.width-10;
                    height:50;
                    fontSize: 10;
                    textButton: "Выбрать файл"
                    radius:0
                    onClicked: {_filesPopUp.close();_fileDialog.open();}
                }

                CustomButton {
                    normalColor: "#797979";
                    width:_filesPopUp.width-10;
                    height:50;
                    fontSize: 10;
                    radius:0
                    textButton: "Выбрать область"
                    onClicked: {
                        _envi.message = "Находится в процессе разработки...."
                        messageDialog.open()
                    }
                }

                CustomButton {
                    normalColor: "#797979";
                    width:_filesPopUp.width-10;
                    height:50;
                    fontSize: 10;
                    radius:0
                    textButton: "Сохранить как BMP"
                    onClicked: {
                        _envi.savePicture()
                    }
                }

                CustomButton {
                    normalColor: "#797979";
                    width:_filesPopUp.width-10;
                    height:50;
                    fontSize: 10;
                    radius:0
                    textButton: "Главное меню"
                    isToolTip: true
                    textToolTip: "Главное меню"
                    delayToolTipTime: 2000


                    onClicked: {
                        _filesPopUp.close()
                        _MainWindow.currentPage = Envi.StartPage
                    }
                }

                CustomButton {
                    normalColor: "#797979";
                    width:_filesPopUp.width-10;
                    height:50;
                    fontSize: 10;
                    textButton: "Закрыть программу"
                    radius:0
                    onClicked: {_filesPopUp.close();_MainWindow.close()}
                }

            }
        }
    }
    CustomButton{
        id: _infoHeader
        anchors.top: _enviPage.top
        anchors.left:_menuButton.right
        normalColor: "#797979"
        width:50
        height:50
        radius:0
        opacity: 0.5
        textToolTip: "Текст заголовочного файла"
        isToolTip: true




        Image{
            anchors.margins: 5
            anchors.fill: _infoHeader
            source: "qrc:/pictures/information.svg"
        }
        onClicked:{

            _headerPopup.open()
        }
        Popup{
            id:_headerPopup
            width:1400
            height:700
            x:50
            y:100
            background: Rectangle{color:"#8023221b";}
            ListView{
                id:_listViewForHeaderInfo
                width:1400
                height:680
                clip: true
                spacing:2
                leftMargin: 0
                rightMargin: 0

                model:_envi.headerInfo
                delegate: Rectangle{
                    id:_rectInfoHeaderDelegate
                    width:_headerPopup.width-25
                    height:80
                    color:"#636173"


                    Text{
                        anchors.leftMargin: 10
                        anchors.left: _rectInfoHeaderDelegate.left
                        width:_rectInfoHeaderDelegate.width-10
                        height:_rectInfoHeaderDelegate.height
                        renderType: Text.NativeRendering
                        text: model.modelData
                        wrapMode: Text.Wrap
                        font.pointSize: 11
                        verticalAlignment: Text.AlignVCenter
                    }
                }

            }

        }

    }
    CustomButton{
        id:_openFolderButton
        anchors.top: _enviPage.top
        anchors.left: _infoHeader.right
        normalColor: "#797979"
        width:50
        height:50
        radius:0
        opacity: 0.5
        textToolTip: "Открыть папку с темновыми областями"
        isToolTip:true

        Image{
            anchors.margins: 10
            anchors.fill: parent
            source:"qrc:/pictures/open.svg"
        }
        onClicked: {
            _envi.openImagesFolder()
        }
    }

    DoubleSpinBox{
        id:zoomSpinBox
        anchors.rightMargin: 100
        anchors.right: _chooserControl.left
        implicitWidth: 240
        implicitHeight: 50
        stepSize: 0.1
        wrap: false
        from:0.1
        value:1
        to:1.2
        focus:false
        editable: false
        background: Rectangle{
            anchors.centerIn: parent
            width:100
            height:50
            z:3
        }

        Image{
            id:_zoomImage
            anchors.leftMargin: 50
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            mirror: true
            width:30
            height:30
            opacity:0.7
            source:"qrc:/pictures/zoom.svg"
            z:4
        }
        Rectangle{
            width:90
            height:50
            anchors.left: _zoomImage.right
            z:4
            Text{
                id:_percentageText
                anchors.centerIn: parent
                anchors.leftMargin: 5
                anchors.left: parent.left
                text:" "+(zoomSpinBox.value*100).toFixed(0).toString()+" %"
                font.pointSize: 16

            }
        }

        onValueChanged: {
            _envi.scaleFactor = value
            _envi.zoomScaleFactor(value)
        }
    }

    Rectangle   {
        id:_channelsContainer
        anchors.horizontalCenter:_enviPage.horizontalCenter
        width:childrenRect.width
        height:childrenRect.height
        color:"#00000000"
        CustomButton{
            id:_channelsButton
            property bool isChanged:false
            anchors.left:parent.left
            normalColor: "#797979"
            width: 50
            height:50
            radius:0
            opacity: 0.5
            fontSize: 13
            textToolTip:"Выбор активных каналов"
            isToolTip:true


            Image{
                anchors.margins: 10
                anchors.fill: parent
                source:"qrc:/pictures/bgr.svg"
            }
            onClicked: {
                _channelsPanelPopUp.open()
            }

            Popup{
                id:_channelsPanelPopUp
                x: _channelsContainer.width/2 - 400
                y: 150
                width: 800
                height:500
                modal: true
                focus: true
                leftMargin: 0
                leftPadding: 0
                rightPadding: 0
                rightMargin: 0
                closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
                background: Rectangle{color:"#8023221b";}
                onClosed: {
                    if(_channelsButton.isChanged){
                        //_applyChangesButton.clicked();
                        applyChanges();
                        _channelsButton.isChanged = false
                    }
                }
                ListView{
                    id:_channelsView
                    anchors.fill:parent
                    model:_envi.channelsNames
                    clip:true
                    spacing: 10
                    delegate: Rectangle{

                        id:_checkerContainer
                        property int   indexBox:index
                        property bool  isChecked:false
                        property color currentColor:{
                            if(indexBox+1 === 1)  return "blue"
                            if(indexBox+1 === 2)  return "green"
                            if(indexBox+1 === 3)  return "red"
                            return "white"
                        }
                        property string currentColorID:{
                            if(indexBox+1 === 1)  return "blue"
                            if(indexBox+1 === 2)  return "green"
                            if(indexBox+1 === 3)  return "red"
                            return "white"
                        }
                        height:50
                        width:_channelsPanelPopUp.width
                        color:"#636173"

                        Row{
                            id:_rowChannels
                            spacing:10
                            leftPadding:10

                            Text {
                                width:_channelsPanelPopUp.width-
                                      _checkBox.width-
                                      _rowChannels.spacing-_rowChannels.leftPadding
                                height:50
                                renderType: Text.NativeRendering
                                text: model.modelData
                                wrapMode:Text.Wrap
                                antialiasing: true
                                verticalAlignment: Text.AlignVCenter
                                font.pixelSize: 16
                            }
                            CheckBox{
                                id:_checkBox
                                width: 50
                                height:50
                                checked:_checkerContainer.isChecked

                                indicator:Rectangle{
                                    anchors.centerIn: parent
                                    implicitWidth: 40
                                    implicitHeight: 40
                                    x: _checkBox.leftPadding
                                    y: _checkBox.height / 2 - height / 2
                                    radius: 3
                                    border.width:2
                                    border.color: _checkBox.down ? "#c3c3c3" : "#585858"

                                    Rectangle{
                                        width: 30
                                        height:30
                                        x: 6
                                        y: 6
                                        radius: 2
                                        color: _checkerContainer.currentColor
                                        visible: _checkBox.checked
                                    }
                                }

                                onCheckedChanged: {

                                    _checkerContainer.isChecked = _checkBox.checked

                                    if(isInitiazed){

                                        bandButtonStateWasChanged()
                                        _channelsButton.isChanged = orginizeOrder(_checkerContainer.indexBox);
                                    }
                                }
                            }

                        }
                    }

                }

            }

        }
        Rectangle   {

            id:_signalControl
            anchors.top: _channelsContainer.top
            anchors.left: _channelsButton.right
            width:200
            height:50
            color:"#00000000"




            signal refreshTextValue()
            onRefreshTextValue: {
                console.info("Обновление значения уровня сигнала:"+_envi.multic)
                _multiplicator.setValue(_envi.multic)
            }
            DoubleSpinBox{
                id:_multiplicator
                width:150
                height:48
                font.pointSize: 12
                value: _envi.multic
                hoverEnabled: true

                ToolTip{
                    text: "Уровень сигнала"
                    visible:_multiplicator.hovered
                    delay: 2000
                    font.pointSize: 12
                }

                Keys.onReturnPressed:{

                    _envi.setMultiplicator(_multiplicator.value)
                    console.info(" Значение уровня:"+_multiplicator.value+"\n")
                    forceActiveFocus(_signalControl)
                    focus = false
                    //_applyChangesButton.clicked()
                    applyChanges();

                }
                onValueModified: {

                    _envi.setMultiplicator(value)                }

            }


            CustomButton{
                id:_applyChangesButton
                anchors.verticalCenter: _signalControl.verticalCenter
                anchors.right: _signalControl.right
                normalColor: "#797979"
                opacity:0.5
                width:50
                height:50
                isToolTip: true
                textToolTip: "Вкл выкл голосовой помощник"
                property bool isSound: false
                Image{
                    anchors.margins: 10
                    anchors.fill: _applyChangesButton
                    source:{
                        if(_applyChangesButton.isSound)return "qrc:/pictures/sound_on.svg";
                         if(!_applyChangesButton.isSound)return "qrc:/pictures/sound_off.svg";
                    }
                }

                onClicked: {

                    //_envi.visible = false;
                   // _progressBar.textTask = "Изменение состояния каналов...."
                   // _progressBar.visible = true;
                    //changeBands()
                     isSound ? isSound = false:isSound = true

                    _envi.changeSoundState(isSound);
                    if(isSound)playSound("soundOn.wav");
                    if(!isSound)playSound("noSound.mp3");


                }
            }


        }
    }
    Rectangle   {
        id:_chooserControl
        property bool isSelectMode:_chooseMode.checked
        anchors.right:_enviPage.right
        anchors.top: _enviPage.top
        width:_rowSelect.width
        height:50
        color:"#8023221b"

        Row{
            id:_rowSelect
            anchors.right: _chooserControl.right
            anchors.verticalCenter: _chooserControl.verticalCenter
            spacing:0

            CustomButton{
                id:_removeBlackAreas
                width:50
                height:50
                normalColor: "#797979"
                opacity: 0.5
                textToolTip: "Удаление всех областей"
                isToolTip: true




                Image{
                    anchors.margins: 10
                    anchors.fill: parent
                    source:"qrc:/pictures/bin.svg"
                }
                onClicked: {
                    _envi.clearAllAreas()
                    _envi.update()
                }
            }

            CustomButton{
                id:_addBlackArea
                width:50
                height:50
                normalColor: "#797979"
                opacity: 0.5
                textToolTip:  "Добавление области для поиска ТП"
                isToolTip: true


                Image{
                    anchors.margins: 10
                    anchors.fill: parent
                    source: "qrc:/pictures/add.svg"
                }

                onClicked: {
                    if(_chooseMode.checked){
                        if(_envi.getMode()===Envi.SelectRectArea)_envi.addAreaToList();
                    }
                    else
                    {
                        _envi.message = "Для добавления области активируйте режим выделения!"
                        _envi.playSound("activateSelectMode.wav");
                        messageDialog.open()

                    }
                }
            }

            Button{

                id:_chooseMode
                property string mode:"View"
                width:50
                height:50
                checkable: true
                checked:false
                opacity: 0.5

                ToolTip{
                    text: "Выбор режима (Просмотр|Выбор области)"
                    visible: _chooseMode.hovered
                    font.pointSize: 12
                }

                background: Rectangle{
                    color:"#797979"
                    border.color: "black"
                    border.width: 2
                }

                Image{
                    anchors.margins: 10
                    anchors.fill: parent
                    source: {
                        if(_chooseMode.mode === "View") return "qrc:/pictures/eye.svg"
                        if(_chooseMode.mode === "Triangle") return "qrc:/pictures/triangle-target.svg"
                        if(_chooseMode.mode === "Rectangle") return "qrc:/pictures/select.svg"

                    }
                }
                onClicked: {_selectVariantMenu.open()}

                Popup{
                    id:_selectVariantMenu
                    width:50
                    height: 180
                    modal:true
                    leftMargin: 0
                    leftPadding: 0
                    rightPadding: 0
                    rightMargin: 0
                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
                    background: Rectangle{color:"#5b6a75";}
                    Column{
                        spacing: 10
                        CustomButton{
                            id:_LomanAreaMode
                            width:50
                            height: 50
                            onClicked: {
                                _chooseMode.mode = "Triangle"
                                _envi.isSelectMode = true;
                                _chooseMode.checked = true
                                _envi.setMode(Envi.SelectLomanArea)
                                _selectVariantMenu.close()
                            }
                            Image{
                                anchors.margins: 10
                                anchors.fill: parent
                                source: "qrc:/pictures/triangle-target.svg"
                            }

                        }
                        CustomButton{
                            id:_RectAreaMode
                            width:50
                            height: 50
                            onClicked: {
                                _chooseMode.mode = "Rectangle"
                                _envi.isSelectMode = true;
                                _chooseMode.checked = true
                                _envi.setMode(Envi.SelectRectArea)
                                _envi.selectAllArea();
                                _selectVariantMenu.close()
                            }
                            Image{
                                anchors.margins: 10
                                anchors.fill: parent
                                source:"qrc:/pictures/select.svg"
                            }

                        }
                        CustomButton{
                            id:_ViewMode
                            width:50
                            height: 50
                            onClicked: {
                                _chooseMode.mode = "View"
                                _chooseMode.checked = false
                                _envi.isSelectMode = false;
                                _envi.setMode(Envi.View)
                                _selectVariantMenu.close()
                            }
                            Image{
                                anchors.margins: 10
                                anchors.fill: parent
                                source:"qrc:/pictures/eye.svg"
                            }

                        }
                    }
                    onClosed: {_envi.update()}
                }
            }

            CustomButton{
                id:_startBlackSearching
                width:50
                height:50
                normalColor: "#797979"
                opacity: 0.5
                textToolTip:"Запуск поиска тёмного пикселя"
                isToolTip: true

                Image{
                    anchors.margins: 10
                    anchors.fill: parent
                    source:"qrc:/pictures/startBlackSearch.svg"
                }
                onClicked: {
                    _envi.findDarkestPixel()
                }
            }
        }

    }


    //Диалоги, прогрессы и.т.д

    FileDialog    {
        id: _fileDialog
        nameFilters: ["Заголовочный файл (*.hdr)"]
        title: "Выберите заголовочный файл Envi (*.hdr)"
        folder: path
        visible:false
        onAccepted: {
            isInitiazed = false
            zoomSpinBox.setValue(1)
            console.info("\nВыбран файл: " + _fileDialog.fileUrls+"\n")
            _enviPage.fileWasChanged(_fileDialog.fileUrls)
            close()
            _progressBar.textTask = "Загрузка файла данных....."
            _progressBar.visible = true;

        }
        onRejected: {
            console.info("Отмена выбора файла")
            close()
            //_darkDialog.open()
        }

    }
    MessageDialog {
        id: messageDialog
        title: "Внимание!"
        text: _envi.message
        modality: Qt.WindowModal
        visible: false
        onAccepted: {
            close();
        }

    }
    ProgressBar   {
        id:_progressBar
        anchors.centerIn: _enviPage
        width:500
        height:30
        from:0
        to:_envi.maxProgress
        value:_envi.progress
        visible:false
        property string textTask:""

        Rectangle{
            anchors.horizontalCenter: _progressBar.horizontalCenter
            anchors.bottom: _progressBar.top
            color: "#805b6a75"
            radius:5
            width:_progressBar.width
            height:childrenRect.height
            Text {

                id: name
                width:parent.width
                text: _progressBar.textTask
                font.pointSize: 20
                visible: true
                antialiasing: true
                color: "black"
                horizontalAlignment: Text.AlignHCenter

            }

        }

    }



    //Графические элементы для отладки

    Rectangle{
        id:_currentCoordinates
        anchors.right:_flickArea.right
        anchors.bottom:_enviPage.bottom
        width: childrenRect.width
        height: childrenRect.height
        color:"#c0000000"
        visible: false
        Text{
            id:_labelX
            text:"X:"+myMouseArea.cursorPos.x+
                 " Y:"+myMouseArea.cursorPos.y;
            color:"white"
            font.pointSize: 20
        }
    }
    Rectangle{
        id:_zoomArea
        anchors.bottom: _enviPage.bottom
        anchors.horizontalCenter: _enviPage.horizontalCenter
        radius:100
        width:200
        height:200
        z:100
        color:"gray"
        visible:false
    }
    InfoBox {
        id:_iBox
        visible: false
        onIgnoreAndSearchAgain:{
            _envi.ignoreTheDarkPixelAndFindNew();
            close();
        }
        onAccept: {
        console.log("accept event is under construction...")
        }
        onCancel: {
        close()
        }
    }

    //Сигналы, слоты
    onPageOpen: {_fileDialog.open() ;forceActiveFocus(_enviPage)}
    signal pageOpen();

    onBandButtonStateWasChanged: {

        console.info("Доступное количество каналов:"+_channelsView.count+"\n")
        var counter = 0;
        for(var i=0;i<_channelsView.count;++i){

            _channelsView.currentIndex = i
            if(_channelsView.currentItem.isChecked)++counter;

            if(counter>3){
                _channelsView.currentItem.isChecked = false
                _channelsView.currentItem.currentColorID = "white"
                console.info("Отключаю лишний канал: "+i+" "+_channelsView.currentItem.isChecked+"\n")
            }

        }
    }
    signal bandButtonStateWasChanged();

    onFileWasChanged: {

        _envi.visible = false;
        _envi.silentClearAreas();
        _envi.openEnvi(path);
    }
    signal fileWasChanged(string path);

    onPlaySound:{
        _envi.playSound(SoundIndex);
    }
    signal playSound(var SoundIndex);

    onAltPressed:{
        if(_envi.getMode()===Envi.SelectLomanArea){_envi.addLine()}
        if(_envi.getMode()===Envi.SelectRectArea)_addBlackArea.clicked()
    }
    signal altPressed();


    //Функции:

    function initChannels(){
        //По умолчанию загружаются первые три канала

        var channelsNumber = _channelsView.count
        console.info("\nИнициализация первых трёх каналов в активный режим (доступно):"+_channelsView.count)
        if(channelsNumber>0){
            _channelsView.currentIndex = 0;
            _channelsView.currentItem.isChecked = true;
            _channelsView.currentItem.currentColorID = "blue"
        }
        if(channelsNumber>1){
            _channelsView.currentIndex = 1;
            _channelsView.currentItem.isChecked = true;
            _channelsView.currentItem.currentColorID = "green"
        }
        if(channelsNumber>2){
            _channelsView.currentIndex = 2;
            _channelsView.currentItem.isChecked = true;
            _channelsView.currentItem.currentColorID = "red"
        }
        if(channelsNumber>3){
            for(var i = 3; i < channelsNumber;++i){
                _channelsView.currentIndex = i;
                _channelsView.currentItem.isChecked = false;
                _channelsView.currentItem.currentColorID = "white"
            }
        }
    }

    function changeBands(){

        var stringArr = new Array(_channelsView.count)

        for(var i=0;i<_channelsView.count;++i){

            _channelsView.currentIndex = i
            if(_channelsView.currentItem.currentColorID === "red")  {

                stringArr[i]="red";
                continue;
            }
            if(_channelsView.currentItem.currentColorID === "green"){

                stringArr[i]="green";
                continue;
            }
            if(_channelsView.currentItem.currentColorID === "blue") {

                stringArr[i]="blue";
                continue;
            }
            stringArr[i]="noColor";

        }
        _envi.changeChannels(stringArr)
    }

    function flickCalculator(x,y){


        var contWidth = _flickArea.contentWidth
        var contHeight =_flickArea.contentHeight
        var vX = _flickArea.visibleArea.xPosition
        var vY = _flickArea.visibleArea.yPosition
        var wRatio =_flickArea.visibleArea.widthRatio
        var hRatio =_flickArea.visibleArea.heightRatio


        x.flickX = 0//contWidth - _envi.dp().x - contWidth*vX/2;
        y.flickY = 0//contHeight -_envi.dp().y - contHeight*vY/2;
    }

    function orginizeOrder(index){

        var isRedFree    =  true;
        var isGreenFree  =  true;
        var isBlueFree   =  true;
        var changedIndex = index

        for(var i=0;i<_channelsView.count;++i){
            _channelsView.currentIndex = i;
            if(_channelsView.currentItem.isChecked){
                if(_channelsView.currentItem.currentColorID === "red")  {console.info("Red is checked");  isRedFree   = false }
                if(_channelsView.currentItem.currentColorID === "green"){console.info("Green is checked");isGreenFree = false }
                if(_channelsView.currentItem.currentColorID === "blue") {console.info("Blue is checked"); isBlueFree  = false }
            }
        }
        console.info("Red is free: "
                     +isRedFree
                     +"\nGreen is free: "
                     +isGreenFree
                     +"\nBlue is free: "
                     +isBlueFree);

        if(isRedFree===false&&isGreenFree===false&&isBlueFree===false)return false;

        _channelsView.currentIndex = changedIndex
        console.info("\nТекущий индекс: "+_channelsView.currentIndex);

        if(_channelsView.currentItem.isChecked){

            if(isRedFree)  {
                isRedFree   = false;
                console.info("Устанавливаю красный цвет.")
                _channelsView.currentItem.currentColorID = "red"
                _channelsView.currentItem.currentColor   = "red"
                return true;
            }

            if(isGreenFree){
                isGreenFree = false;
                _channelsView.currentItem.currentColorID = "green"
                _channelsView.currentItem.currentColor   = "green"
                return true;
            }

            if(isBlueFree) {
                isBlueFree  = false;
                _channelsView.currentItem.currentColorID = "blue"
                _channelsView.currentItem.currentColor   = "blue"
                return true;
            }
        }

        if(!_channelsView.currentItem.isChecked){

            console.info("Освобождаю цвет для: "+_channelsView.currentItem.currentColorID);
            if(_channelsView.currentItem.currentColorID==="red")  {
                isRedFree   = true;
                _channelsView.currentItem.currentColorID = "white"
                return true;
            }
            if(_channelsView.currentItem.currentColorID==="green"){
                isGreenFree = true;
                _channelsView.currentItem.currentColorID = "white"
                return true;
            }
            if(_channelsView.currentItem.currentColorID==="blue") {
                isBlueFree  = true;
                _channelsView.currentItem.currentColorID = "white"
                return true;
            }

        }

    }

    function applyChanges(){
        _envi.visible = false;
        _progressBar.textTask = "Изменение состояния каналов...."
        _progressBar.visible = true;
        changeBands()
    }

    function getSatellites(){
      return _envi.getSatellitesList();
    }
}


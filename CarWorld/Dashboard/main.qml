import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.VirtualKeyboard 2.15
import QtQuick.Extras 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls 1.4

Window {
  width: 640
  height: 480
  visible: true
  title: qsTr("iGeek CarWorld Dashboard")

  Column {
    id: leftPanel
    anchors.left: parent.left
    anchors.top: parent.top

    width: parent.width * 0.2

    Button {
      id: confButton
      text: "Settings"
    }

    Button {
      id: dashButton
      text: "Dashboard"
    }
  }

  ValueSource {
    id: valueSource
  }

  Rectangle {
    id: dashboard
    anchors.centerIn: parent
    height: 500
    width: 800
    color: "black"

    Row {
      id: dashboardRow
      spacing: dashboard.width * 0.02
      anchors.centerIn: parent

      ArrowIndicator {
        id: leftIndicator
        direction: Qt.LeftArrow
        anchors.verticalCenter: parent.verticalCenter
        height: dashboard.height * 0.2 - dashboardRow.spacing
        width: height
      }

      CircularGauge {
        id: rpmMeter
        width: height
        height: dashboard.height * 0.6
        maximumValue: 5000

        style: RPMMeterStyle {}
      }

      CircularGauge {
        id: speedometer
        value: acceleration ? maximumValue : 0
        width: height
        height: dashboard.height * 0.6
        maximumValue: 180

        property bool acceleration: false

        style: SpeedometerStyle {}

        Behavior on value {
          NumberAnimation {
            duration: 9000
          }
        }
        Component.onCompleted:  forceActiveFocus();
      }

      ArrowIndicator {
        id: rightIndicator
        direction: Qt.RightArrow
        anchors.verticalCenter: parent.verticalCenter
        height: dashboard.height * 0.2 - dashboardRow.spacing
        width: height
      }
    }

    Keys.onUpPressed: {
      acceleration = true;
    }

    Keys.onReleased: {
      if (event.key === Qt.Key_Up) {
        speedometer.acceleration = false;
        event.accepted = true;
      }
    }

    Keys.onLeftPressed: {
      leftIndicator.on = true;
      rightIndicator.on = false;
    }

    Keys.onRightPressed: {
      rightIndicator.on = true;
      leftIndicator.on = false;
    }
  }
}

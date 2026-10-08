import QtQuick 2.15
import QtQuick.Controls.Styles 1.4

CircularGaugeStyle {
  id: rpmMeterStyle

  function degree2Rad(deg) {
    return deg * (Math.PI / 180);
  }

  labelStepSize: 1000

  tickmark: Rectangle {
    visible: styleData.value % 100 == 0
    implicitWidth: outerRadius * 0.02
    antialiasing: true
    implicitHeight: outerRadius * 0.06
    color: "#e5e5e5"
  }

  background: Canvas {

    Row {
      anchors.horizontalCenter: parent.horizontalCenter
      anchors.top: parent.verticalCenter
      anchors.topMargin: outerRadius * 0.2

      Text {
        id: speedText
        color: "white"
        font.pixelSize: outerRadius * 0.1
        horizontalAlignment: Text.AlignRight
        text: kphValue
        readonly property int kphValue: speedometer.value
      }

      Text {
        id: unitSpeed
        text: "km/h"
        color: "white"
        font.pixelSize: outerRadius * 0.3

        readonly property string gear: "P"
      }
    }
  }
}

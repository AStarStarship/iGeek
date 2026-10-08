import QtQuick 2.15

Item {
  property int direction: Qt.LeftArrow

  property bool on: false

  property bool blinking: false

  scale: direction === Qt.LeftArrow ? 1 : -1

  Timer {
    id: indicatorTime
    interval: 10000
    running: on
    repeat: false
    onTriggered: on = false
  }

  Timer {
    id: blinkerTimer
    interval: 500
    running: on
    repeat: true
    onTriggered: blinking = !blinking
  }

  function drawArrow (ctr) {
    ctr.beginPath();
    ctr.moveTo(0, height * 0.5);
    ctr.lineTo(0.6 * width, 0);             // 1
    ctr.lineTo(0.6 * width, 0.28 * height); // 2
    ctr.lineTo(width, 0.28 * height);       // 3
    ctr.lineTo(width, 0.72 * height);       // 4
    ctr.lineTo(0.6 * width, 0.72 * height); // 5
    ctr.lineTo(0.6 * width, height);        // 6
    ctr.lineTo(0, height * 0.5);            // 7
  }

  Canvas {
    id: background
    anchors.fill: parent
    onPaint: {
      var ctr = getContext("2d");
      ctr.reset();

      drawArrow(ctr);
      ctr.lineWidth = 1;
      ctr.strokeStyle = "white";
      ctr.stroke();
    }
  }

  Canvas {
    id: foreground
    anchors.fill: parent
    visible: on && blinking

    onPaint: {
      var ctr = getContext("2d");
      ctr.reset();
      drawArrow(ctr);

      ctr.fillStyle = "green";
      ctr.fill();
    }
  }
}

function httpGet(theUrl) {
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.open("GET", theUrl, true);
  xmlHttp.send(null);
}
function udpateCar(speed, direction) {
  document.getElementById("speedOutput").innerText = speed;
  document.getElementById("directionOutput").innerText = direction;
  httpGet(window.location + "speed=" + speed + "/direction=" + direction + "/");
}
function stopCar() {
  udpateCar(0, 0);
}

function pointsDistance(x1, y1, x2, y2) {
  return Math.sqrt(Math.pow(x1 - x2, 2) + Math.pow(y1 - y2, 2));
}

function customize() {
  mode = CUSTOMIZE_MODE;
  interfaceCustomize.style.display = "block";
  interfaceDrive.style.display = "none";
  interfaceInstructions.style.display = "none";
  calculateDimensions();
}

function drive() {
  mode = DRIVE_MODE;
  interfaceCustomize.style.display = "none";
  interfaceDrive.style.display = "block";
  interfaceInstructions.style.display = "none";
  calculateDimensions();
}

function showInstructions() {
  mode = INSTRUCTIONS_MODE;
  interfaceCustomize.style.display = "none";
  interfaceDrive.style.display = "none";
  interfaceInstructions.style.display = "block";
  calculateDimensions();
}

function onLoadPage() {
  drive();
  calculateDimensions();
}

function calculateDimensions() {
  rect = svg.getBoundingClientRect();
  svgX = rect.left;
  svgY = rect.top;

  rectOuterCircle = outerCircle.getBoundingClientRect();
  rectInnerCircle = innerCircle.getBoundingClientRect();

  circleCenterX = (rectInnerCircle.left + rectInnerCircle.right) / 2;
  circleCenterY = (rectInnerCircle.top + rectInnerCircle.bottom) / 2;

  outerCircleRadius = circleCenterX - rectOuterCircle.left;
}

window.onload = onLoadPage;
window.onorientationchange = calculateDimensions;

var mode;
const DRIVE_MODE = 0;
const CUSTOMIZE_MODE = 1;
const INSTRUCTIONS_MODE = 2;

var driveButton = document.getElementById("drive");
var interfaceDrive = document.getElementById("interfaceDrive");
driveButton.onclick = drive;

var instructionsButton = document.getElementById("instructions");
var interfaceInstructions = document.getElementById("interfaceInstructions");
instructionsButton.onclick = showInstructions;

var customizeButton = document.getElementById("customize");
var interfaceCustomize = document.getElementById("interfaceCustomize");
customizeButton.onclick = customize;

var svg = document.getElementById("svg");
var rect;
var svgX;
var svgY;

var outerCircle = document.getElementById("outerCircle");
var outerCircleColor = document.getElementById("outerCircleColor");
var rectOuterCircle;

var innerCircle = document.getElementById("innerCircle");
var innerCircleColor = document.getElementById("innerCircleColor");
var rectInnerCircle;

var circleCenterX;
var circleCenterY;

var outerCircleRadius;

dragElement(document.getElementById("innerCircle"));

function dragElement(element) {
  element.onmousedown = dragMouseDown;
  element.ontouchstart = dragMouseDown;

  function dragMouseDown(e) {
    e = e || window.event;
    e.preventDefault();
    document.onmouseup = closeDragElement;
    element.ontouchend = closeDragElement;

    document.onmousemove = elementDrag;
    element.ontouchmove = elementDrag;
  }

  function elementDrag(e) {
    e = e || window.event;
    e.preventDefault();
    var posX;
    var posY;

    if (e.touches === undefined) {
      var posX = e.clientX;
      var posY = e.clientY;
    } else {
      var posX = e.touches[0].clientX;
      var posY = e.touches[0].clientY;
    }

    var distanceFromCenter = pointsDistance(
      posX,
      posY,
      circleCenterX,
      circleCenterY
    );

    var speed = Math.round((distanceFromCenter / outerCircleRadius) * 100);
    if (posY > circleCenterY) {
      speed = -speed;
    }
    var direction = Math.round(
      ((Math.acos((posX - circleCenterX) / distanceFromCenter) - Math.PI / 2) *
        200) /
        Math.PI
    );
    direction = -direction;
    // if (posX > circleCenterX) {
    //   direction = -direction;
    // }

    if (distanceFromCenter < outerCircleRadius) {
      // set the element's new position:
      element.setAttribute("cx", posX - svgX);
      element.setAttribute("cy", posY - svgY);

      if (mode == DRIVE_MODE) {
        udpateCar(speed, direction);
      }
    }
  }

  function closeDragElement() {
    /* stop moving when mouse button is released:*/
    // element.setAttribute("cx", "50%");
    // element.setAttribute("cy", "50%");
    element.setAttribute("cx", circleCenterX - svgX);
    element.setAttribute("cy", circleCenterY - svgY);
    document.onmouseup = null;
    document.onmousemove = null;

    if (mode == DRIVE_MODE) {
      stopCar();
    }
  }
}

function changeInnerCircleColor() {
  innerCircle.style.fill = innerCircleColor.value;
}
function changeOuterCircleColor() {
  outerCircle.style.fill = outerCircleColor.value;
}

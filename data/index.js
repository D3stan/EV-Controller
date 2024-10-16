var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

var currentDegrees = 0;
var animationSpeed = 0.05; // Adjust this value to control the speed of the gauge animation

var raveRpmClose = 3500
var raveRpmOpen = 9500

var wifiPSW = ""
var wifiSSID = ""


// ----------------------------------------------------------------------------
// Elements
// ----------------------------------------------------------------------------

var statusLed
var raveLed
var gauge
var rpmText
var updateButton
var settingsButton
var popUp
var popUpText

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

window.addEventListener('load', onLoad);

function onLoad(event) {
    // websocket loading
    initWebSocket();

    // html elements
    statusLed = document.getElementById('status-led')
    raveLed   = document.getElementById('rave-led')
    gauge     = document.getElementById('gauge')
    rpmText   = document.getElementById('rpm-value')
    updateButton = document.getElementById('update')
    settingsButton = document.getElementById('settings')
    popUp = document.getElementById('popup')
    popUpText = document.getElementById('popup-text')

    updateButton.addEventListener("click", updateSoftware)
    settingsButton.addEventListener("click", openSettings)
    

}

// ----------------------------------------------------------------------------
// WebSocket handling
// ----------------------------------------------------------------------------

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
    statusLed.className = 'led-green';
}

function onClose(event) {
    console.log('Connection closed');
    updateGauge(0);
    statusLed.className = 'led-red';
    raveLed.className = 'led-red';
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    let data = JSON.parse(event.data);
    if (data.update == 1) {
        // settings update
        wifiPSW = data.wifiPSW
        wifiSSID = data.wifiSSID
        raveRpmClose = data.raveRpmClose
        raveRpmOpen = data.raveRpmOpen

    } else {
        if (data.rpm) {
            updateGauge(data.rpm);
        }
    }
}

// ----------------------------------------------------------------------------
// Functions
// ----------------------------------------------------------------------------

function showPopup() {
    popUp.classList.remove('hidden');
}

function closePopup() {
    popUp.classList.add('hidden');
}

function updateSoftware() {
    popUpText.textContent = "Checking for new update..."
    
    if (popUp.firstElementChild.lastElementChild.tagName == 'BUTTON') {
        popUp.firstElementChild.removeChild(popUp.firstElementChild.lastElementChild)
        popUp.firstElementChild.removeChild(popUp.firstElementChild.lastElementChild)
        popUp.firstElementChild.removeChild(popUp.firstElementChild.lastElementChild)
    }

    // check if loading circle already added
    if (popUp.firstElementChild.lastElementChild.classList.value != "loading-circle") {
        let loadingCircle = document.createElement('div')
        loadingCircle.classList.add('loading-circle')
        popUp.firstElementChild.appendChild(loadingCircle)
    }
    
    showPopup()
}

function openSettings() {
    popUpText.textContent = "Update Settings"

    if (popUp.firstElementChild.lastElementChild.classList.value == "loading-circle") {
        popUp.firstElementChild.removeChild(popUp.firstElementChild.lastElementChild)
    }
    
    // check if inputField already added
    if (popUp.firstElementChild.lastElementChild.tagName != 'BUTTON') {
        let inputField = document.createElement('input')
        inputField.type = "text"
        inputField.placeholder = "wifi name"
        popUp.firstElementChild.appendChild(inputField)
        inputField = document.createElement('input')
        inputField.type = "text"
        inputField.placeholder = "wifi password"
        popUp.firstElementChild.appendChild(inputField)

        let buttonWifi = document.createElement("button")
        buttonWifi.classList.add("wifi-settings")
        buttonWifi.textContent = "SEND"
        popUp.firstElementChild.appendChild(buttonWifi)
        buttonWifi.addEventListener("click", () => {
            wifiPSW = document.getElementsByTagName("input")[0].value
            wifiSSID = document.getElementsByTagName("input")[1].value
            let toSend = JSON.stringify({
                wifiPSW: wifiPSW,
                wifiSSID: wifiSSID
            })
            websocket.send(toSend)
            console.log(toSend)
        })
    }
    
    showPopup()
}

function updateGauge(rpm) {
    const maxRpm = 15000; // Maximum RPM value
    let targetDegrees;
    
    // Calculate the target degrees based on RPM
    if (rpm < maxRpm) {
        targetDegrees = ((rpm * 270) / maxRpm); // Convert the percentage to degrees from 0 to 270
    } else {
        targetDegrees = 270; // Max degrees for max RPM
    }

    currentDegrees = parseFloat(gauge.getAttribute('data-current-degrees')) || 0;

    // Function to animate the transition
    function animateGauge() {
        // Smoothly move currentDegrees towards targetDegrees
        currentDegrees += (targetDegrees - currentDegrees) * animationSpeed;

        // Stop the animation when the change is small enough
        if (Math.abs(targetDegrees - currentDegrees) < 0.5) {
            currentDegrees = targetDegrees; // Snap to the target value when close enough
        } else {
            requestAnimationFrame(animateGauge); // Continue the animation
        }

        // Update the gauge color based on the current degrees
        var raveColor = "green"
        raveLed.className = 'led-green';
        if (rpm > raveRpmClose && rpm < raveRpmOpen) {
            raveColor = "red"
            raveLed.className = 'led-red';
        }

        let gaugeColor;
        if (currentDegrees > 180) {
            gaugeColor = `conic-gradient(
                ${raveColor} 0deg, 
                ${raveColor} ${currentDegrees - 180}deg,
                rgb(170, 170, 170) ${currentDegrees - 180}deg,
                rgb(170, 170, 170) 90deg,
                white 90deg,
                white 180deg,
                ${raveColor} 180deg,
                ${raveColor} 360deg
            )`;
        } else {
            gaugeColor = `conic-gradient(
                rgb(170, 170, 170) 0deg,
                rgb(170, 170, 170) 90deg,
                white 90deg,
                white 180deg,
                ${raveColor} 180deg,
                ${raveColor} ${currentDegrees + 180}deg,
                rgb(170, 170, 170) ${currentDegrees + 180}deg,
                rgb(170, 170, 170) 360deg
            )`;
        }

        // Apply the updated background gradient
        gauge.style.background = gaugeColor;

        // Save the current degrees as a custom attribute for the next animation
        gauge.setAttribute('data-current-degrees', currentDegrees);
    }

    // Start the animation
    animateGauge();

    // Update the RPM text
    rpmText.textContent = rpm;
}

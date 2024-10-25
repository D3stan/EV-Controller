var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var statusPageActive = true

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
var statusPage
var settingsPage
var raveLed
var gauge
var rpmText
var updateButton
var settingsButton
var popUp
var popUpText
var updateSettingsButton
var dropdownButton 
var dropdownOptions
var raveSettings
var wifiSettings

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

window.addEventListener('load', onLoad);

function onLoad(event) {
    // websocket loading
    initWebSocket();

    // html elements
    statusLed = document.getElementById('status-led')
    statusPage   = document.getElementById('status-page')
    settingsPage   = document.getElementById('settings-page')
    raveLed   = document.getElementById('rave-led')
    gauge     = document.getElementById('gauge')
    rpmText   = document.getElementById('rpm-value')
    updateButton = document.getElementById('update')
    settingsButton = document.getElementById('settings')
    popUp = document.getElementById('popup')
    popUpText = document.getElementById('popup-text')
    updateSettingsButton = document.getElementById('update-settings')
    dropdownButton = document.getElementById("dropdownButton");
    dropdownOptions = document.getElementById("dropdownOptions");
    raveSettings = document.getElementById("rave-settings");
    wifiSettings = document.getElementById("wifi-settings");

    updateButton.addEventListener("click", openUpdateDialog)
    settingsButton.addEventListener("click", openSettings)
    updateSettingsButton.addEventListener("click", updateSettings)


    

    // Toggle dropdown visibility on button click
    dropdownButton.addEventListener("click", function () {
        dropdownOptions.parentElement.classList.toggle("open");
    });

    // Select an option and update button text
    dropdownOptions.addEventListener("click", function (event) {
        if (event.target.tagName === "LI") {
            const selectedOption = event.target;
            dropdownButton.textContent = selectedOption.textContent;
            dropdownOptions.parentElement.classList.remove("open");
            


            // Trigger custom actions based on selected option
            handleOptionSelect(selectedOption.getAttribute("data-value"));
        }
    });

    // Close dropdown if clicked outside
    document.addEventListener("click", function (event) {
        if (!dropdownOptions.contains(event.target) && !dropdownButton.contains(event.target)) {
            dropdownOptions.parentElement.classList.remove("open");
        }
    });

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

function openUpdateDialog() {
    popUpText.textContent = "Update Settings"
    
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
        buttonWifi.textContent = "Update"
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

function openSettings() {
    if (statusPageActive) {
        statusPage.style.display = "none"
        settingsPage.removeAttribute("style")
        statusPageActive = false
    } else {
        settingsPage.style.display = "none"
        statusPage.removeAttribute("style")
        statusPageActive = true
    }
    
}

function updateSettings() {
    wifiPSW = document.getElementsByTagName("input")[0].value
    wifiSSID = document.getElementsByTagName("input")[1].value
    let toSend = JSON.stringify({
        wifiPSW: wifiPSW,
        wifiSSID: wifiSSID
    })
    websocket.send(toSend)
    console.log(toSend)
}


// Function to handle option selection
function handleOptionSelect(value) {
    // Add any additional actions here based on the selected option
    if (value === "rave") {
        wifiSettings.style.display = "none"
        raveSettings.removeAttribute("style")

    } else if (value === "wifi") {
        raveSettings.style.display = "none"
        wifiSettings.removeAttribute("style")

    }
}


slider.oninput = function() {
    if (timerID) clearTimeout(timerID)
    valueDisplay.innerText = this.value;
    timerID = setTimeout(() => {
        setRpm()
    }, 200)
    /*
    var req = new XMLHttpRequest();
    req.open('GET', `${window.location.hostname}/rpm?RPM=${this.value}`, true);
    req.send();
    */
};

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

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var statusPageActive = true
var timerID

var currentDegrees = 0;
var animationSpeed = 0.1; // Adjust this value to control the speed of the gauge animation

var raveRpmClose = 3500
var raveRpmOpen = 11500

var wifiPSW     = ""
var wifiSSID    = ""
var apPSW       = ""
var apSSID      = ""

var fwid = "Error"
var fsid = "Error"
var hwid = "Error"


// ----------------------------------------------------------------------------
// Elements
// ----------------------------------------------------------------------------

// Pages
var statusPage
var settingsPage

// Containers
var popUp
var raveSettings
var wifiSettings

// Interactives
var updateButton
var settingsButton
var updateSettingsButton
var dropdownButton
var restartButton
var dropdownOptions

// Indicators
var statusLed
var raveLed
var gauge

// Text fields
var rpmText
var popUpText
var fwidText
var fsidText
var hwidText
var rpmOpenText
var rpmCloseText

// Input Fields
var wifiSsidInput
var wifiPswInput
var apSsidInput
var apPswInput
var rpmOpenSlider
var rpmCloseSlider


// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

window.addEventListener('load', onLoad);

function onLoad(event) {
    // websocket loading
    initWebSocket();

    // html elements
    statusLed               = document.getElementById('status-led')
    statusPage              = document.getElementById('status-page')
    settingsPage            = document.getElementById('settings-page')
    raveLed                 = document.getElementById('rave-led')
    gauge                   = document.getElementById('gauge')
    rpmText                 = document.getElementById('rpm-value')
    updateButton            = document.getElementById('update')
    settingsButton          = document.getElementById('settings')
    popUp                   = document.getElementById('popup')
    popUpText               = document.getElementById('popup-text')
    updateSettingsButton    = document.getElementById('update-settings')
    dropdownButton          = document.getElementById("dropdownButton");
    dropdownOptions         = document.getElementById("dropdownOptions");
    raveSettings            = document.getElementById("rave-settings");
    wifiSettings            = document.getElementById("wifi-settings");
    restartButton           = document.getElementById("restart-button");
    fwidText                = document.getElementById("popup-fwid");
    fsidText                = document.getElementById("popup-fsid");
    hwidText                = document.getElementById("popup-hwid");
    wifiSsidInput           = document.getElementById('wifi-ssid')
    wifiPswInput            = document.getElementById('wifi-psw')
    apSsidInput             = document.getElementById('ap-ssid')
    apPswInput              = document.getElementById('ap-psw')
    rpmOpenSlider           = document.getElementById('rpm-open-slider')
    rpmCloseSlider          = document.getElementById('rpm-close-slider')
    rpmOpenText             = document.getElementById('rpm-open-value')
    rpmCloseText            = document.getElementById('rpm-close-value')



    updateButton.addEventListener("click", showPopup)
    settingsButton.addEventListener("click", openSettings)
    updateSettingsButton.addEventListener("click", updateSettings)
    restartButton.addEventListener("click", restartForUpdate)
    rpmCloseSlider.oninput = updateRpmSliderValue
    rpmOpenSlider.oninput = updateRpmSliderValue

    
    rpmCloseText.innerText = raveRpmClose
    rpmOpenText.innerText = raveRpmOpen


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


    retrieveConfigValues()
    
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
    if (data.update) {
        if (data.update == "updated") {
            // settings updated successfully
            alert("Settings Updated")
        } else {
            // checking for updates
            alert("Rebooting...")
        }
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

function updateAllDialogFields() {
    popUpText.textContent = "Update Settings"
    fwidText.textContent += fwid
    fsidText.textContent += fsid
    hwidText.textContent += hwid

    wifiSsidInput.value   =  wifiSSID
    wifiPswInput.value    =  wifiPSW 
    apSsidInput.value     =  apSSID
    apPswInput.value      =  apPSW

    rpmOpenText = raveRpmOpen.toString()
    rpmCloseText = raveRpmClose.toString()
}

function openSettings() {
    settingsButton.classList.add('rotate');

    if (timerID) clearTimeout(timerID)
    timerID = setTimeout(() => {
        settingsButton.classList.remove('rotate');
    }, 1500)

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
    let toSend

    if (raveSettings.style.display !== "none") {
        // rave settings active
        toSend = JSON.stringify({
            type: "rave-settings",
            raveRpmOpen: raveRpmOpen,
            raveRpmClose: raveRpmClose
        })
        


    } else if (wifiSettings.style.display !== "none") {
        // wifi settings active
        if (
            wifiPswInput.value.length < 8 ||
            apPswInput.value.length < 8
        ) {
            let content = updateSettingsButton.innerText
            updateSettingsButton.innerText = "Error"
            setTimeout(() => {
                updateSettingsButton.innerText = content
            }, 5000)
            alert("Input password must be at least 8 characters long!")
            return
        }
        
        wifiSSID = wifiSsidInput.value
        wifiPSW = wifiPswInput.value

        apSSID = apSsidInput.value
        apPSW = apPswInput.value

        toSend = JSON.stringify({
            type: "wifi-settings",
            wifiPsw: wifiPSW,
            wifiSsid: wifiSSID,
            apPsw: apPSW,
            apSsid: apSSID
        })


    } else {
        console.log("No page active")
        return
    }


    websocket.send(toSend)
    console.log(toSend)
}

function retrieveConfigValues() {
    fetch(`/config.json`, { method: "GET" })
    .then(async res => {
        let config = await res.json()
        
        raveRpmClose = config.raveRpmClose
        raveRpmOpen = config.raveRpmOpen
        
        wifiPSW = config.wifiPsw
        wifiSSID = config.wifiSsid
        apPSW = config.apPsw
        apSSID = config.apSsid

        fwid = config.fwid.substring(config.fwid.indexOf("_") + 1).replaceAll("_", ".")
        fsid = config.fsid.substring(config.fsid.indexOf("_") + 1).replaceAll("_", ".")
        hwid = config.hwid
        
        updateAllDialogFields()
        if (config.lastError != "") {
            if (confirm(config.lastError)) {
                let toSend = JSON.stringify({
                    type: "reset-error"
                })
                websocket.send(toSend)
            }
        }

        /*
        fwidText.innerText = config.fwid
        fsidText.innerText = config.fsid
        hwidText.innerText = config.hwid
        wifiSsidInput.value = config.wifiSsid
        wifiPswInput.value = config.wifiPsw
        apSsidInput.value = config.apSsid
        apPswInput.value = config.apPsw
        */
    })
    .catch(err => console.log(`Error when retrieving config values: ${err}`))
}


function restartForUpdate() {

    if (!wifiPSW || !wifiSSID) {
        console.log("WiFi psw or name not set!")
        let preText = restartButton.textContent
        restartButton.textContent = "Error"
        if (confirm("Set WiFi name and password in the settings page!")) {
            closePopup()
            openSettings()
            dropdownButton.click()
            dropdownOptions.children[1].click()
        }
        
        setTimeout(() => {
            restartButton.textContent = preText
        }, 5000)
        return
    }

    let toSend = JSON.stringify({
        type: "update",
        wifiPsw: wifiPSW,
        wifiSsid: wifiSSID
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

function updateRpmSliderValue() {
    document.getElementById(this.id.substring(0, this.id.lastIndexOf("-") + 1) + "value").innerText = this.value;
    if (this.id.substring(this.id.indexOf("-") + 1, this.id.lastIndexOf("-")) == "close") {
        raveRpmClose = parseInt(this.value)
    } else if (this.id.substring(this.id.indexOf("-") + 1, this.id.lastIndexOf("-")) == "open") {
        raveRpmOpen = parseInt(this.value)
    }

    let min = parseInt(this.min)
    let max = parseInt(this.max)
    let range = (max - min) / 4

    this.style.setProperty('--thumb-color', ((min + range > this.value) || (max - range < this.value)) ? '#f44336' : '#4CAF50');
}


function updateGauge(rpm) {
    clearTimeout(timerID)
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

    timerID = setTimeout(() => {
        updateGauge(0)
    }, 3000)
}

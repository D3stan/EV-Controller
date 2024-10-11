var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var maxRPM = 0;
var currentDegrees = 0;
var animationSpeed = 0.05; // Adjust this value to control the speed of the gauge animation

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
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
    document.getElementById('status-led').className = 'led-green';
}

function onClose(event) {
    console.log('Connection closed');
    document.getElementById('status-led').className = 'led-red';
    //document.getElementById('rpm-value').textContent = "0";
    updateGauge(0);
    document.getElementById('maxrpm').textContent = "MAX RPM: " + 0;
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    let data = JSON.parse(event.data);
    if (data.status)
        document.getElementById('led').className = data.status;
    if (data.rpm) {
        updateGauge(data.rpm);
        if (data.rpm > maxRPM) maxRPM = data.rpm;
        document.getElementById('maxrpm').textContent = "MAX RPM: " + maxRPM;
    }
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

    currentDegrees = parseFloat(document.getElementById('gauge').getAttribute('data-current-degrees')) || 0;

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
        let gaugeColor;
        if (currentDegrees > 180) {
            gaugeColor = `conic-gradient(
                red 0deg, 
                red ${currentDegrees - 180}deg,
                rgb(170, 170, 170) ${currentDegrees - 180}deg,
                rgb(170, 170, 170) 90deg,
                white 90deg,
                white 180deg,
                red 180deg,
                red 360deg
            )`;
        } else {
            gaugeColor = `conic-gradient(
                rgb(170, 170, 170) 0deg,
                rgb(170, 170, 170) 90deg,
                white 90deg,
                white 180deg,
                red 180deg,
                red ${currentDegrees + 180}deg,
                rgb(170, 170, 170) ${currentDegrees + 180}deg,
                rgb(170, 170, 170) 360deg
            )`;
        }

        // Apply the updated background gradient
        document.getElementById('gauge').style.background = gaugeColor;

        // Save the current degrees as a custom attribute for the next animation
        document.getElementById('gauge').setAttribute('data-current-degrees', currentDegrees);
    }

    // Start the animation
    animateGauge();

    // Update the RPM text
    document.getElementById('rpm-value').textContent = rpm;
}

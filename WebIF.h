String getHTML() {
String html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Underwater Vehicle Technology</title>
</head>
<body>
    <H1>Underwater Vehicle Technology</H1>
    
    <H2>IMU Status</H2>
    <strong>Heading: </strong><span id="hdg">0.00</span>
    <strong>  Pitch: </strong><span id="pitch">0.00</span>
    <strong>  Calibration: </strong><span id="cal">0</span>
    
    <H2>Controller Status</H2>
    <H3>Test Status</H3>
    <strong>Running: </strong><span id="running">false</span>
    <strong>  Elapsed: </strong><span id="elapsed">false</span>
    
    <H3>Control Surface</H3>
    <strong>Rudder: </strong><span id="rud">0.00</span>
    <strong>  Elevator: </strong><span id="elev">0.00</span>
    <strong>  Thruster: </strong><span id="thrust">0.00</span>
    
    <H2>Test Parameters</H2>
    <form id="testform">
        <H3>Pitch PID</H3>
        <label for="pitch_enable">Enable</label><input type="checkbox" id="pitch_enable" name="pitch_enable" value="pitch_enable">
        <label for="KP">KP:</label><input type="number" id="pitch_kp" name="pitch_kp" min="0.0" max="5.0" step="0.1" value="0">
        <label for="KI">KI:</label><input type="number" id="pitch_ki" name="pitch_ki" min="0.0" max="5.0" step="0.1" value="0">
        <label for="KD">KD:</label><input type="number" id="pitch_kd" name="pitch_kd" min="0.0" max="5.0" step="0.1" value="0">
        <label for="KP">Setpoint:</label><input type="number" id="pitch_sp" name="pitch_sp" min="-10" max="10.0" step="0.1" value="0">

        <H3>Heading PID</H3>
        <label for="heading_enable">Enable</label><input type="checkbox" id="heading_enable" name="heading_enable" value="headign_enable">
        <label for="KP">KP:</label><input type="number" id="heading_kp" name="heading_kp" min="0.0" max="5.0" step="0.1" value="0">
        <label for="KI">KI:</label><input type="number" id="heading_ki" name="heading_ki" min="0.0" max="5.0" step="0.1" value="0">
        <label for="KD">KD:</label><input type="number" id="heading_kd" name="heading_kd" min="0.0" max="5.0" step="0.1" value="0">
        
        <H3>Thruster</H3>
        <label for="thrust_start">Start Time: </label><input type="number" id="thrust_start" name="thrust_start" min="0" max="60" step="1" value="0">
        <label for="thrust_stop">Stop Time: </label><input type="number" id="thrust_stop" name="thrust_stop" min="0" max="20" step="1" value="0">

        <H3>Rudder</H3>
        <label for="rudder_init">Initial: </label><input type="number" id="rudder_init" name="rudder_init" min="-100" max="100" step="1" value="0">
        <label for="rudder_change">Change to: </label><input type="number" id="rudder_change" name="rudder_change" min="-100" max="100" step="1" value="0">
        <label for="rudder_time">At Time: </label><input type="number" id="rudder_time" name="rudder_time" min="0" max="20" step="1" value="0">

        <H3>Elevator</H3>
        <label for="elevator_init">Initial: </label><input type="number" id="elevator_init" name="elevator_init" min="-100" max="100" step="1" value="0">

        <p><input type="submit" value="Send It!"></p>
    </form>

    <script>
        async function updateIMU() {
            try {
                const response = await fetch('/state');       
                const data = await response.json();
                document.getElementById("hdg").innerHTML = data.hdg;
                document.getElementById("pitch").innerHTML = data.pitch;
                document.getElementById("cal").innerHTML = data.cal;
                document.getElementById("rud").innerHTML = data.rud;
                document.getElementById("elev").innerHTML = data.elev;
                document.getElementById("running").innerHTML = data.run;
                document.getElementById("thrust").innerHTML = data.thrust;
                document.getElementById("elapsed").innerHTML = data.elapsed;
            } catch (error) {
                console.error('Failed to fetch IMU:', error);
            }
        }
        updateIMU();
        setInterval(updateIMU, 100);

        const test_form = document.getElementById("testform");
        test_form.addEventListener("submit", async (event) => {
            event.preventDefault();
            try {
                const response = await fetch('/test', {
                    method: 'PUT',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({
                        pitch_en: document.getElementById("pitch_enable").checked,
                        pitch_sp: document.getElementById("pitch_sp").value,
                        pitch_kp: document.getElementById("pitch_kp").value,
                        pitch_ki: document.getElementById("pitch_ki").value,
                        pitch_kd: document.getElementById("pitch_kd").value,
                        heading_en: document.getElementById("heading_enable").checked,
                        heading_kp: document.getElementById("heading_kp").value,
                        heading_ki: document.getElementById("heading_ki").value,
                        heading_kd: document.getElementById("heading_kd").value,
                        thrust_start: document.getElementById("thrust_start").value,
                        thrust_stop: document.getElementById("thrust_stop").value,
                        rudder_init: document.getElementById("rudder_init").value,
                        rudder_change: document.getElementById("rudder_change").value,
                        rudder_time: document.getElementById("rudder_time").value,
                        elevator_init: document.getElementById("elevator_init").value
                    })
                });
                const data = await response.json();                
            } catch (error) {
                console.log(error);
                console.error('Failed to update Pitch PID');
            }
        });
    </script>
</body>
</html>
)";
return(html);
}

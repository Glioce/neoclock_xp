// HTML de la página (con marcadores para valores dinámicos)
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>NeoClock</title>
  <style>
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; max-width: 500px; margin: 0 auto; padding: 20px; background-color: #f5f5f5; color: #333; }
    h1 { text-align: center; color: #2c3e50; margin-bottom: 30px; }
    .alarm-container { background-color: white; border-radius: 10px; padding: 20px; box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1); }
    .time-input { display: flex; justify-content: center; gap: 10px; margin-bottom: 20px; }
    .time-input input { width: 60px; padding: 10px; font-size: 18px; text-align: center; border: 1px solid #ddd; border-radius: 5px; }
    .days-container { display: flex; justify-content: space-between; margin-bottom: 20px; }
    .day-checkbox { display: flex; flex-direction: column; align-items: center; }
    .day-checkbox label { margin-top: 5px; font-size: 14px; }
    .toggle-container { display: flex; justify-content: center; align-items: center; gap: 10px; }
    .toggle-switch { position: relative; display: inline-block; width: 60px; height: 34px; }
    .toggle-switch input { opacity: 0; width: 0; height: 0; }
    .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; transition: .4s; border-radius: 34px; }
    .slider:before { position: absolute; content: ""; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; transition: .4s; border-radius: 50%; }
    input:checked + .slider { background-color: #2196F3; }
    input:checked + .slider:before { transform: translateX(26px); }
    .submit-btn { display: block; width: 100%; padding: 12px; background-color: #2c3e50; color: white; border: none; border-radius: 5px; font-size: 16px; cursor: pointer; margin-top: 20px; }
    .submit-btn:hover { background-color: #1a252f; }
    .ip-footer { text-align: center; margin-top: 20px; font-size: 14px; color: #777; }
  </style>
</head>
<body>
  <div class="alarm-container">
    <h1>NeoClock</h1>
    <form action="/set-alarm" method="POST">
      <div class="time-input">
        <input type="number" id="hours" name="hours" min="0" max="23" placeholder="HH" value="%HOURS%" required>
        <span style="font-size: 18px; align-self: center;">:</span>
        <input type="number" id="minutes" name="minutes" min="0" max="59" placeholder="MM" value="%MINUTES%" required>
      </div>
      <div class="days-container">
        <div class="day-checkbox">
          <input type="checkbox" id="monday" name="days" value="1" %MONDAY%>
          <label for="monday">Lun</label>
        </div>
        <div class="day-checkbox">
          <input type="checkbox" id="tuesday" name="days" value="2" %TUESDAY%>
          <label for="tuesday">Mar</label>
        </div>
        <div class="day-checkbox">
          <input type="checkbox" id="wednesday" name="days" value="3" %WEDNESDAY%>
          <label for="wednesday">Mie</label>
        </div>
        <div class="day-checkbox">
          <input type="checkbox" id="thursday" name="days" value="4" %THURSDAY%>
          <label for="thursday">Jue</label>
        </div>
        <div class="day-checkbox">
          <input type="checkbox" id="friday" name="days" value="5" %FRIDAY%>
          <label for="friday">Vie</label>
        </div>
        <div class="day-checkbox">
          <input type="checkbox" id="saturday" name="days" value="6" %SATURDAY%>
          <label for="saturday">Sab</label>
        </div>
        <div class="day-checkbox">
          <input type="checkbox" id="sunday" name="days" value="0" %SUNDAY%>
          <label for="sunday">Dom</label>
        </div>
      </div>
      <div class="toggle-container">
        <span>Alarma activa:</span>
        <label class="toggle-switch">
          <input type="checkbox" id="alarm-active" name="alarm-active" %ACTIVE%>
          <span class="slider"></span>
        </label>
      </div>
      <button type="submit" class="submit-btn">Guardar Alarma</button>
    </form>
    <div class="ip-footer">
      IP: %IP%
    </div>
  </div>
</body>
</html>
)rawliteral";

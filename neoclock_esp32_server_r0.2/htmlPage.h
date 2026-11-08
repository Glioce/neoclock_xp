// HTML de la página web
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Control ESP32-C3</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; }
    .toggle { position: relative; display: inline-block; width: 120px; height: 68px; }
    .toggle input { display: none; }
    .slider { position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px; transition: .4s; }
    .slider:before { position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: white; border-radius: 50%; transition: .4s; }
    input:checked + .slider { background-color: #2196F3; }
    input:checked + .slider:before { transform: translateX(52px); }
  </style>
</head>
<body>
  <h1>Control LED</h1>
  <form action="/toggle">
    <label class="toggle">
      <input type="checkbox" onchange="this.form.submit()" %CHECKED%>
      <span class="slider"></span>
    </label>
  </form>
</body>
</html>
)rawliteral";

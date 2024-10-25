#include <Arduino.h>

const char* errorPlaceHolder = "%error-msg%";
const char* errorCodePlaceHolder = "%error-code%";
const char index_html[] PROGMEM = R"(

<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>%error-msg%</title>
  <style>
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      font-family: Arial, sans-serif;
    }

    body {
      display: flex;
      align-items: center;
      justify-content: center;
      min-height: 100vh;
      background-color: #f7f7f7;
      color: #333;
    }

    .container {
      text-align: center;
      padding: 20px;
      max-width: 500px;
      border: 1px solid #ddd;
      border-radius: 8px;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
    }

    .error-code {
      font-size: 6em;
      font-weight: bold;
      color: #e63946;
    }

    .error-message {
      font-size: 1.2em;
      margin-top: 10px;
      color: #666;
    }

    .home-link {
      display: inline-block;
      margin-top: 20px;
      padding: 10px 20px;
      color: #fff;
      background-color: #457b9d;
      border-radius: 5px;
      text-decoration: none;
      font-size: 1em;
      transition: background-color 0.3s;
    }

    .home-link:hover {
      background-color: #1d3557;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="error-code">%error-code%</div>
    <p class="error-message">%error-msg%</p>
    <a href="/" class="home-link">Go Home</a>
  </div>
</body>
</html>


)";
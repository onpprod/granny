const char *WebPage = R"(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0" >
    <title>GrannyWatch</title>
    <link href="css/style.css" rel= "stylesheet" />     
  </head>
  <body>
    <div id="wifi-container">
      <h1>Conexão GrannyWatch - ESP32</h1>
      <form action="" method="POST">
        <label for="ssid">Rede WiFi</label>
        <input type="text" name="ssid" placeholder="Digite seu SSID">
        <label for="senha">Senha da rede WiFi</label>
        <input type="password" name="password" placeholder="Senha da rede">
        <input type="submit" class="button" value="Enviar">
      </form>

 </div>
</body>
<style>
    *{
      margin: 0;
      padding: 0;
      box-sizing: border-box;
      font-family:Georgia, 'Times New Roman', Times, serif;
      color: black;
      border: none;
  }body{
      background-color: rgb(234, 2241, 249);
      background-size: cover;
  }textarea:focus , input:focus{
      outline: none;
  } a, label{
      font-size: .8rem;
  }
 #wifi-container{
      background-color: #fff ;
      width: 400px;
      margin-left: auto;
      margin-right: auto;
      padding: 20px 30px;
      margin-top: 20vh;
      border-radius: 10px;
      text-align: center;
  }
form{
      margin-top: 30px;
      margin-bottom: 40px;
  }
label, input{
      display: block;
      width: 100%;
      text-align: left;
  }
 label{
      font-weight: bold;
  }
input{
      border-bottom: 2px solid black;
      padding: 10px;
      font-size: 1rem;
      margin-bottom: 20px;
  }
    input:focus{
      border-bottom: 2px solid #08558b;
  }

  input[type="submit"]{
      text-align: center;
      text-transform: uppercase;
      font-weight: bold;
      border: none;
      height: 40px;
      border-radius: 20px;
      margin-top: 30px;
      color: white;
      background-color: rgb(111, 175, 171);
  }
</style> 
</html>
)";

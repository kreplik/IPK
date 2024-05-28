import socket

def receive(message, client):

    parts = message.split()

    response1 = parse_tcp(parts)
    client.sendall(response1.encode())

    print("Response sent to the client")


def parse_tcp(parts):
    if parts[0] == "AUTH":
        if len(parts) > 0:
            if parts[2] == "AS" and parts[4] == "USING":
                response1 = "REPLY OK IS Authentication successful.\r\n"
            else:
                response1 = "REPLY NOK IS Authentication failed.\r\n"

        if len(parts) > 6:
            response1 = "REPLY NOK IS Authentication failed.\r\n"
        return response1
    
    if parts[0] == "JOIN":
    
        if len(parts) > 0:
            if parts[2] == "AS":
                response1 = "REPLY OK IS Join successful.\r\n"
            else:
                response1 = "REPLY NOK IS Join failed.\r\n"
        if len(parts) > 4:
            response1 = "REPLY NOK IS Join failed.\r\n"
        return response1
    
    if parts[0] == "MSG":
     
        if len(parts) > 0:
            if parts[1] == "FROM" and parts[3] == "IS":
                response1 = f"MSG FROM Server IS Message {parts[4]} parsed.\r\n"
            else:
                response1 = "REPLY NOK IS Massage cannot be parsed.\r\n"
       
        return response1

def start_tcp_server(host, port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:

        server_socket.bind((host, port))

        server_socket.listen(1)
        print(f"Server is listening on {host}:{port}...")

        while True:
            client_socket, client_address = server_socket.accept()
            print(f"Connection from {client_address}")

            with client_socket:
    
                while True:
                    try:
                        data = client_socket.recv(1500)
                        if not data:
                            break
                        message = " "
                        message = data.decode()
                        print(f"Received data from {client_address}: {message}")

                        receive(message, client_socket)
                        
                    except OSError as e:
                        print(f"Error receiving data from {client_address}: {e}")
                        break 

if __name__ == "__main__":
    HOST = '0.0.0.0'
    PORT = 4567

    try:
        start_tcp_server(HOST, PORT)
    except InterruptedError:
        print()

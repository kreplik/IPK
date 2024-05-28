import subprocess

def start_test_server():
   
    server_process = subprocess.Popen(['python3', 'tester.py'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    return server_process

def run_client(input_data,mode):

    if mode == 1:
        client_process = subprocess.Popen(['./ipk24chat-client', '-t', 'tcp', '-s', '127.0.0.1'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    elif mode == 2:
        client_process = subprocess.Popen(['./ipk24chat-client', '-t', 'udp', '-s', '127.0.0.1'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    client_process.stdin.write(input_data)

    client_output, client_stderr = client_process.communicate()
    return client_output,client_stderr

def main():

    passed = 0
    test_cases = 0
    print("------------TCP-------------")
    mode = 1 # 1 == TCP
    server_process = start_test_server()

    success_replies = ["/auth xniesl00 123456 user1\n","/auth xniesl00 123 user1\n/join channel2\n"]
    print("------Success tests---------")

    # Run the client program
    for i, test in enumerate(success_replies):
        test_cases += 1
        client_output,client_stderr = run_client(test,mode)
        output = client_stderr.split("\n")

        try:
            assert "Success:" in output[0], "Expected 'Success:' not found in output"
            if len(output) > 2:
                assert "Success:" in output[1], "Expected 'Success:' not found in output"
            print(f"Test {i+1}: Passed.")
            passed += 1
        except AssertionError as e:
            print(f"Test {i+1}: Failed.")

    print("----------------------------")
    print("---------ERR tests----------")
    err_replies = ["/auth xniesl00 123456\n","/join channel2\n","/unknown\n"]

    for i, test in enumerate(err_replies):
        test_cases += 1
        client_output,client_stderr = run_client(test,mode)
    
        try:
            assert "ERR:" in client_stderr, "Expected 'ERR:' not found in client_stderr"
            print(f"Test {i+1}: Passed.")
            passed += 1
        except AssertionError:
            print(f"Test {i+1}: Failed.")
            print()

    print("----------------------------")

    print("---------MSG tests----------")
    msg_replies = ["/auth xniesl00 123456 user1\ntest message","/auth xniesl00 12345 user1\ntest message2"]

    for i, test in enumerate(msg_replies):
        test_cases += 1
        client_output,client_stderr = run_client(test,mode)    
        output = client_stderr.split("\n")

        try:
            assert "Success:" in output[0], "Expected 'Success:' not found in output"
            if len(output) > 2:
                assert "Server:" in client_output, "Expected 'Server:' not found in output"
            print(f"Test {i+1}: Passed.")
            passed += 1
        except AssertionError as e:
            print(f"Test {i+1}: Failed.")
            
    print("----------------------------")

    print("----------------------------")

    print("--------REGEX tests---------")
    msg_replies = ["/auth xniesl00$ 1233 user1\n","/auth xniesl00@ 12345 user1\n","/auth xniesl00 12345 use§r1\n"]

    for i, test in enumerate(msg_replies):
        test_cases += 1
        client_output,client_stderr = run_client(test,mode)    

        try:
            assert "ERR:" in client_stderr, "Expected 'ERR:' not found in output"
            print(f"Test {i+1}: Passed.")
            passed += 1
        except AssertionError as e:
            print(f"Test {i+1}: Failed.")
            
    print("----------------------------")
    print(f"Passed {passed}/{test_cases} in TCP")
    print("----------------------------")
    

    server_process.kill()

    print("------------UDP-------------")
    mode = 2 # 2 == UDP
    test_cases = 0
    passed = 0

    print("---------ERR tests----------")
    err_replies = ["/auth xniesl00 123456\n","/join channel2\n","/unknown\n"]

    for i, test in enumerate(err_replies):
        test_cases += 1
        client_output,client_stderr = run_client(test,mode)
    
        try:
            assert "ERR:" in client_stderr, "Expected 'ERR:' not found in client_stderr"
            print(f"Test {i+1}: Passed.")
            passed += 1
        except AssertionError:
            print(f"Test {i+1}: Failed.")
            print()

    print("----------------------------")

    print("--------REGEX tests---------")
    msg_replies = ["/auth xniesl00$ 1233 user1\n","/auth xniesl00@ 12345 user1\n","/auth xniesl00 12345 use§r1\n"]

    for i, test in enumerate(msg_replies):
        test_cases += 1
        client_output,client_stderr = run_client(test,mode)    

        try:
            assert "ERR:" in client_stderr, "Expected 'ERR:' not found in output"
            print(f"Test {i+1}: Passed.")
            passed += 1
        except AssertionError as e:
            print(f"Test {i+1}: Failed.")

    print("----------------------------")
    print(f"Passed {passed}/{test_cases} in UDP")

if __name__ == "__main__":
    main()

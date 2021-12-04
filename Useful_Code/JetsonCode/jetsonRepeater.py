import socket
import queue
import os
import time
import threading
from threading import Thread
import numpy as np

timeout = 0.01

#global
clients = []


#threading function for data transfer
def externalCommunicationClient(inQueue, outQueue):
    file = os.path.expanduser("~/Desktop/ip_address.txt")
    with open(file) as f:
        file_contents = f.readlines()
    file_contents = [line.strip() for line in file_contents]
    host = file_contents[0]
    port = int(file_contents[1])
    print("Host: ", host)
    print("Port: ", str(port))
    connected = False
    data = ""
    while True:
        if not connected:
            try:
                client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                client_socket.connect((host, port))
                client_socket.settimeout(timeout)
                connected = True
                print("Connected to Server! Waiting commands")
            except:
#                print("FAILED. Sleep briefly & try again in 1 second")
#                time.sleep(1)
                continue
        else:
            # send any messages out to connections
            if not outQueue.empty():
                print("sending message")
                out_data = outQueue.get()
                client_socket.sendall(out_data.encode())
                print('External client sent message: ' + str(out_data))

            try:
                #check if any connections have messages incoming
                data = client_socket.recv(4096) # Kirk set to 4096 Feb 27
            except socket.timeout:
                continue
            except:
                print("Lost communications")
                connected = False
                continue
            if len(data) == 0:
                print("Lost communications")
                connected = False
            else:
                print('External Client Got message: ' + str(data))
                print("Connected: " + str(connected))
                inQueue.put(data.decode())

#local machine communications
def localServerSocketManager(lock):
    global clients
    s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    try:
        os.remove("/tmp/jetsonRepeater")
    except OSError:
        pass
    s.bind("/tmp/jetsonRepeater")
    s.settimeout(timeout)
    s.listen(5)
    on = True
    print('Waiting for new client')
    while on:
        try:
            socket_client, addr = s.accept()
        except socket.timeout:
            continue
        except:
            continue
        lock.acquire()
        print('New client connected')
        clients.append(socket_client)
        print('Waiting for new client')
        lock.release()

def localRepeaterServerCommunicationsManager(inQueue, outQueue, lock):
    on = True
    global clients
    while on:
        removeList = []
        #Check if there are any incoming messages to be broadcasted
        if not inQueue.empty():
            #print("here")
            lock.acquire()
            data = inQueue.get()
        #    lock.release()
            print("Clients:" + str(len(clients)))
            for client in clients:
                try:
                    client.sendall(bytes(str.encode(data)))
                    print("sending data to client")
                except Exception as e:
                    print("type error: " + str(e))
                    if str(e) == "[Errno 32] Broken pipe":
                        print("Removing Broken Pipe")
                        removeList.append(client)
            lock.release()
        #Check if there are any outcoming messages to be sent to external connections
        lock.acquire()

        for client in clients:
            client.settimeout(timeout)
            try:
                data = client.recv(1024) # Kirk changed from 512 to 1024 on May 5 2021
            except socket.timeout:
                continue
            except:
                print(" Exception Lost communications")
                continue
            if len(data) == 0:
                print("Length data is 0")
                removeList.append(client)
            else:
                msgData = np.array(data.decode().splitlines())
                if msgData[0] == "SPOTNet":
                #print('Repeater got message: ' + str(data))
                #outQueue.put(data.decode())
                    print('Repeater rebroadcasting message')
                    inQueue.put(data.decode())
                else:
                    print('Repeater got message: ' + str(data))
                    outQueue.put(data.decode())

        if not len(removeList) == 0:
            clients = [x for x in clients if x not in removeList]
            print("Clients:" + str(len(clients)))

        lock.release()


def Main():
    inQueue = queue.Queue()
    outQueue = queue.Queue()
    lock = threading.Lock()

    commThread = Thread(target=externalCommunicationClient, args=(inQueue, outQueue))
    localSocketManagerThread = Thread(target=localServerSocketManager, args=(lock,))
    localRepeaterCommThread = Thread(target=localRepeaterServerCommunicationsManager, args=(inQueue, outQueue, lock))

    commThread.start()
    localSocketManagerThread.start()
    localRepeaterCommThread.start()

    commThread.join()
    localSocketManagerThread.join()
    localRepeaterCommThread.join()

if __name__ == "__main__":
    Main()

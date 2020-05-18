import logging
import threading
import time
import random
import configparser

call_counter = 0
next_call = 0
allCalls = []
allWorkers = []
access_lock = threading.Lock()
stop_threads = 0

class call:
    def __init__ (self, callid, status):
        self.callid = callid
        self.status = status

class worker:
    def __init__ (self, status, calls_taken, efficiency):
        self.status
        self.calls_taken
        self.efficiency

def createCall(avgCalls, stdDev):
    global call_counter
    newCalls = avgCalls + random.randint(-stdDev, stdDev)
    for i in range (0, newCalls):
        c = call(call_counter, "on hold")
        allCalls.append(c)
        call_counter += 1

def processCall():
    global stop_threads
    while (stop_threads == 0):
        global access_lock
        # Critical selection of call
        access_lock.acquire()
        global next_call
        current_call = next_call
        next_call += 1
        access_lock.release()
        # If the call doesn't exits: wait
        while (len(allCalls)+1<current_call):
            #worker status: inactive
            print("waiting for call")
            time.sleep(1)
        print ("working on call: " + str(current_call) + "\n")
        allCalls[current_call].status = "processing"
        time.sleep(1)
        allCalls[current_call].status = "completed"


def print_calls ():
    for i in range (0, len(allCalls)):
        print("call " + str(allCalls[i].callid) + ", " + str(allCalls[i].status))
    
def main():
    # - - - - - - - - - - -  SETUP  - - - - - - - - - - - 
    #initiate logger
    print ("program started")
    random.seed()
    wthreads = []

    # input number of workers
    workers = int(input("Enter the number of workers: "))

    # read config file for time statistics
    config = configparser.ConfigParser()
    config.read("config.ini")
    avgCalls = int(config["Statistics"]["avgCalls"])
    stdCalls = int(config["Statistics"]["stdCalls"])
    avgCallTime = int(config["Statistics"]["avgCallTime"])
    contingency = int(config["Other Values"]["contingency"])

    if (avgCalls<stdCalls):
        print ("warning: there will be no calls in some cycles")
        

    # - - - - - - - - - - -  EXECUTION  - - - - - - - - - - -

    # Create a thread for each worker
    for i in range (0, workers):
        worker = threading.Thread(target=processCall)
        wthreads.append(worker)
        wthreads[i].start()

    # create calls in a cycle
    for i in range (0,12):
       createCall(avgCalls, stdCalls)
       time.sleep(1)

    # Work day is over
    global stop_threads
    stop_threads = 1
    
    # Join threads
    for i in range (0, workers):
        wthreads[i].join()

    print_calls()

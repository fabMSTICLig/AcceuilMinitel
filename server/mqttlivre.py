from paho.mqtt import client as MqttClient
import certifi
import json

class MinitelCommand:
    LORA_MSG_MAXSIZE = 50
    CMD_HEAD_SIZE = 3
    date = None
    user = ""
    messages = []
    xor = 0
    length = 0


ACTUS_BASE_FILE="actusbase.json"
ACTUS_PUBLISH_FILE="/path/to/actus.json"
ACTUS_TOPIC="TOPIC"
miniCmd = MinitelCommand()

def parseActuCommand(obj):
    print(obj)
    if(obj['subCmd']==1):
        print("sub 1")
        miniCmd.date = obj['date']
        miniCmd.xor = obj['xor']
        miniCmd.length = obj['len']
        miniCmd.user=obj['user']
        print(int((miniCmd.length+MinitelCommand.CMD_HEAD_SIZE)/MinitelCommand.LORA_MSG_MAXSIZE+1))
        miniCmd.messages = [None] * int((miniCmd.length+MinitelCommand.CMD_HEAD_SIZE)/MinitelCommand.LORA_MSG_MAXSIZE+1)
        miniCmd.messages[0]=obj['message']
        return (miniCmd.length+MinitelCommand.CMD_HEAD_SIZE)<MinitelCommand.LORA_MSG_MAXSIZE
    elif(obj['subCmd']==2):
        if(obj['pos']<len(miniCmd.messages)):
            miniCmd.messages[obj['pos']-1]=obj['message']
            return False
        else:
            raise IndexError
    elif(obj['subCmd']==3):
        miniCmd.messages[-1]=obj['message']
        return True
    else:
        raise ValueError

def publishActu():
    print("publishActu")
    xort=int(miniCmd.length)
    for code in miniCmd.user.encode('ascii'):
        if(xort is None):
            xort=code
        else:
            xort=xort^code
    message=""
    for msg in miniCmd.messages:
        if(msg is None):
            print("One part is missing")
            return False
        else:
            message=message+msg
    print(message)
    for code in message.encode('ascii'):
        xort=xort^code
    xort=xort^0
    if(miniCmd.xor != xort):
        print("Xor not equals "+str(miniCmd.xor)+" "+str(xort))
        return False

    lastactus=[]
    with open(ACTUS_BASE_FILE,'r+') as file:
        file_data = json.load(file)
        file_data["actus"].insert(0,{'user':miniCmd.user,'message':message, 'date':miniCmd.date})
        lastactus = file_data["actus"][:10]
        file.seek(0)
        json.dump(file_data, file, indent = 4)
    with open(ACTUS_PUBLISH_FILE,"w") as file:
        json.dump({'actus':lastactus}, file)

       


def on_connect(mqtt_client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
        mqtt_client.on_message = on_message
        mqtt_client.subscribe(ACTUS_TOPIC)
    else:
        print("Failed to connect, return code %d\n", rc)

# Callback called when MQTT messages are received
def on_message(mqtt_client, userdata, msg):
    obj = json.loads(msg.payload.decode())['object']
    if(obj['cmd']==1):
        try:
            if(parseActuCommand(obj)):
                publishActu()
        #except ValueError:
        #    print("Wrong sub command")
        except IndexError:
            print("Message position out of bound")

mqtt_client = MqttClient.Client()
mqtt_client.username_pw_set("user", "pass")
mqtt_client.on_connect = on_connect
mqtt_client.tls_set(certifi.where())
mqtt_client.tls_insecure_set(True)
mqtt_client.connect("127.0.0.1", 8883)
mqtt_client.loop_forever()


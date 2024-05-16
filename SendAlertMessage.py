from firebase import firebase

from twilio.rest import Client

data1="000"
data2="000"

prevdata1="000"
prevdata2="000"

account_sid = "AC19bcacffd13d450737cb04a7a8364f9f"
auth_token = "122f5070752f0197152c64afd1cd655e"

# Your Twilio phone number and the recipient's phone number
twilio_number = "+19149158335"  # Your Twilio phone number
to_number = "+918792373343"     # Recipient's phone number


# Initialize Twilio client
client = Client(account_sid, auth_token)

def SendSMS(message_body):
    print(message_body)
    
    # Send SMS message
    message = client.messages.create(
        body=message_body,
        from_=twilio_number,
        to=to_number
    )

    # Print message SID
    print('Message SID:', message.sid)
    
while True:
    FBConn = firebase.FirebaseApplication('https://woman-safety-project-default-rtdb.firebaseio.com/',None)
    #FBConn.put('Employee Details/Emp001','Status',"Logged OUT")
    
    #FBConn.put('','Fetch',{'ok':1})
    database = FBConn.get('Database',None)
    data1=database['Node1']['MasterSignal']
    data2=database['Node2']['MasterSignal']
    
    if data1!=prevdata1:
        print("SMS Trigger1")
        if data1=="100":
            SendSMS("Alert !!! Node1 - Uncertain HeartRate detected !! Please take neccessary actions , Current Location - "+database['Node1']['Located'])
        if data1=="010":
            SendSMS("Alert !!! Node1 - Emergency Call !! Please take neccessary actions , Current Location - "+database['Node1']['Located'])
        if data1=="001":
            SendSMS("Alert !!! Node1 - Call for Help !! Please take neccessary actions , Current Location - "+database['Node1']['Located'])
    if data2!=prevdata2:
        print("SMS Trigger2")
        if data2=="100":
            SendSMS("Alert !!! Node1 - Uncertain HeartRate detected !! Please take neccessary actions , Current Location - "+database['Node1']['Located'])
        if data2=="010":
            print("Node2 - Emergency")
            SendSMS("Alert !!! Node1 - Emergency Call !! Please take neccessary actions , Current Location - "+database['Node1']['Located'])
        if data2=="001":
            SendSMS("Alert !!! Node1 - Call for Help !! Please take neccessary actions , Current Location - "+database['Node1']['Located'])
    
    prevdata1=data1
    prevdata2=data2
    print(data1,data2)
#FBConn.delete("/Cards",None)
#print(database)
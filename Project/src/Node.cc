//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "Node.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <bitset>

using namespace std;
Define_Module(Node);

ifstream inputFile;
ofstream outputFile;

void Node::IntroduceError(MyMessage_Base* Newmsg, string Error)
{
    //////////////// Introduce Errors /////////////////////////
    double delay = TD;
    int modified = -1;
    string lost = "No";
    int version = 0;
    double delayout = 0;

    if (Error[2] =='1')
    {
        version = 1;
    }
    if (!isfirstmsg) //correct?
    {
        //delay+=PT;
    }

    if (Error[0] == '1') // Modification
    {
        int addValue = uniform(1, 25);
        string Payload = Newmsg->getPayload();
        Payload[1] = Payload[1] + addValue;
        Newmsg->setPayload(Payload.c_str());

        modified = 1;
    }

    if (Error[1] == '1') //Loss
    {
        lost = "Yes";
        EV << "Message lost with sequence number " << Newmsg->getHeader() << endl;
        outputFile << "At time " << simTime() << ", Node" << nodeID << " sent frame with seq_num= " << Newmsg->getHeader() << " and payload= " << Newmsg->getPayload() << " and trailer= " << Newmsg->getTrailer() << " , Modified " << modified << " ,Lost " << lost << ", Duplicate " << version << ", Delay " << delayout << endl;
        return;
    }

    if (Error[3] == '1') //Delay
    {
        delay += ED;
        delayout = ED;
    }

    outputFile << "At time " << simTime() << ", Node" << nodeID << " sent frame with seq_num= " << Newmsg->getHeader() << " and payload= " << Newmsg->getPayload() << " and trailer= " << Newmsg->getTrailer() << " , Modified " << modified << " ,Lost " << lost << ", Duplicate " << version << ", Delay " << delayout << endl;

    ///////////////// Send Message  ////////////////////////////


    EV << "Sent Message: " << Newmsg->getHeader() << " " << Newmsg->getPayload() << " " << Newmsg->getAck_num() << " " << Newmsg->getTrailer() << " " << Newmsg->getFrame_Type() << " at time = " << simTime() << endl;

    sendDelayed(Newmsg,delay,"out");

    if (Error[2] == '1') //Duplication
    {
        delay += DD;
        MyMessage_Base* Dup = new MyMessage_Base("");
        Dup->setHeader(Newmsg->getHeader());
        Dup->setPayload(Newmsg->getPayload());
        Dup->setTrailer(Newmsg->getTrailer());
        Dup->setFrame_Type(Newmsg->getFrame_Type());
        Dup->setAck_num(Newmsg->getAck_num());
        sendDelayed(Dup,delay,"out");
        version = 2;
        EV << "Sent Duplicate message with sequence number " << Dup->getHeader() << endl;

        outputFile << "At time " << simTime() + DD << ", Node" << nodeID << " sent frame with seq_num= " << Dup->getHeader() << " and payload= " << Dup->getPayload() << " and trailer= " << Dup->getTrailer() << " , Modified " << modified << " ,Lost " << lost << ", Duplicate " << version << ", Delay " << delayout << endl;

    }

}


string ByteStuffing(const string& input) {
    string result = "$"; // Start with the dollar sign

    for (char c : input) {
        // Add a '/' before '$' or '/' characters
        if (c == '$' || c == '/') {
            result += '/';
        }
        result += c; // Add the current character
    }

    result += "$"; // Add a dollar sign at the end
    return result;
}

string Destuffing(const string& modified) {
    string original;


    if (modified.length() >= 2 && modified.front() == '$' && modified.back() == '$') {
        for (size_t i = 1; i < modified.length() - 1; ++i) { // Skip first and last '$'
            if (modified[i] == '/' && (modified[i + 1] == '$' || modified[i + 1] == '/')) {
                // Skip the '/' and directly add the next character
                ++i;
            }
            original += modified[i];
        }
    } else {
        cout << "Invalid input format! The string must start and end with '$'.\n";
    }

    return original;
}


void Node::resendWindow(int seqnum, string message, string Error)
{
    MyMessage_Base * Newmsg = new MyMessage_Base("Sending Message");

    //outputFile << "At time " << simTime() - PT << ", Node" << nodeID << " , Introducing channel error with code " << Error << endl;

        /////////////// Byte Stuffing /////////////////////

        string stuffedmsg = ByteStuffing(message);

        /////////////// Calculate Parity /////////////////

       bitset<8>  bset(stuffedmsg[0]);
       bitset<8> parity = bset;

       for(int i = 1; i < stuffedmsg.size(); i++)
       {
           bitset<8>  bset(stuffedmsg[i]);
           parity ^=  bset;
       }

       Newmsg->setHeader(seqnum);
       Newmsg->setPayload(stuffedmsg.c_str());
       Newmsg->setAck_num(0);
       Newmsg->setTrailer(parity.to_string().c_str());
       Newmsg->setFrame_Type(2);


       IntroduceError(Newmsg, Error);
       startTimer(Newmsg->getHeader());
}

void Node::sendMessage(string message, string Error)
{

    if (start <= End)
    {
        outputFile << "At time " << simTime() - PT << ", Node" << nodeID << " , Introducing channel error with code " << Error << endl;

        MyMessage_Base * Newmsg = new MyMessage_Base("Sending Message");

        /////////////// Byte Stuffing /////////////////////

        string stuffedmsg = ByteStuffing(message);

        /////////////// Calculate Parity /////////////////

       bitset<8>  bset(stuffedmsg[0]);
       bitset<8> parity = bset;

       for(int i = 1; i < stuffedmsg.size(); i++)
       {
           bitset<8>  bset(stuffedmsg[i]);
           parity ^=  bset;
       }

       Newmsg->setHeader(count);
       Newmsg->setPayload(stuffedmsg.c_str());
       Newmsg->setAck_num(0);
       Newmsg->setTrailer(parity.to_string().c_str());
       Newmsg->setFrame_Type(2);

       IntroduceError(Newmsg, Error);
       count = (count + 1)%(WS + 1);
       start++;

       startTimer(Newmsg->getHeader());

    }
}

void Node::receiveMessage(MyMessage_Base* mmsg)
{
    ///////// Check expected frame ////////

   if (mmsg->getHeader() == expectedFrame) /// expected frame
   {
       ///////// Check Parity ////////////////

       string msg_received = Destuffing(mmsg->getPayload());
       string stuffedmsg = mmsg->getPayload();
       bitset<8>  bset(stuffedmsg[0]);
       bitset<8> parity = bset;

       for(int i = 1; i < stuffedmsg.size(); i++)
       {
           bitset<8>  bset(stuffedmsg[i]);
           parity ^=  bset;
       }

      bitset<8> parity_rec(mmsg->getTrailer());
      parity ^= parity_rec;

      int parity_check_int  = parity.to_ulong();
      if (parity_check_int)
      {
          EV << "Incorrect Message " << endl;
          string msgname = "Send NAck" + to_string(expectedFrame);
          scheduleAt(simTime() + PT, new cMessage(msgname.c_str()));

      }
      else
      {
          EV << "Correct Message"  << endl;

          string msgname = "Send Ack" + to_string(expectedFrame);
          scheduleAt(simTime() + PT, new cMessage(msgname.c_str()));
          expectedFrame = (expectedFrame + 1)%(WS + 1);

      }

       EV << "Received Message: " << mmsg->getHeader() << " " << msg_received << " " << mmsg->getAck_num() << " " << mmsg->getTrailer() << " " << mmsg->getFrame_Type() << " at time = " << simTime() << endl;
       outputFile << "At time " << simTime() << ", Node" << nodeID << " received frame with seq_num= " << mmsg->getHeader() << " and payload= " << msg_received << " and trailer= " << mmsg->getTrailer() << endl;


   }
   else //unexpected frame
   {
       EV << "Unexpected Frame with seq no " << mmsg->getHeader() << "and payload " << mmsg->getPayload() << endl;
       EV << "Last ack = " << lastAck << endl;;
       if (lastAck < 0)
       {
           return;
       }
       else
       {
           ///////////////// Send Ack ///////////////////
           MyMessage_Base * Ackmsg = new MyMessage_Base("Ack");
           Ackmsg->setAck_num(lastAck);
           Ackmsg->setFrame_Type(1);
           Ackmsg->setHeader(0);
           Ackmsg->setTrailer("");
           Ackmsg->setPayload("");

           sendDelayed(Ackmsg,TD + PT,"out");
           EV << "Sent Ack number " << lastAck << endl;
       }

   }

}

void Node::startTimer(int seqNum)
{
    EV << "Started timer number " << seqNum << endl;

    if (timers.count(seqNum) == 1) {
       stopTimer(seqNum);
   }

   cMessage* timer = new cMessage(to_string(seqNum).c_str());
   scheduleAt(simTime() + TO, timer);
   timers[seqNum] = timer;

}

void Node::stopTimer(int seqNum)
{
    if (timers[seqNum])
    {
        cancelAndDelete(timers[seqNum]);
        timers.erase(seqNum);
    }
}

void Node::stopAllTimers()
{
    for (const auto& pair : timers)
    {
        cancelAndDelete(pair.second);
        timers.erase(pair.first);
    }
}

bool Node::between(int a, int b, int c)
{
    //// ack num between expected ack and end circularly
    if (((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a)))
    {
        /// stop all timers of acknowledged frames
        cout << "Condition successful at time " << simTime() << " ack no " << b << " expected = " << a << " end = " << c << endl;
        int cond = b;
        if (cond < expectedAck)
        {
            cond += WS + 1;
        }

        for (int i = expectedAck; i <= cond; i++)
        {
            stopTimer(i%(WS + 1));
        }
        return true;
    }
    else
    {
        return false;
    }
}


void Node::initialize()
{
    // TODO - Generated method body
    TD = getParentModule()->par("TD").doubleValue();
    PT = getParentModule()->par("PT").doubleValue();
    WS = getParentModule()->par("WS").intValue();
    LP = getParentModule()->par("LP").intValue();
    TO = getParentModule()->par("TO").intValue();
    ED = getParentModule()->par("ED").doubleValue();
    DD = getParentModule()->par("DD").doubleValue();

    End = WS - 1;
    //////////////////////// Read Line ////////////////////////////

    string FileName;
     if (!strcmp(this->getName(), "node0"))
     {
         nodeID = 0;
         FileName="input0.txt";
     }
     else
     {
         nodeID = 1;
         FileName="input1.txt";
     }
     inputFile.open(FileName); // Open the file
     if (!inputFile.is_open()) {
         cout << "Error opening file!" << endl;
     }

     string line;
     string ErrorType; // To hold the first 4 bits
     string message;   // To hold the rest of the line

     // Read the first line of the file
     while (getline(inputFile, line)) {
         // Extract the first 4 characters as ErrorType
         ErrorType = line.substr(0, 4);

         // Extract the rest of the line as message
         message = line.substr(5); // Skip the space after the 4 bits

         Data.push(message);
         Error_Type.push(ErrorType);

         cout << "Input file message : " << message << endl;
         cout << "Input error type :" << ErrorType << endl;
     }

    // Close the file
    inputFile.close();


}

void Node::handleMessage(cMessage *msg)
{
    string msgname = msg->getName();
    if (msg->isSelfMessage() && !strcmp(msg->getName(),"Processing" )) // Received Self Message (processing delay)
    {
        if (Data.empty())
        {
           return;
        }

        if (start > End)
        {
            scheduleAt(simTime() + PT, new cMessage("Processing"));
            return;
        }

        string message = Data.front();
        Data.pop();
        string Error = Error_Type.front();
        Error_Type.pop();

        Frame newframe;
        newframe.message=message;
        newframe.error=Error;
        newframe.seqnum=count;

        buffer.push(newframe);

        sendMessage(message, Error);
        isfirstmsg = 0;
        scheduleAt(simTime() + PT, new cMessage("Processing"));
        return;
    }
    else if (msg->isSelfMessage() && msgname.substr(0,11) == "Process Ack"  )
    {
       string num = msgname.substr(11,1);
       EV << "Received Ack number " << num << endl;
       stopTimer(atoi(num.c_str()));
       End++;
       //lastAck = atoi(num.c_str());

       buffer.pop();
       //expectedAck = (expectedAck + 1)%(WS + 1);
       return;
    }
    else if (msg->isSelfMessage() && msgname.substr(0,14) == "Cumulative Ack"  )
    {
        string num = msgname.substr(14,1);
        EV << "Received Cumulative Ack number " << num << endl;

        int index = atoi(num.c_str());


        while (1)
        {
            if (buffer.empty())
            {
                break;
            }
            if (buffer.front().seqnum == index)
            {
                break;
            }
            End++;
            buffer.pop();
        }

        End++;
        buffer.pop();
        return;

    }
    else if (msg->isSelfMessage() && msgname.substr(0,8) == "Send Ack" )
    {
        ///////////////// Send Ack ///////////////////
        MyMessage_Base * Ackmsg = new MyMessage_Base("Ack");
        string num = msgname.substr(8,1);
        Ackmsg->setAck_num(atoi(num.c_str()));
        Ackmsg->setFrame_Type(1);
        Ackmsg->setHeader(0);
        Ackmsg->setTrailer("");
        Ackmsg->setPayload("");

        string lost = "Yes";
        int prob = int(uniform(0,100));
        cout << "prob = " << prob << "at time = " << simTime() << endl;

        if (prob >= LP)
        {
            sendDelayed(Ackmsg,TD,"out");
            EV << "Sent Ack number " << msgname.substr(8,1) << endl;
            lost = "No";
        }
        else
        {
            EV << "Ack number " << msgname.substr(8,1) << " lost " << endl;
        }

        string Acknum = msgname.substr(8,1);
        lastAck = atoi(Acknum.c_str());

       outputFile << "At time " << simTime() << ", Node" << nodeID << " Sending ACK with number " << msgname.substr(8,1) << ", loss " << lost << endl;

        return;
    }
    else if (msg->isSelfMessage() && msgname.substr(0,9) == "Send NAck" )
    {
        ///////////////// Send NACK ///////////////////
        MyMessage_Base * Ackmsg = new MyMessage_Base("NAck");
        string num = msgname.substr(9,1);
        Ackmsg->setAck_num(atoi(num.c_str()));
        Ackmsg->setFrame_Type(0);
        Ackmsg->setHeader(0);
        Ackmsg->setTrailer("");
        Ackmsg->setPayload("");

        string lost = "Yes";
        int prob = int(uniform(0,100));
        if (prob >= LP)
        {
            sendDelayed(Ackmsg,TD,"out");
            lost = "No";
            EV << "Sent NACK number " << msgname.substr(9,1) << endl;
        }
        else
        {
            EV << "NACK number " << msgname.substr(9,1) << " lost " << endl;
        }

       outputFile << "At time " << simTime() << ", Node" << nodeID << " Sending NACK with number " << msgname.substr(9,1) << ", loss " << lost << endl;


            return;
     }
    else if (msg->isSelfMessage()) // Received Self Message (Timeout)
    {
        if (!strcmp(msg->getName(),"Resending buffer")) // resend with error
        {
            if (!temp.empty())
            {
              Frame newframe = temp.front();
              resendWindow(newframe.seqnum, newframe.message, newframe.error);
              temp.pop();

              scheduleAt(simTime() + PT, new cMessage("Resending buffer"));
              EV << "Sending rest of window with error message number " << newframe.seqnum << endl;
            }
        }
        else if (!strcmp(msg->getName(),"Resending buffer no error"))  // resend without error
        {
            if (!temp.empty())
            {
              Frame newframe = temp.front();
              resendWindow(newframe.seqnum, newframe.message, "0000");
              temp.pop();

              scheduleAt(simTime() + PT, new cMessage("Resending buffer"));
              EV << "Sending rest of window without error message number " << newframe.seqnum << endl;
            }
        }
        else /// Receive timeout number
        {
            EV << "Timeout occurred for Sequence number " << msg->getName() << endl;

            string name = msg->getName();
            int errorseq = atoi(name.c_str());

            if (buffer.empty())
            {
                return;
            }

            if (buffer.front().seqnum == errorseq)
            {
                /////// Stop all timers ////////

                stopAllTimers();

                //////////////////

                temp = buffer;
                if (temp.empty())
                {
                    return;
                }

               Frame newframe = temp.front();
               outputFile << "Time out event at time " << simTime() << ", at Node" << nodeID << " for frame with seq_num= " << newframe.seqnum << endl;
               //resendWindow(newframe.seqnum, newframe.message, "0000");
               //temp.pop();

               EV << "Resending without error message number " << newframe.seqnum << endl;
               scheduleAt(simTime() + PT, new cMessage("Resending buffer no error"));
           }

        }

        return;
    }


    // TODO - Generated method body
    if (!strcmp(msg->getArrivalGate()->getName(),"in_coordinator" )) /// Received start from coordinator
    {
        //sendMessage();
        outputFile.open("output.txt", ios::trunc);
        outputFile.clear();
        scheduleAt(simTime() + PT, new cMessage("Processing"));
        return;
    }

    MyMessage_Base *mmsg = check_and_cast<MyMessage_Base *>(msg);

    if (mmsg->getFrame_Type() == 2) //Received Data at receiver
    {
        receiveMessage(mmsg);
    }
    else if (mmsg->getFrame_Type() == 1)  // Received Ack at sender
    {

       if (mmsg->getAck_num() == expectedAck)
       {
           string msgg = "Process Ack" + to_string(expectedAck);
           scheduleAt(simTime() + PT, new cMessage(msgg.c_str()));

           /*
           EV << "Received Ack number " << mmsg->getAck_num() << endl;
           stopTimer(expectedAck);
           End++;
           lastAck = expectedAck;

           buffer.pop();
           */
           expectedAck = (expectedAck + 1)%(WS + 1);

       }

       else if (between(expectedAck, mmsg->getAck_num(), End%(WS + 1))) /// Cumulative Ack
       {
           string msgg = "Cumulative Ack" + to_string(mmsg->getAck_num());
           scheduleAt(simTime() + PT, new cMessage(msgg.c_str()));
       }

    }
    else if (mmsg->getFrame_Type() == 0)// Receives NACK at sender
    {
        EV << "Received NACk number " << mmsg->getAck_num() << endl;
    }

}

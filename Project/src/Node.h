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

#ifndef __PROJECT_NODE_H_
#define __PROJECT_NODE_H_

#include <omnetpp.h>
#include <queue>
#include "MyMessage_m.h"
#include <unordered_map>

using namespace omnetpp;
using namespace std;

struct Frame
{
    int seqnum;
    string message;
    string error;
};

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
  private:
    int WS;
    int LP;
    int TO;
    double PT;
    double TD;
    double ED;
    double DD;

    int nodeID;
    int startingTime;
    int expectedFrame = 0;
    int count = 0;
    int start = 0;
    int End;
    int lastAck = -1;
    int expectedAck = 0;
    bool isfirstmsg = 1;

    unordered_map<int, cMessage *> timers;
    queue<Frame> buffer;
    queue<Frame> temp;

    queue<string> Data;
    queue<string> Error_Type;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void sendMessage(string message, string Error);
    void receiveMessage(MyMessage_Base* msg);
    void startTimer(int seqNum);
    void stopTimer(int seqNum);
    void stopAllTimers();
    void IntroduceError(MyMessage_Base* msg, string Error);
    void resendWindow(int seqnum, string message, string Error);
    bool between(int a,int b, int c);
};

#endif

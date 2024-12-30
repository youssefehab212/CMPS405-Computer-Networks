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

#include "Coordinator.h"
#include "MyMessage_m.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

Define_Module(Coordinator);

void Coordinator::initialize()
{
    // TODO - Generated method body
    int nodeId;
    int startingTime;

    ifstream inputFile("coordinator.txt");

    // Check if the file was successfully opened
    if (!inputFile.is_open()) {
        cout << "Error: Unable to open file " << endl;
    }

    string line;
    getline(inputFile, line);

    stringstream ss(line);
    string temp;

    getline(ss, temp, '['); // Skip the opening bracket [
    ss >> nodeId >> startingTime;

    inputFile.close();
    MyMessage_Base * msg = new MyMessage_Base("Start");
    if (nodeId == 0)
    {
        sendDelayed(msg,startingTime,"out0");
    }
    else
    {
        sendDelayed(msg,startingTime,"out1");
    }

}

void Coordinator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}

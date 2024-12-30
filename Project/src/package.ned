package project;

import ned.IdealChannel;


@license(LGPL);
//
// TODO documentation
//
network Network
{
    //@display("bgb=392,267");
    
    parameters:
        int WS = default(5);
        int TO = default(10);
        double PT = default(0.5);
        double TD = default(1.0);
        double ED = default(4.0);
        double DD = default(0.1);
        int LP = default(10);
    
    submodules:
        node0: Node {
            @display("p=58,80");
        }
        node1: Node {
            @display("p=307,86");
        }
        coordinator: Coordinator {
            @display("p=183,174");
        }
    connections:
        coordinator.out0 --> IdealChannel --> node0.in_coordinator;
        coordinator.out1 --> IdealChannel --> node1.in_coordinator;
        node0.out --> IdealChannel --> node1.in;
        node1.out --> IdealChannel --> node0.in;
}


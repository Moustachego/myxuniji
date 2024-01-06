package nesting.simulations.DelayJitter.Demo_03;

import ned.DatarateChannel;
import nesting.node.ethernet.VlanEtherHostQ;
import nesting.node.ethernet.VlanEtherHostSched;
import nesting.node.ethernet.VlanEtherSwitchPreemptable;


network DelayJitter
{
    @display("bgb=1215.76,1148.784");
    types:
        channel C extends DatarateChannel
        {
            delay = 0us;
            datarate = 1Gbps;
        }
    submodules:
        SW1: VlanEtherSwitchPreemptable {
            parameters:
                @display("p=371.28,694.512;is=vl");
            gates:
                ethg[3];
        }
        SW2: VlanEtherSwitchPreemptable {
            parameters:
                @display("p=371.28,391.664;is=vl");
            gates:
                ethg[4];
        }
        SW3: VlanEtherSwitchPreemptable {
            parameters:
                @display("p=889.616,663.93604;is=vl");
            gates:
                ethg[3];
        }
        SW4: VlanEtherSwitchPreemptable {
            parameters:
                @display("p=889.616,391.664;is=vl");
            gates:
                ethg[5];
        }
        ES1: VlanEtherHostSched {
            @display("p=371.28,958.04803");
        }
        ES2: VlanEtherHostQ {
            @display("p=144.144,695.968");
        }
        ES3: VlanEtherHostSched {
            @display("p=144.144,393.12");
        }
        ES4: VlanEtherHostSched {
            @display("p=371.28,173.264");
        }
        ES5: VlanEtherHostSched {
            @display("p=889.616,934.752");
        }
        ES6: VlanEtherHostQ {
            @display("p=1138.592,665.392");
        }
        ES7: VlanEtherHostSched {
            @display("p=678.49603,173.264");
        }
        ES8: VlanEtherHostSched {
            @display("p=924.56,173.264");
        }
        Dst: VlanEtherHostQ {
            @display("p=1140.048,393.12");
        }
    connections:
        ES1.ethg <--> C <--> SW1.ethg[1];
        ES2.ethg <--> C <--> SW1.ethg[2];
        ES3.ethg <--> C <--> SW2.ethg[3];
        ES4.ethg <--> C <--> SW2.ethg[0];
        ES5.ethg <--> C <--> SW3.ethg[2];
        ES6.ethg <--> C <--> SW3.ethg[1];
        ES7.ethg <--> C <--> SW4.ethg[4];
        ES8.ethg <--> C <--> SW4.ethg[0];
        Dst.ethg <--> C <--> SW4.ethg[1];
        SW1.ethg[0] <--> C <--> SW2.ethg[2];
        SW2.ethg[1] <--> C <--> SW4.ethg[3];
        SW3.ethg[0] <--> C <--> SW4.ethg[2];
}

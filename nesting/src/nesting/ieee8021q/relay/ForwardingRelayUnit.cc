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

#include "nesting/ieee8021q/relay/ForwardingRelayUnit.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/vlan/VlanTag_m.h"

#include <sstream>

namespace nesting {

Define_Module(ForwardingRelayUnit);

void ForwardingRelayUnit::initialize(int stage) {
    if (stage == INITSTAGE_LOCAL) {
        fdb = getModuleFromPar<FilteringDatabase>(par("filteringDatabaseModule"), this);
        ifTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        numberOfPorts = par("numberOfPorts");
    } else if (stage == INITSTAGE_LINK_LAYER) {
        registerService(Protocol::ethernetMac, nullptr, gate("ifIn"));
        registerProtocol(Protocol::ethernetMac, gate("ifOut"), nullptr);
    }
}

void ForwardingRelayUnit::handleMessage(cMessage *msg) {
    Packet* packet = check_and_cast<Packet*>(msg);
    const auto& frame = packet->peekAtFront<EthernetMacHeader>();
    int arrivalInterfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();

    // Remove old service indications but keep packet protocol tag and add VLAN request
    auto oldPacketProtocolTag = packet->removeTag<PacketProtocolTag>();
    auto vlanInd = packet->removeTag<VlanInd>();
    packet->clearTags();
    auto newPacketProtocolTag = packet->addTag<PacketProtocolTag>();
    *newPacketProtocolTag = *oldPacketProtocolTag;
    auto vlanReq = packet->addTag<VlanReq>();
    vlanReq->setVlanId(vlanInd->getVlanId());
    delete oldPacketProtocolTag;

    packet->trim();

    // Distinguish between broadcast-, multicast- and unicast-ethernet frames
    if (frame->getDest().isBroadcast()) {
        processBroadcast(packet, arrivalInterfaceId);
    } else if (frame->getDest().isMulticast()) {
        processMulticast(packet, arrivalInterfaceId);
    } else {
        processUnicast(packet, arrivalInterfaceId);
    }
}

void ForwardingRelayUnit::processBroadcast(Packet* packet, int arrivalInterfaceId) {
    EV_INFO << "Broadcasting packet " << packet->getName() << std::endl;
    for (int interfacePos = 0; interfacePos < ifTable->getNumInterfaces(); interfacePos++) {
        InterfaceEntry* destInterface = ifTable->getInterface(interfacePos);
        int destInterfaceId = destInterface->getInterfaceId();
        if (destInterfaceId != arrivalInterfaceId && !fdb->isInterfaceBlocked(destInterfaceId)) {
            Packet* dupPacket = packet->dup();
            dupPacket->addTagIfAbsent<InterfaceReq>()->setInterfaceId(destInterfaceId);
            send(dupPacket, gate("ifOut"));
        }
    }
    delete packet;
}

void ForwardingRelayUnit::processMulticast(Packet* packet, int arrivalInterfaceId) {
    const auto& frame = packet->peekAtFront<EthernetMacHeader>();
    std::vector<int> destInterfaces = fdb->getDestInterfaceIds(frame->getDest(), simTime());

    if (destInterfaces.size() == 0) {
        EV_INFO << "No configured multicast forwarding entry found for packet "
                << packet->getName() << ". Falling back to broadcast!" << std::endl;
        processBroadcast(packet, arrivalInterfaceId);
    } else {
        std::ostringstream strBuffer;
        for (int interfaceIndex = 0; interfaceIndex < destInterfaces.size(); interfaceIndex++) {
            int destInterfaceId = destInterfaces[interfaceIndex];
            if (destInterfaceId != arrivalInterfaceId) {
                Packet* dupPacket = packet->dup();
                dupPacket->addTagIfAbsent<InterfaceReq>()->setInterfaceId(destInterfaceId);
                send(dupPacket, gate("ifOut"));
                if (interfaceIndex < destInterfaces.size() - 1) {
                    strBuffer << destInterfaceId << ", ";
                } else {
                    strBuffer << destInterfaceId;
                }
            }
        }
        EV_INFO << "Forwarding multicast packet " << packet->getName()
                << " to interfaces [" << strBuffer.str() << "]" << std::endl;
        delete packet;
    }
}

/* void ForwardingRelayUnit::processUnicast(Packet* packet, int arrivalInterfaceId) {
    //Learning MAC port mappings
    const auto& frame = packet->peekAtFront<EthernetMacHeader>();               
    learn(frame->getSrc(), arrivalInterfaceId);                                //这里get了源地址
    int destInterfaceId = fdb->getDestInterfaceId(frame->getDest(), simTime());//这里get了目的地址
    
    //Identify stream  可以尝试分别获取流的源与目的，源与目的不同则是不同流，源与目的相同则是相同流
    //但是在这之前要先对流进行ID赋值,创建一个经验库，存储已经处理的流的源与目的，流一进入就查看它的源地址与目的地址，然后与经验库进行对照
    //对照的结果如果是相同的，那么它们就是相同流，如果不同，就增加一个位置存储流，如果经验库是空的，那么就不用比较直接更新
    
    //Routing entry available or not?
    if (destInterfaceId == -1) {
        EV_INFO << "No unicast forwarding entry for packet " << packet->getName()
                << " found. Falling back to broadcast!" << std::endl;
        processBroadcast(packet, arrivalInterfaceId);
    } else {
        EV_INFO << "Forwarding unicast packet " << packet->getName()
                << " to interface " << destInterfaceId << std::endl;
        packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(destInterfaceId);
        send(packet, gate("ifOut"));
    }
} */

 void ForwardingRelayUnit::processUnicast(Packet* packet, int arrivalInterfaceId) {
    //Learning MAC port mappings
    const auto& frame = packet->peekAtFront<EthernetMacHeader>();              //首先把包取下来，用帧的形式保存           
    //learn(frame->getSrc(), arrivalInterfaceId);                                //这里get了源地址
    //int destInterfaceId = fdb->getDestInterfaceId(frame->getDest(), simTime());//这里get了目的地址 fdb到底表示什么意思
    
    //这里从包里取下源地址与目的地址
    flow F；
    F.id = frame->getSrc();                     ///这里是getsrc，我直接用来getid是不是没定义过；
    F.dst = frame->getDest();
    learn( F , arrivalInterfaceId);  
    int destInterfaceId = fdb->getDestInterfaceId( F , simTime());

    //Routing entry available or not?
    if (destInterfaceId == -1) {
        EV_INFO << "No unicast forwarding entry for packet " << packet->getName()
                << " found. Falling back to broadcast!" << std::endl;
        processBroadcast(packet, arrivalInterfaceId);
    } else {
        EV_INFO << "Forwarding unicast packet " << packet->getName()
                << " to interface " << destInterfaceId << std::endl;
        packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(destInterfaceId);
        send(packet, gate("ifOut"));
    }
} 

void ForwardingRelayUnit::learn(flow F, int arrivalInterfaceId)
{
    fdb->insert( F , simTime(), arrivalInterfaceId);                         //这个insert函数没找到 
}

} // namespace nesting


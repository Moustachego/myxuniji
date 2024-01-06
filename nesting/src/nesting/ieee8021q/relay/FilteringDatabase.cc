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

#include "nesting/ieee8021q/relay/FilteringDatabase.h"

#include "inet/common/ModuleAccess.h"

#include <iostream>
#include <vector>
#include <sstream>

namespace nesting {

Define_Module(FilteringDatabase);


FilteringDatabase::FilteringDatabase()                  //这里
{
    this->agingActive = false;
    this->agingThreshold = 0;
}

FilteringDatabase::FilteringDatabase(bool agingActive, simtime_t agingThreshold)
{
    this->agingActive = agingActive;
    this->agingThreshold = agingThreshold;
}

FilteringDatabase::~FilteringDatabase()
{
}

void FilteringDatabase::clearAdminFdb()
{
//    adminFdb.clear();
    adminFdb2.clear();
}
void FilteringDatabase::initialize(int stage) {                            //这里
    if (stage == INITSTAGE_LOCAL) {
        ifTable = check_and_cast<IInterfaceTable*>(getModuleByPath(par("interfaceTableModule")));
        WATCH_SET(blockedPorts);
    } else if (stage == INITSTAGE_LINK_LAYER) {
        EV_INFO << "test2"<< std::endl;                                   //现在在这里卡住？
        cXMLElement* fdb = par("database");  
        EV_INFO << "fdb"  << fdb << std::endl;                           //这个fdb每次都会变是什么情况                            
        loadDatabase(fdb);                                                 //进这里  //这里有问题

    }
}

int FilteringDatabase::numInitStages() const {
    return INITSTAGE_LINK_LAYER + 1;
}

void FilteringDatabase::loadDatabase(cXMLElement* xml) {
    std::string switchName = this->getModuleByPath(par("switchModule"))->getFullName();
    cXMLElement* fdb;
    EV_INFO << "fdb2"  << fdb << std::endl;
    //TODO this bool can probably be refactored to a nullptr check
    bool databaseFound = false;
    //try to extract the part of the filteringDatabase xml belonging to this module
    for (cXMLElement* host : xml->getChildren()) {
        if (host->hasAttributes() && host->getAttribute("id") == switchName) {
            fdb = host;
            databaseFound = true;
            break;
        }
    }
    EV_INFO << "fdb3"  << fdb << std::endl;
    //only continue if a filtering database was found for this switch
    if (!databaseFound) {
        return ;                                           //这里加了一个false //加false就错误了
    }

    // Get static rules from XML file
    cXMLElement* staticRules = fdb->getFirstChildWithTag("static");
    EV_INFO << "staticRules"  << staticRules << std::endl;
    if (staticRules != nullptr) {
        clearAdminFdb();

        cXMLElement* forwardingXml = staticRules->getFirstChildWithTag("forward");
        EV_INFO << "forwardingXml"  << forwardingXml << std::endl;
        if (forwardingXml != nullptr) {
            EV_INFO << "test3" << std::endl;
            this->parseEntries(forwardingXml);                   //进这里 没出来
        }

        cXMLElement* blockedPortsXml = staticRules->getFirstChildWithTag("blockedPorts");
        EV_INFO << "blockedPortsXml"  << blockedPortsXml << std::endl;          //这里没打印
        if (blockedPortsXml != nullptr) {
            this->parseBlockedPorts(blockedPortsXml);                    //进这里
        }
    }
    operFdb2.swap(adminFdb2);
//    operFdb.swap(adminFdb);
    clearAdminFdb();
}

void FilteringDatabase::parseEntries(cXMLElement* xml) {                            //解析条目
    // If present get rules from XML file
    if (xml == nullptr) {
        throw new cRuntimeError("Illegal xml input");
    }
    // Rules for individual addresses
    cXMLElementList individualAddresses = xml->getChildrenByTagName(                 //xml元素表获取单播地址
            "individualAddress");
    EV_INFO << "test4" << std::endl;
    for (auto individualAddress : individualAddresses) {                             //单播地址遍历？
        EV_INFO << "test5" << std::endl;
//        std::string macAddressStr = std::string(individualAddress->getAttribute("macAddress"));  //执行到这里就执行不下去了，应该就是这里的问题？
        std::string srcAddressStr = std::string(individualAddress->getAttribute("srcAddress"));   //存储源与目的地址；
        std::string dstAddressStr = std::string(individualAddress->getAttribute("dstAddress"));   //
        //如何输出这个目的地址与源地址
        EV_INFO << "test6" << std::endl;
        EV_INFO << "srcAddressStr"<< srcAddressStr << std::endl;
        if (srcAddressStr.empty()|dstAddressStr.empty()) {                                        //源与目的为空
                    throw cRuntimeError(
                            "individualAddress tag in forwarding database XML must have an "
                                    "macAddress attribute");
        }
//        if (macAddressStr.empty()) {
//            throw cRuntimeError(
//                    "individualAddress tag in forwarding database XML must have an "
//                            "macAddress attribute");
//        }

        if (!individualAddress->getAttribute("port")) {                                          //单播端口不为空
            throw cRuntimeError(
                    "individualAddress tag in forwarding database XML must have an "
                            "port attribute");
        }

        std::vector<int> interfaceIds;                                                  //接口ID
        int port = atoi(individualAddress->getAttribute("port"));                       //读取端口信息
        int interfaceId = ifTable->getInterface(port)->getInterfaceId();                //// 通过端口信息获取对应接口的ID
//        interfaceIds.insert(interfaceIds.begin(), 1, port);
        interfaceIds.insert(interfaceIds.begin(), 1, interfaceId);                      //将接口ID插入到vector的开头，初始值为1

        uint8_t vid = 0;                                                                //创建一个vid
        if (individualAddress->getAttribute("vid"))
            vid = static_cast<uint8_t>(atoi(
                    individualAddress->getAttribute("vid")));

        // Create and insert entry for different individual address types
        if (vid == 0) {
//            MacAddress macAddress;
            MacAddress srcAddress;                                                  //声明了一个名为源地址的Mac地址变量
            MacAddress dstAddress;
//            if (!macAddress.tryParse(macAddressStr.c_str())) {
//                throw new cRuntimeError("Cannot parse invalid Mac address.");
//            }
            if (!srcAddress.tryParse(srcAddressStr.c_str())) {                                  //如果尝试解析源地址失败
                throw new cRuntimeError("Cannot parse invalid Src address.");
            }
            if (!dstAddress.tryParse(dstAddressStr.c_str())) {
                throw new cRuntimeError("Cannot parse invalid Dst address.");
            }
            std::string a = srcAddress.str();                                                //将源地址字符化，传递给a（好像没什么用  ）
            std::string b = dstAddress.str();
            flow f;
            f.src = srcAddress;
            f.dst = dstAddress;
            adminFdb2.insert({f, std::pair<simtime_t, std::vector<int>>(0, interfaceIds)});
//            adminFdb.insert({macAddress, std::pair<simtime_t, std::vector<int>>(0, interfaceIds)});
        } else {
            // TODO
            throw cRuntimeError(
                    "Individual address rules with VIDs aren't supported yet");
        }
    }

    // Rules for multicastAddresses
    cXMLElementList multicastAddresses = xml->getChildrenByTagName(
            "multicastAddress");
    for (auto multicastAddress : multicastAddresses) {
        std::string dstAddressStr = std::string(
                multicastAddress->getAttribute("dstAddress"));
        std::string srcAddressStr = std::string(
                multicastAddress->getAttribute("srcAddress"));
//        std::string macAddressStr = std::string(
//                multicastAddress->getAttribute("macAddress"));
        if (dstAddressStr.empty()) {
            throw cRuntimeError(
                    "multicastAddress tag in forwarding database XML must have an "
                            "macAddress attribute");
        }

        if (!multicastAddress->getAttribute("ports")) {
            throw cRuntimeError(
                    "multicastAddress tag in forwarding database XML must have an "
                            "ports attribute");
        }
        std::string portsString = multicastAddress->getAttribute("ports");
        std::vector<int> ports;
        std::stringstream stream(portsString);
        while(1) {
           int n;
           stream >> n;
           if(!stream)
              break;
           ports.push_back(n);
        }

        std::vector<int> destInterfaces;
        for (int port : ports) {
            int interfaceId = ifTable->getInterface(port)->getInterfaceId();
            destInterfaces.push_back(interfaceId);
        }

        uint8_t vid = 0;
        if (multicastAddress->getAttribute("vid"))
            vid = static_cast<uint8_t>(atoi(
                    multicastAddress->getAttribute("vid")));

        // Create and insert entry for different individual address types
        if (vid == 0) {
            flow f;
            MacAddress srcAddress;
            srcAddress.tryParse(srcAddressStr.c_str());
            MacAddress dstAddress;
            if (!dstAddress.tryParse(dstAddressStr.c_str())) {
                throw new cRuntimeError("Cannot parse invalid Mac address.");
            }
            if (!dstAddress.isMulticast()) {
                throw new cRuntimeError(
                        "Mac address is not a Multicast address.");
            }
            f.src = srcAddress;
            f.dst = dstAddress;
//            MacAddress macAddress;
//            if (!macAddress.tryParse(macAddressStr.c_str())) {
//                throw new cRuntimeError("Cannot parse invalid Mac address.");
//            }
//            if (!macAddress.isMulticast()) {
//                throw new cRuntimeError(
//                        "Mac address is not a Multicast address.");
//            }
//            adminFdb.insert({macAddress, std::pair<simtime_t, std::vector<int>>(0, destInterfaces)});
            adminFdb2.insert({f, std::pair<simtime_t, std::vector<int>>(0, destInterfaces)});
        } else {
            // TODO
            throw cRuntimeError(
                    "Multicast address rules with VIDs aren't supported yet");
        }
    }
}

void FilteringDatabase::parseBlockedPorts(cXMLElement* xml) {
    blockedPorts.clear();

    std::string portsString = xml->getAttribute("ports");
    std::stringstream stream(portsString);
    while(stream.good()) {
        std::string substring;
        std::getline(stream, substring, ',');
        blockedPorts.insert(atoi(substring.c_str()));
    }
}

void FilteringDatabase::handleMessage(cMessage *msg) {
    throw cRuntimeError("Must not receive messages.");
}

//void FilteringDatabase::insert(MacAddress macAddress, simtime_t curTS, int interfaceId) {
void FilteringDatabase::insert(flow f, simtime_t curTS, int interfaceId) {
    std::vector<int> tmp;
    tmp.insert(tmp.begin(), 1, interfaceId);
//    operFdb[macAddress] = std::pair<simtime_t, std::vector<int>>(curTS, tmp);
    operFdb2[f] = std::pair<simtime_t, std::vector<int>>(curTS, tmp);
}

//int FilteringDatabase::getDestInterfaceId(MacAddress macAddress, simtime_t curTS) {
int FilteringDatabase::getDestInterfaceId(flow f, simtime_t curTS) {
    simtime_t ts;
    std::vector<int> port;

//    auto it = operFdb.find(macAddress);
    auto it = operFdb2.find(f);
    std::string srcAddressStr = std::string(f.src.str());
    std::string dstAddressStr = std::string(f.dst.str());
    //is element available?
//    if (it != operFdb.end()) {
    if (it != operFdb2.end()) {
        ts = it->second.first;
        port = it->second.second;
        // return if mac address belongs to multicast
        if (port.size() != 1) {
            return -1;                                //这个return可能有问题
        }
        // static entries (ts == 0) do not age
        if (!agingActive || (ts == 0 || curTS - ts < agingThreshold)) {
            operFdb2[f] = std::pair<simtime_t, std::vector<int>>(curTS,
                    port);
//            operFdb[macAddress] = std::pair<simtime_t, std::vector<int>>(curTS,
//                    port);
            return port.at(0);
        } else {
            operFdb2.erase(f);
//            operFdb.erase(macAddress);
        }
    }

    return -1;                                      //这个return可能有问题
}

//std::vector<int> FilteringDatabase::getDestInterfaceIds(MacAddress macAddress,
//        simtime_t curTS) {
std::vector<int> FilteringDatabase::getDestInterfaceIds(flow f,
        simtime_t curTS) {
    simtime_t ts;
    std::vector<int> ports;

//    if (!macAddress.isMulticast()) {
//        throw cRuntimeError("Expected multicast MAC address!");
//    }
    if (!f.dst.isMulticast()) {
        throw cRuntimeError("Expected multicast MAC address!");
    }

    //auto it = operFdb.find(macAddress);
    auto it = operFdb2.find(f);

    //is element available?
    //if (it != operFdb.end()) {
    if (it != operFdb2.end()) {
        ts = it->second.first;
        ports = it->second.second;
        // static entries (ts == 0) do not age
        if (!agingActive || (ts == 0 || curTS - ts < agingThreshold)) {
//            operFdb[macAddress] = std::pair<simtime_t, std::vector<int>>(curTS, ports);
            operFdb2[f] = std::pair<simtime_t, std::vector<int>>(curTS, ports);
            return ports;
        } else {
//            operFdb.erase(macAddress);
            operFdb2.erase(f);
        }

    }

    return ports;
}

bool FilteringDatabase::isInterfaceBlocked(int interfaceId)
{
    InterfaceEntry* interfaceEntry = ifTable->getInterfaceById(interfaceId);
    int portNumber = interfaceEntry->getIndex();
    return blockedPorts.find(portNumber) != blockedPorts.end();
}

} // namespace nesting

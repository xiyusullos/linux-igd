/******************************************************************************
*  Copyright (c) 2002 Linux UPnP Internet Gateway Device Project              *    
*  All rights reserved.                                                       *
*                                                                             *   
*  This file is part of The Linux UPnP Internet Gateway Device (IGD).         *
*                                                                             *
*  The Linux UPnP IGD is free software; you can redistribute it and/or modify *
*  it under the terms of the GNU General Public License as published by       *
*  the Free Software Foundation; either version 2 of the License, or          *
*  (at your option) any later version.                                        *
*                                                                             *    
*  The Linux UPnP IGD is distributed in the hope that it will be useful,      *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*  GNU General Public License for more details.                               *
*                                                                             *   
*  You should have received a copy of the GNU General Public License          * 
*  along with Foobar; if not, write to the Free Software                      *
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  *
*                                                                             *  
*                                                                             *  
******************************************************************************/

#ifndef _GATE_H_
#define _GATE_H_

#include <upnp/upnp.h>

#include "ipcon.h"
#include "pmlist.h"

class Gate
{
	public:
		Gate();
		~Gate();
		int GateDeviceHandleSubscriptionRequest(struct Upnp_Subscription_Request *sr_event);
		int GateDeviceStateTableInit(char *DescDocURL);
		int GateDeviceHandleGetVarRequest(struct Upnp_State_Var_Request *cg_event);
		int GateDeviceHandleActionRequest(struct Upnp_Action_Request *ca_event);
		int GateDeviceCallbackEventHandler(Upnp_EventType EventType, void *Event, void *Cookie);
		int GateDeviceSetConnectionType(struct Upnp_Action_Request *ca_event);
		int GateDeviceGetConnectionTypeInfo(struct Upnp_Action_Request *ca_event);
		int GateDeviceRequestConnection(struct Upnp_Action_Request *ca_event);
		int GateDeviceRequestTermination(struct Upnp_Action_Request *ca_event);
		int GateDeviceForceTermination(struct Upnp_Action_Request *ca_event);
		int GateDeviceSetAutoDisconnectTime(struct Upnp_Action_Request *ca_event);
		int GateDeviceSetIdleDisconnectTime(struct Upnp_Action_Request *ca_event);
		int GateDeviceSetWarnDisconnectDelay(struct Upnp_Action_Request *ca_event);
		int GateDeviceGetStatusInfo(struct Upnp_Action_Request *ca_event);
		int GateDeviceGetAutoDisconnectTime(struct Upnp_Action_Request *ca_event);
		int GateDeviceGetIdleDisconnectTime(struct Upnp_Action_Request *ca_event);
		int GateDeviceGetWarnDisconnectDelay(struct Upnp_Action_Request *ca_event);
		int GateDeviceGetNATRSIPStatus(struct Upnp_Action_Request *ca_event);
		int GateDeviceGetGenericPortMappingEntry(struct Upnp_Action_Request *ca_event);
		int GateDeviceGetSpecificPortMappingEntry(struct Upnp_Action_Request *ca_event);
		int GateDeviceGetExternalIPAddress(struct Upnp_Action_Request *ca_event);
		int GateDeviceAddPortMapping(struct Upnp_Action_Request *ca_event);
		int GateDeviceDeletePortMapping(struct Upnp_Action_Request *ca_event);
	
		int GateDeviceX(struct Upnp_Action_Request *ca_event);
		int GateDeviceGetCommonLinkProperties(struct Upnp_Action_Request *ca_event);
                int GateDeviceGetTotalBytesSent(struct Upnp_Action_Request *ca_event);
                int GateDeviceGetTotalBytesReceived(struct Upnp_Action_Request *ca_event);
                int GateDeviceGetTotalPacketsSent(struct Upnp_Action_Request *ca_event);
                int GateDeviceGetTotalPacketsReceived(struct Upnp_Action_Request *ca_event);


		PortMapList m_list;
		UpnpDevice_Handle device_handle;
		char *gate_udn;
		IPCon *m_ipcon;
		
		// State Variables
		char m_ConnectionType[50];
		char m_PossibleConnectionTypes[50];
		char m_ConnectionStatus[20];
		long int startup_time;
		char LastConnectionError[35];
		long int m_AutoDisconnectTime;
		long int m_IdleDisconnectTime;
		long int m_WarnDisconnectDelay;
		bool RSIPAvailable;
		bool NATEnabled;
		char m_ExternalIPAddress[20];
		int m_PortMappingNumberOfEntries;
		bool m_PortMappingEnabled;
			
};

#endif

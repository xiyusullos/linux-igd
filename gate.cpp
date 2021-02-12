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

#include <string.h>
#include <syslog.h>
#include <pthread.h>

#include "gateway.h"
#include "sample_util.h"
#include "portmap.h"
#include <arpa/inet.h>
#include "gate.h"
#include <unistd.h>

pthread_mutex_t DevMutex = PTHREAD_MUTEX_INITIALIZER;

char *GateDeviceType[] = {"urn:schemas-upnp-org:device:InternetGatewayDevice:1"
                        ,"urn:schemas-upnp-org:device:WANDevice:1"
                         ,"urn:schemas-upnp-org:device:WANConnectionDevice:1"};

char *GateServiceType[] = {"urn:schemas-microsoft-com:service:OSInfo:1"
                          ,"urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1"
                          ,"urn:schemas-upnp-org:service:WANIPConnection:1"};

char *GateServiceId[] = {"urn:microsoft-com:serviceId:OSInfo1"
                        ,"urn:upnp-org:serviceId:WANCommonIFC1"
                        ,"urn:upnp-org:serviceId:WANIPConn1"};

int getProtoNum(char * proto)
{
	if (strcmp(proto,"TCP")==0)
                return 6;
        else if (strcmp(proto,"UDP")==0)
                return 17;
	else return 0;
}
char *getProtoName(int prt)
{
	switch (prt)
	{
		case 6:
			return "TCP";
			break;
		case 17:
			return "UDP";
			break;
		default:
			return "TCP";
	}
}

int chkIPADDRstring(char * addr) {
        struct in_addr *dmy=NULL;
        return inet_aton(addr, dmy);
}

Gate::Gate()
{
}


Gate::~Gate()
{
	if (gate_udn)
		delete [] gate_udn;
	if (m_ipcon)
		delete m_ipcon;
}

int Gate::GateDeviceCallbackEventHandler(Upnp_EventType EventType,
                         void *Event,
                         void *Cookie)
{
	switch ( EventType)
	{

		case UPNP_EVENT_SUBSCRIPTION_REQUEST:
			GateDeviceHandleSubscriptionRequest((struct Upnp_Subscription_Request *) Event);
			break;

		case UPNP_CONTROL_GET_VAR_REQUEST:
			GateDeviceHandleGetVarRequest((struct Upnp_State_Var_Request *) Event);
			break;

		case UPNP_CONTROL_ACTION_REQUEST:
			GateDeviceHandleActionRequest((struct Upnp_Action_Request *) Event);
			break;

		default:

			syslog(LOG_DEBUG, "Error in DeviceCallbackEventHandler: unknown event type %d\n", EventType);
	}

	return(0);
}


int Gate::GateDeviceStateTableInit (char* DescDocURL)
{
	Upnp_Document DescDoc = NULL;
	int ret = UPNP_E_SUCCESS;

	if (UpnpDownloadXmlDoc(DescDocURL, &DescDoc) != UPNP_E_SUCCESS)
	{
		syslog(LOG_ERR, "DeviceStateTableInit -- Error Parsing %s\n", DescDocURL);
		ret =UPNP_E_INVALID_DESC;
	}

	gate_udn = SampleUtil_GetFirstDocumentItem(DescDoc, "UDN");

	return (ret);
}

int Gate::GateDeviceHandleSubscriptionRequest (struct Upnp_Subscription_Request *sr_event)
{
	Upnp_Document PropSet;
	PropSet = NULL;
	char *address;
	address = m_ipcon->IPCon_GetIpAddrStr();

	pthread_mutex_lock(&DevMutex);

	if (strcmp(sr_event->UDN, gate_udn) == 0)
	{
		if (strcmp(sr_event->ServiceId, GateServiceId[GATE_SERVICE_OSINFO]) == 0)
		{
			UpnpAddToPropertySet(&PropSet, "OSMajorVersion","5");
			UpnpAddToPropertySet(&PropSet, "OSMinorVersion","1");
			UpnpAddToPropertySet(&PropSet, "OSBuildNumber","2600");
			UpnpAddToPropertySet(&PropSet, "OSMachineName","Linux IGD");
			UpnpAcceptSubscriptionExt(device_handle, sr_event->UDN,
					sr_event->ServiceId, PropSet, sr_event->Sid);
			UpnpDocument_free(PropSet);


		}
		else if (strcmp(sr_event->ServiceId, GateServiceId[GATE_SERVICE_CONFIG]) == 0)
		{
			UpnpAddToPropertySet(&PropSet, "PhysicalLinkStatus", m_ipcon->IPCon_GetIfStatStr());
			UpnpAcceptSubscriptionExt(device_handle, sr_event->UDN, sr_event->ServiceId, PropSet, sr_event->Sid);
			UpnpDocument_free(PropSet);
		}
		else if (strcmp(sr_event->ServiceId, GateServiceId[GATE_SERVICE_CONNECT])==0)
		{
			UpnpAddToPropertySet(&PropSet, "PossibleConnectionTypes","IP_Routed");
			UpnpAddToPropertySet(&PropSet, "ConnectionStatus","Connected");
			UpnpAddToPropertySet(&PropSet, "X_Name","Local Area Connection");
			UpnpAddToPropertySet(&PropSet, "ExternalIPAddress",address);
			UpnpAddToPropertySet(&PropSet, "PortMappingNumberOfEntries","0");
			UpnpAcceptSubscriptionExt(device_handle, sr_event->UDN,	sr_event->ServiceId, PropSet, sr_event->Sid);
			UpnpDocument_free(PropSet);
		}
	}
	pthread_mutex_unlock(&DevMutex);
	if (address) delete [] address;
	return(1);
}

int Gate::GateDeviceHandleGetVarRequest(struct Upnp_State_Var_Request *cgv_event)
{
	int getvar_succeeded = 0;
	cgv_event->CurrentVal = NULL;
	pthread_mutex_lock(&DevMutex);

        if (getvar_succeeded)
       	{
	        cgv_event->ErrCode = UPNP_E_SUCCESS;
	}
       	else
       	{
		syslog(LOG_ERR,"Error in UPNP_CONTROL_GET_VAR_REQUEST callback:\n");
		syslog(LOG_ERR,"   Unknown variable name = %s\n",  cgv_event->StateVarName);
		cgv_event->ErrCode = 404;
		strcpy(cgv_event->ErrStr, "Invalid Variable");
        }
	pthread_mutex_unlock(&DevMutex);
	return(cgv_event->ErrCode == UPNP_E_SUCCESS);
}

int Gate::GateDeviceHandleActionRequest(struct Upnp_Action_Request *ca_event)
{
	int result = 0;
	
	pthread_mutex_lock(&DevMutex);
	if (strcmp(ca_event->DevUDN, gate_udn) == 0)
	{
		if (strcmp(ca_event->ServiceID,"urn:upnp-org:serviceId:WANIPConn1") ==0)
		{
				if (strcmp(ca_event->ActionName,"SetConnectionType") == 0)
				     	result = GateDeviceSetConnectionType(ca_event);
				else if (strcmp(ca_event->ActionName,"GetConnectionTypeInfo") == 0)
					result = GateDeviceGetConnectionTypeInfo(ca_event);
				else if (strcmp(ca_event->ActionName,"RequestConnection") == 0)
					result = GateDeviceRequestConnection(ca_event);
				else if (strcmp(ca_event->ActionName,"RequestTermination") == 0)
					result = GateDeviceRequestTermination(ca_event);
				else if (strcmp(ca_event->ActionName,"ForceTermination") == 0)
					result = GateDeviceForceTermination(ca_event);
				else if (strcmp(ca_event->ActionName,"SetAutoDisconnectTime") == 0)
					result = GateDeviceSetAutoDisconnectTime(ca_event);
				else if (strcmp(ca_event->ActionName,"SetIdleDisconnectTime") == 0)
					result = GateDeviceSetIdleDisconnectTime(ca_event);
				else if (strcmp(ca_event->ActionName,"SetWarnDisconnectDelay") == 0)
					result = GateDeviceSetWarnDisconnectDelay(ca_event);
				else if (strcmp(ca_event->ActionName,"GetStatusInfo") == 0)
					result = GateDeviceGetStatusInfo(ca_event);
				else if (strcmp(ca_event->ActionName,"GetAutoDisconnectTime") == 0)
					result = GateDeviceGetAutoDisconnectTime(ca_event);
				else if (strcmp(ca_event->ActionName,"GetIdleDisconnectTime") == 0)
					result = GateDeviceGetIdleDisconnectTime(ca_event);
				else if (strcmp(ca_event->ActionName,"GetWarnDisconnectDelay") == 0)
					result = GateDeviceGetWarnDisconnectDelay(ca_event);
				else if (strcmp(ca_event->ActionName,"GetNATRSIPStatus") == 0)
					result = GateDeviceGetNATRSIPStatus(ca_event);
				else if (strcmp(ca_event->ActionName,"GetGenericPortMappingEntry") == 0)
					result = GateDeviceGetGenericPortMappingEntry(ca_event);
				else if (strcmp(ca_event->ActionName,"GetSpecificPortMappingEntry") == 0)
					result = GateDeviceGetSpecificPortMappingEntry(ca_event);
				else if (strcmp(ca_event->ActionName,"GetExternalIPAddress") == 0)
					result = GateDeviceGetExternalIPAddress(ca_event);
				else if (strcmp(ca_event->ActionName,"AddPortMapping") == 0)
					result = GateDeviceAddPortMapping(ca_event);
				else if (strcmp(ca_event->ActionName,"DeletePortMapping") == 0)
					result = GateDeviceDeletePortMapping(ca_event);
				else result = 0;
		}
		else if (strcmp(ca_event->ServiceID,GateServiceId[GATE_SERVICE_CONFIG]) == 0)
		{
			if (strcmp(ca_event->ActionName,"GetCommonLinkProperties") == 0)
				result = GateDeviceGetCommonLinkProperties(ca_event);
			else if (strcmp(ca_event->ActionName, "X") == 0)
				result = GateDeviceX(ca_event);
			else if (strcmp(ca_event->ActionName,"GetTotalBytesSent") == 0)
				result = GateDeviceGetTotalBytesSent(ca_event);
			else if (strcmp(ca_event->ActionName,"GetTotalBytesReceived") == 0)
				result = GateDeviceGetTotalBytesReceived(ca_event);
			else if (strcmp(ca_event->ActionName,"GetTotalPacketsSent") == 0)
				result = GateDeviceGetTotalPacketsSent(ca_event);
			else if (strcmp(ca_event->ActionName,"GetTotalPacketsReceived") == 0)
				result = GateDeviceGetTotalPacketsReceived(ca_event);
			else result = 0;
		}
	}
	
	pthread_mutex_unlock(&DevMutex);

	return(result);
}

int Gate::GateDeviceX(struct Upnp_Action_Request *ca_event)
{
	ca_event->ErrCode = 401;
	strcpy(ca_event->ErrStr, "Invalid Action");
	ca_event->ActionResult = NULL;
	return (ca_event->ErrCode);
}

int Gate::GateDeviceGetCommonLinkProperties(struct Upnp_Action_Request *ca_event)
{
	char result_str[500];

        ca_event->ErrCode = UPNP_E_SUCCESS;
        sprintf(result_str, "<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>", ca_event->ActionName,
                "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1",
                "<NewWANAccessType>Cable</NewWANAccessType><NewLayer1UpstreamMaxBitRate>131072</NewLayer1UpstreamMaxBitRate><NewLayer1DownstreamMaxBitRate>614400</NewLayer1DownstreamMaxBitRate><NewPhysicalLinkStatus>Up</NewPhysicalLinkStatus>",
                ca_event->ActionName);
        ca_event->ActionResult = UpnpParse_Buffer(result_str);

        return(ca_event->ErrCode);


}

int Gate::GateDeviceGetTotalBytesSent(struct Upnp_Action_Request *ca_event)
{
	char result_str[500];
	char dev[15];
	char *iface;
	FILE *stream;
	unsigned long bytes=0, total=0;

	/* Read sent from /proc */
	stream = fopen ( "/proc/net/dev", "r" );
	if ( stream != NULL )
	{
		iface=m_ipcon->IPCon_GetIfName();
		while ( getc ( stream ) != '\n' );
		while ( getc ( stream ) != '\n' );

		while ( !feof( stream ) )
		{
			fscanf ( stream, "%[^:]:%*u %*u %*u %*u %*u %*u %*u %*u %lu %*u %*u %*u %*u %*u %*u %*u\n", dev, &bytes );
			if ( strcmp ( dev, iface )==0 )
				total += bytes;
		}
		fclose ( stream );
	}
	else
	{
		total=1;
	}

        ca_event->ErrCode = UPNP_E_SUCCESS;
        sprintf(result_str, "<u:%sResponse xmlns:u=\"%s\">\n<NewTotalBytesSent>%lu</NewTotalBytesSent>\n</u:%sResponse>",
		ca_event->ActionName,
                "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1",
                total, ca_event->ActionName);
        ca_event->ActionResult = UpnpParse_Buffer(result_str);

        return(ca_event->ErrCode);

}
int Gate::GateDeviceGetTotalBytesReceived(struct Upnp_Action_Request *ca_event)
{
	char result_str[500];
	char dev[15];
        char *iface;
        FILE *stream;
        unsigned long bytes=0,total=0;

	/* Read received from /proc */
        stream = fopen ( "/proc/net/dev", "r" );
        if ( stream != NULL )
        {
                iface=m_ipcon->IPCon_GetIfName();

                while ( getc ( stream ) != '\n' );
                while ( getc ( stream ) != '\n' );

                while ( !feof( stream ) )
                {
                        fscanf ( stream, "%[^:]:%lu %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u\n", dev, &bytes );
                        if ( strcmp ( dev, iface )==0 )
                                total += bytes;
                }
		fclose ( stream );
        }
	else
	{
                total=1;
        }

        ca_event->ErrCode = UPNP_E_SUCCESS;
        sprintf(result_str, "<u:%sResponse xmlns:u=\"%s\">\n<NewTotalBytesReceived>%lu</NewTotalBytesReceived>\n</u:%sResponse>",
		ca_event->ActionName,
                "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1",
                total, ca_event->ActionName );
        ca_event->ActionResult = UpnpParse_Buffer(result_str);

        return(ca_event->ErrCode);

}
int Gate::GateDeviceGetTotalPacketsSent(struct Upnp_Action_Request *ca_event)
{
	char result_str[500];
	char dev[15];
        char *iface;
        FILE *stream;
        unsigned long pkt=0, total=0;

        /* Read sent from /proc */
        stream = fopen ( "/proc/net/dev", "r" );
        if ( stream != NULL )
        {
                iface=m_ipcon->IPCon_GetIfName();
                while ( getc ( stream ) != '\n' );
                while ( getc ( stream ) != '\n' );

                while ( !feof( stream ) )
                {
                        fscanf ( stream, "%[^:]:%*u %*u %*u %*u %*u %*u %*u %*u %*u %lu %*u %*u %*u %*u %*u %*u\n", dev, &pkt );
                        if ( strcmp ( dev, iface )==0 )
                                total += pkt;
		}
                fclose ( stream );
        }
        else
        {
                total=1;
        }

        ca_event->ErrCode = UPNP_E_SUCCESS;
        sprintf(result_str, "<u:%sResponse xmlns:u=\"%s\">\n<NewTotalPacketsSent>%lu</NewTotalPacketsSent>\n</u:%sResponse>", ca_event->ActionName,
                "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1",
                total, ca_event->ActionName);
        ca_event->ActionResult = UpnpParse_Buffer(result_str);

        return(ca_event->ErrCode);

}
int Gate::GateDeviceGetTotalPacketsReceived(struct Upnp_Action_Request *ca_event)
{
	char result_str[500];
	char dev[15];
        char *iface;
        FILE *stream;
        unsigned long pkt=0, total=0;

        /* Read sent from /proc */
        stream = fopen ( "/proc/net/dev", "r" );
        if ( stream != NULL )
        {
                iface=m_ipcon->IPCon_GetIfName();
                while ( getc ( stream ) != '\n' );
                while ( getc ( stream ) != '\n' );

                while ( !feof( stream ) )
                {
                        fscanf ( stream, "%[^:]:%*u %lu %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u\n", dev, &pkt );
                        if ( strcmp ( dev, iface )==0 )
                                total += pkt;
                }
                fclose ( stream );
        }
        else
	{
                total=1;
        }

        ca_event->ErrCode = UPNP_E_SUCCESS;
        sprintf(result_str, "<u:%sResponse xmlns:u=\"%s\">\n<NewTotalPacketsReceived>%lu</NewTotalPacketsReceived>\n</u:%sResponse>", ca_event->ActionName,
                "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1",
                total, ca_event->ActionName);
        ca_event->ActionResult = UpnpParse_Buffer(result_str);

        return(ca_event->ErrCode);

}

int Gate::GateDeviceSetConnectionType(struct Upnp_Action_Request *ca_event)
{
	return(UPNP_E_SUCCESS);
}

int Gate::GateDeviceGetConnectionTypeInfo(struct Upnp_Action_Request *ca_event)
{
	char result_str[500];

	ca_event->ErrCode = UPNP_E_SUCCESS;
	sprintf(result_str, "<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>", ca_event->ActionName,
		"urn:schemas-upnp-org:service:WANIPConnection:1",
		"<NewConnectionType>IP_Routed</NewConnectionType>\n<NewPossibleConnectionTypes>IP_Routed</NewPossibleConnectionTypes>",
		ca_event->ActionName);
	ca_event->ActionResult = UpnpParse_Buffer(result_str);

	return(ca_event->ErrCode);
}

int Gate::GateDeviceRequestConnection(struct Upnp_Action_Request *ca_event)
{
	return(UPNP_E_SUCCESS);
}

int Gate::GateDeviceRequestTermination(struct Upnp_Action_Request *ca_event)
{
	return(UPNP_E_SUCCESS);
}

int Gate::GateDeviceForceTermination(struct Upnp_Action_Request *ca_event)
{
	return(UPNP_E_SUCCESS);
}

int Gate::GateDeviceSetAutoDisconnectTime(struct Upnp_Action_Request *ca_event)
{
	return(UPNP_E_SUCCESS);
}

int Gate::GateDeviceSetIdleDisconnectTime(struct Upnp_Action_Request *ca_event)
{
	return(UPNP_E_SUCCESS);
}

int Gate::GateDeviceSetWarnDisconnectDelay(struct Upnp_Action_Request *ca_event)
{
	return(UPNP_E_SUCCESS);
}

int Gate::GateDeviceGetStatusInfo(struct Upnp_Action_Request *ca_event)
{

	long int uptime;
	char result_str[500];
        
	uptime = (time(NULL) - startup_time);
	ca_event->ErrCode = UPNP_E_SUCCESS;
        sprintf(result_str, "<u:%sResponse xmlns:u=\"%s\">\n<NewConnectionStatus>%s</NewConnectionStatus><NewLastConnectionError>ERROR_NONE</NewLastConnectionError><NewUptime>%li</NewUptime>\n</u:%sResponse>", ca_event->ActionName,
	                "urn:schemas-upnp-org:service:WANIPConnection:1",
	                "Connected",uptime,
	                ca_event->ActionName);
        ca_event->ActionResult = UpnpParse_Buffer(result_str);
        return(ca_event->ErrCode);
}

int Gate::GateDeviceGetAutoDisconnectTime(struct Upnp_Action_Request *ca_event)
{
	return(UPNP_E_SUCCESS);
}

int Gate::GateDeviceGetIdleDisconnectTime(struct Upnp_Action_Request *ca_event)
{
	return(UPNP_E_SUCCESS);
}

int Gate::GateDeviceGetWarnDisconnectDelay(struct Upnp_Action_Request *ca_event)
{
	return(UPNP_E_SUCCESS);
}

int Gate::GateDeviceGetNATRSIPStatus(struct Upnp_Action_Request *ca_event)
{
	char result_str[500];

	ca_event->ErrCode = UPNP_E_SUCCESS;
	sprintf(result_str, "<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>", ca_event->ActionName,
		"urn:schemas-upnp-org:service:WANIPConnection:1",
		"<NewRSIPAvailable>0</NewRSIPAvailable>\n<NewNATEnabled>1</NewNATEnabled>",
		ca_event->ActionName);
	ca_event->ActionResult = UpnpParse_Buffer(result_str);

	return(ca_event->ErrCode);
}

int Gate::GateDeviceGetExternalIPAddress(struct Upnp_Action_Request *ca_event)
{
	char result_str[500];
	char result_parm[500];
	char *ip_address;

	ip_address = m_ipcon->IPCon_GetIpAddrStr();	

	ca_event->ErrCode = UPNP_E_SUCCESS;
	sprintf(result_parm,"<NewExternalIPAddress>%s</NewExternalIPAddress>", ip_address);
	sprintf(result_str, "<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>", ca_event->ActionName,
		"urn:schemas-upnp-org:service:WANIPConnection:1",result_parm, ca_event->ActionName);
	ca_event->ActionResult = UpnpParse_Buffer(result_str);
	
	
	if (ip_address) delete [] ip_address;

	return(ca_event->ErrCode);
}

int Gate::GateDeviceGetGenericPortMappingEntry(struct Upnp_Action_Request *ca_event)
{
	int i = 0;
	int index = 0;
	char *mapindex = NULL;
	int action_succeeded = 0;
	char result_parm[500];
	char result_str[500];

	if ((mapindex = SampleUtil_GetFirstDocumentItem(ca_event->ActionRequest, "NewPortMappingIndex")))
	{
		index = atoi(mapindex);
		for (list<PortMap *>::iterator itr = m_list.m_pmap.begin(); itr != m_list.m_pmap.end(); itr++,i++)
		{
			if (i == index)
			{
				sprintf(result_parm,"<NewRemoteHost>%s</NewRemoteHost><NewExternalPort>%d</NewExternalPort><NewProtocol>%s</NewProtocol><NewInternalPort>%d</NewInternalPort><NewInternalClient>%s</NewInternalClient><NewEnabled>%d</NewEnabled><NewPortMappingDescription>%s</NewPortMappingDescription><NewLeaseDuration>%li</NewLeaseDuration>",
					"",(*itr)->m_ExternalPort, (*itr)->m_PortMappingProtocol, (*itr)->m_InternalPort,
				       	(*itr)->m_InternalClient, 1, (*itr)->m_PortMappingDescription, (*itr)->m_PortMappingLeaseDuration);		
				action_succeeded = 1;
			}
	
		}
		if (action_succeeded)
		{
			ca_event->ErrCode = UPNP_E_SUCCESS;
	                sprintf(result_str, "<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>", ca_event->ActionName,
	                        "urn:schemas-upnp-org:service:WANIPConnection:1",result_parm, ca_event->ActionName);
	                ca_event->ActionResult = UpnpParse_Buffer(result_str);
		}
		else
		{
			ca_event->ErrCode = 713;
                        strcpy(ca_event->ErrStr, "SpecifiedArrayIndexInvalid");
                        ca_event->ActionResult = NULL;
		}

	}
	else
	{
	         syslog(LOG_DEBUG, "Failure in GateDeviceGetGenericortMappingEntry: Invalid Args");
	         ca_event->ErrCode = 402;
                 strcpy(ca_event->ErrStr, "Invalid Args");
                 ca_event->ActionResult = NULL;
	}

	if (mapindex) delete [] mapindex;
	return(ca_event->ErrCode);
}

int Gate::GateDeviceGetSpecificPortMappingEntry(struct Upnp_Action_Request *ca_event)
{
	char *ext_port=NULL;
	char *proto=NULL;
	char result_parm[500];
	char result_str[500];
	int prt;
	int action_succeeded = 0;

	if ((ext_port = SampleUtil_GetFirstDocumentItem(ca_event->ActionRequest, "NewExternalPort")) 
		&& (proto = SampleUtil_GetFirstDocumentItem(ca_event->ActionRequest,"NewProtocol")))
	{
		prt = getProtoNum(proto);
		if ((prt == 17) || (prt == 6))
		{
			for (list<PortMap *>::iterator itr = m_list.m_pmap.begin(); itr != m_list.m_pmap.end(); itr++)
			{
				if ((((*itr)->m_ExternalPort == atoi(ext_port)) && (strcmp((*itr)->m_PortMappingProtocol,proto) == 0)))
				{
					sprintf(result_parm,"<NewInternalPort>%d</NewInternalPort><NewInternalClient>%s</NewInternalClient><NewEnabled>1</NewEnabled><NewPortMappingDescription>%s</NewPortMappingDescription><NewLeaseDuration>%li</NewLeaseDuration>",
							(*itr)->m_InternalPort, 
							(*itr)->m_InternalClient,
							(*itr)->m_PortMappingDescription,
							(*itr)->m_PortMappingLeaseDuration);
					action_succeeded = 1;
				}
			}
			if (action_succeeded)
			{
				ca_event->ErrCode = UPNP_E_SUCCESS;
		                sprintf(result_str, "<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>", ca_event->ActionName,
		                        "urn:schemas-upnp-org:service:WANIPConnection:1",result_parm, ca_event->ActionName);
		                ca_event->ActionResult = UpnpParse_Buffer(result_str);
			}
			else
			{
				syslog(LOG_DEBUG, "Failure in GateDeviceGetSpecificPortMappingEntry: PortMapping Doesn't Exist...");
		                ca_event->ErrCode = 714;
		                strcpy(ca_event->ErrStr, "NoSuchEntryInArray");
		                ca_event->ActionResult = NULL;
			}
		}
		else
		{
		        syslog(LOG_DEBUG, "Failure in GateDeviceGetSpecificPortMappingEntry: Invalid NewProtocol=%s\n",proto);
                        ca_event->ErrCode = 402;
                        strcpy(ca_event->ErrStr, "Invalid Args");
                        ca_event->ActionResult = NULL;
		}
	}
	else
	{
		syslog(LOG_DEBUG, "Failure in GateDeviceGetSpecificPortMappingEntry: Invalid Args");
		ca_event->ErrCode = 402;
		strcpy(ca_event->ErrStr, "Invalid Args");
		ca_event->ActionResult = NULL;
	}
	
	return (ca_event->ErrCode);
}

int Gate::GateDeviceAddPortMapping(struct Upnp_Action_Request *ca_event)
{
	char *remote_host=NULL;
	char *ext_port=NULL;
	char *proto=NULL; 
	char *int_port=NULL;
	char *int_ip=NULL;
	char *desc=NULL;
	int prt,result=0;
	char num[5];
	char result_str[500];
	char *address = NULL;
	Upnp_Document  PropSet= NULL;	
	int action_succeeded = 0;
	
	address = m_ipcon->IPCon_GetIpAddrStr();
	
	if (((ext_port = SampleUtil_GetFirstDocumentItem(ca_event->ActionRequest, "NewExternalPort"))
		&& (proto    = SampleUtil_GetFirstDocumentItem(ca_event->ActionRequest,"NewProtocol"))
		&& (int_port = SampleUtil_GetFirstDocumentItem(ca_event->ActionRequest, "NewInternalPort"))
		&& (int_ip   = SampleUtil_GetFirstDocumentItem(ca_event->ActionRequest, "NewInternalClient"))
		&& (desc     = SampleUtil_GetFirstDocumentItem(ca_event->ActionRequest, "NewPortMappingDescription"))))
	{
		prt = getProtoNum(proto);
		if ((prt == 6) || (prt ==17) )
		{
			if (chkIPADDRstring(int_ip)!=0)
			{
				for (list<PortMap *>::iterator itr=m_list.m_pmap.begin(); itr != m_list.m_pmap.end(); itr++)
				{
					if (((*itr)->m_ExternalPort == atoi(ext_port)) 
						&& (strcmp((*itr)->m_PortMappingProtocol,proto) == 0) 
						&& (strcmp((*itr)->m_InternalClient,int_ip) == 0))
					{
						m_list.delPortForward((*itr)->m_PortMappingProtocol, 
								(*itr)->m_ExternalIP,
								(*itr)->m_ExternalPort, (*itr)->m_InternalClient,
								(*itr)->m_InternalPort);
						delete *itr;
						m_list.m_pmap.erase(itr);
						itr--;
					}
				}
			
				result=m_list.PortMapAdd(NULL, proto, address, atoi(ext_port), int_ip, atoi(int_port), 1, desc, 0);
				if (result==1)
				{
					sprintf(num,"%d",m_list.m_pmap.size());
					PropSet= UpnpCreatePropertySet(1,"PortMappingNumberOfEntries", num);
					UpnpNotifyExt(device_handle, ca_event->DevUDN,ca_event->ServiceID,PropSet);
					UpnpDocument_free(PropSet);
					syslog(LOG_DEBUG, "AddPortMap: RemoteHost: %s Prot: %d ExtPort: %d Int: %s.%d\n",
						remote_host, prt, atoi(ext_port), int_ip, atoi(int_port));
					action_succeeded = 1;
				}
				else
				{
					if (result==718)
					{
						syslog(LOG_DEBUG,"Failure in GateDeviceAddPortMapping: RemoteHost: %s Prot:%d ExtPort: %d Int: %s.%d\n",
							remote_host,prt, atoi(ext_port),int_ip, atoi(int_port));
						ca_event->ErrCode = 718;
						strcpy(ca_event->ErrStr, "ConflictInMappingEntry");
						ca_event->ActionResult = NULL;
					}
				}
			}
			else
			{
		                 syslog(LOG_DEBUG, "Failure in GateDeviceAddPortMapping: Invalid NewInternalClient=%s\n",int_ip);
                                 ca_event->ErrCode = 402;
                                 strcpy(ca_event->ErrStr, "Invalid Args");
                                 ca_event->ActionResult = NULL;
			}
		}
		else
		{
		      syslog(LOG_DEBUG, "Failure in GateDeviceAddPortMapping: Invalid NewProtocol=%s\n",proto);
                      ca_event->ErrCode = 402;
                      strcpy(ca_event->ErrStr, "Invalid Args");
                      ca_event->ActionResult = NULL;
		}
	}
	else
	{
		syslog(LOG_DEBUG, "Failiure in GateDeviceAddPortMapping: Invalid Arguments!");
                ca_event->ErrCode = 402;
                strcpy(ca_event->ErrStr, "Invalid Args");
                ca_event->ActionResult = NULL;
	}
	
	if (action_succeeded)
	{
		ca_event->ErrCode = UPNP_E_SUCCESS;
		sprintf(result_str, "<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>",
			ca_event->ActionName, "urn:schemas-upnp-org:service:WANIPConnection:1", "", ca_event->ActionName);
		ca_event->ActionResult = UpnpParse_Buffer(result_str);
	}

	if (ext_port) delete [] ext_port;
	if (int_port) delete [] int_port;
	if (proto) delete [] proto;
	if (int_ip) delete [] int_ip;
	if (desc) delete [] desc;
	if (remote_host) delete [] remote_host;	
	if (address) delete [] address;
	return(ca_event->ErrCode);
}

int Gate::GateDeviceDeletePortMapping(struct Upnp_Action_Request *ca_event)
{
	char *ext_port=NULL;
	char *proto=NULL;
	int prt,result=0;
	char num[5];
	char result_str[500];
	Upnp_Document  PropSet= NULL;
	int action_succeeded = 0;

	if (((ext_port = SampleUtil_GetFirstDocumentItem(ca_event->ActionRequest, "NewExternalPort")) &&
		(proto = SampleUtil_GetFirstDocumentItem(ca_event->ActionRequest, "NewProtocol"))))
	{

		prt = getProtoNum(proto);
		if ((prt == 6) || (prt == 17))
		{
			result=m_list.PortMapDelete(proto, atoi(ext_port));
			if (result==1)
			{
				syslog(LOG_DEBUG, "DeletePortMap: Proto:%s Port:%s\n",proto, ext_port);
				sprintf(num,"%d",m_list.m_pmap.size());
				PropSet= UpnpCreatePropertySet(1,"PortMappingNumberOfEntries", num);
				UpnpNotifyExt(device_handle, ca_event->DevUDN,ca_event->ServiceID,PropSet);
				UpnpDocument_free(PropSet);
				action_succeeded = 1;
			}
			else
			{
				syslog(LOG_DEBUG, "Failure in GateDeviceDeletePortMapping: DeletePortMap: Proto:%s Port:%s\n",proto, ext_port);
				ca_event->ErrCode = 714;
				strcpy(ca_event->ErrStr, "NoSuchEntryInArray");
				ca_event->ActionResult = NULL;
			}
		}
		else
		{
			syslog(LOG_DEBUG, "Failure in GateDeviceDeletePortMapping: Invalid NewProtocol=%s\n",proto);
			ca_event->ErrCode = 402;
                        strcpy(ca_event->ErrStr, "Invalid Args");
                        ca_event->ActionResult = NULL;
		}
	}
	else
	{
	        syslog(LOG_DEBUG, "Failiure in GateDeviceDeletePortMapping: Invalid Arguments!");
       	        ca_event->ErrCode = 402;
       	        strcpy(ca_event->ErrStr, "Invalid Args");
	        ca_event->ActionResult = NULL;
	}

	if (action_succeeded)
	{
		ca_event->ErrCode = UPNP_E_SUCCESS;
		sprintf(result_str, "<u:%sResponse xmlns:u=\"%s\">\n%s\n</u:%sResponse>",
			ca_event->ActionName, "urn:schemas-upnp-org:service:WANIPConnection:1", "", ca_event->ActionName);
		ca_event->ActionResult = UpnpParse_Buffer(result_str);
	}

	if (ext_port) delete [] ext_port;
	if (proto) delete [] proto;
	return(ca_event->ErrCode);
}


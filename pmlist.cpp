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

#include "pmlist.h"
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

PortMapList::PortMapList()
{

}

PortMapList::~PortMapList()
{

	for (list<PortMap *>::iterator itr = m_pmap.begin(); itr != m_pmap.end(); itr++)
        {
		delPacketFilter((*itr)->m_PortMappingProtocol, (*itr)->m_RemoteHost,
				(*itr)->m_ExternalIP, (*itr)->m_ExternalPort);
                delPortForward((*itr)->m_PortMappingProtocol, (*itr)->m_ExternalIP,
                                (*itr)->m_ExternalPort, (*itr)->m_InternalClient,
                                (*itr)->m_InternalPort);

		delete (*itr);
        }
	m_pmap.clear();
}

int PortMapList::PortMapAdd(char *RemoteHost, char *Proto, char *ExtIP, int ExtPort,
		char *IntIP, int IntPort, int Enabled, char *Desc, int LeaseDuration)
{

	int fd_socket, fd_proto;
	struct sockaddr_in addr;
	PortMap *temp;
	
	if ((strcmp(Proto,"TCP") == 0) || (strcmp(Proto,"tcp") == 0))
		fd_proto = SOCK_STREAM;
	else
		fd_proto = SOCK_DGRAM;
	
	if ((fd_socket = socket(AF_INET,fd_proto, 0)) == -1)
		syslog(LOG_DEBUG, "Socket Error");
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(ExtPort);
	if (bind(fd_socket, (struct sockaddr *) &addr, sizeof(addr)) == -1)
	{
		close(fd_socket);
		syslog(LOG_DEBUG,"Error binding socket");
		return (718);
	}
	close (fd_socket);
	
	temp = new PortMap;
	
	if (RemoteHost)
        {
                temp->m_RemoteHost = new char[strlen(RemoteHost)+1];
                strcpy(temp->m_RemoteHost, RemoteHost);
        }
        else temp->m_RemoteHost = NULL;

        if (Proto)
	{
		temp->m_PortMappingProtocol = new char[strlen(Proto)+1];
		strcpy(temp->m_PortMappingProtocol, Proto);
	}
	else temp->m_PortMappingProtocol = NULL;

        if (ExtIP)
        {
                temp->m_ExternalIP = new char[strlen(ExtIP)+1];
                strcpy(temp->m_ExternalIP, ExtIP);
        }
        else temp->m_ExternalIP = NULL;

        temp->m_ExternalPort = ExtPort;

        if (IntIP)
        {
                temp->m_InternalClient = new char[strlen(IntIP)+1];
                strcpy(temp->m_InternalClient, IntIP);
        }
        else temp->m_InternalClient = NULL;

        temp->m_InternalPort = IntPort;

        if (Desc)
        {
                temp->m_PortMappingDescription = new char[strlen(Desc)+1];
                strcpy(temp->m_PortMappingDescription, Desc);
        }
        else temp->m_PortMappingDescription = NULL;

        temp->m_PortMappingLeaseDuration = LeaseDuration;

	m_pmap.push_back(temp);
	
	addPacketFilter(Proto, RemoteHost, ExtIP, ExtPort, Enabled, Desc);
	addPortForward(Proto, ExtIP, ExtPort, IntIP, IntPort, Enabled, Desc);
	
	return (1);
}


int PortMapList::PortMapDelete(char *Proto, int ExtPort)
{
	
	for (list<PortMap *>::iterator itr = m_pmap.begin(); itr != m_pmap.end(); itr++)
	{
		if (( strcmp((*itr)->m_PortMappingProtocol,Proto) == 0 ) && ((*itr)->m_ExternalPort == ExtPort))
		{
			delPacketFilter((*itr)->m_PortMappingProtocol, (*itr)->m_RemoteHost,
					(*itr)->m_ExternalIP, (*itr)->m_ExternalPort);
			delPortForward((*itr)->m_PortMappingProtocol, (*itr)->m_ExternalIP,
					(*itr)->m_ExternalPort, (*itr)->m_InternalClient,
					(*itr)->m_InternalPort);
			delete (*itr);
			m_pmap.erase(itr);
			return (1);
		}
	}
	return 0;
}



int PortMapList::addPacketFilter(char *Proto, char *SrcIP, char *DestIP, 
	int DestPort,int Enabled, char *Desc)
{

	/*
	static char *IpAny = "0/0";
	char prt[4];
	int ret=0;
	
	if (SrcIP == NULL) 
		SrcIP = IpAny;
	if (Proto==6)
		strcpy(prt, "tcp");
	else
		strcpy(prt, "udp");
	
	// sprintf(command, "/sbin/ipchains -I upnp -j ACCEPT -p %s -s %s -d %s %d",
	// prt, SrcIP, DesIP, DestPort);
	
	ret = 1;
	*/
	return (1);
}

int PortMapList::addPortForward(char *Proto, char *ExtIP, int ExtPort, 
	char *IntIP,int IntPort, int Enabled, char *Desc)
{
	char command[255];

	sprintf(command,"/usr/sbin/iptables -t nat -A PREROUTING -p %s -d %s --dport %d -j DNAT --to %s:%d", Proto, ExtIP, ExtPort, IntIP, IntPort);
	system(command);

	return (1);
}

int PortMapList::delPacketFilter(char *Proto, char *SrcIP, char *DestIP, 
	int DestPort)
{
	/*
	static char *IpAny = "0/0";
	char prt[4];
	int ret=0;

	if (SrcIP == NULL) 
		SrcIP = IpAny;
	if (Proto==6)
		strcpy(prt, "tcp");
	else
		strcpy(prt, "udp");

	sprintf(command,"/sbin/ipchains -D upnp -j ACCEPT -p %s -s %s -d %s %d"
	, prt, SrcIP, DestIP, DestPort);
	  doCommand(command
	
	ret = 1;
	*/
	return(1);
	
}

int PortMapList::delPortForward(char *Proto, char *ExtIP, int ExtPort, 
	char* IntIP, int IntPort)
{
	char command[255];
	

	sprintf(command, "/usr/sbin/iptables -t nat -D PREROUTING -p %s -d %s --dport %d -j DNAT --to %s:%d", Proto, ExtIP, ExtPort, IntIP, IntPort);
	system(command);	
	return (1);
}


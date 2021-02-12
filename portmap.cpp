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

#include "portmap.h"
#include <string.h>

PortMap::PortMap ()
{

}

PortMap::~PortMap ()
{
        if (m_RemoteHost)
                delete [] m_RemoteHost;
        if (m_ExternalIP)
                delete [] m_ExternalIP;
        if (m_InternalClient)
                delete [] m_InternalClient;
        if (m_PortMappingDescription)
                delete [] m_PortMappingDescription;
	if (m_PortMappingProtocol)
		delete [] m_PortMappingProtocol;
}

PortMap::PortMap(char *RemoteHost, char *Proto, char *ExtIP, int ExtPort, char *IntIP, int IntPort, int Enabled, char *Desc, int LeaseDuration)
{
	if (RemoteHost)
        {
                m_RemoteHost = new char[strlen(RemoteHost)+1];
                strcpy(m_RemoteHost, RemoteHost);
        }
        else m_RemoteHost = NULL;

        if (Proto)
        {
                m_PortMappingProtocol = new char[strlen(Proto)+1];
                strcpy(m_PortMappingProtocol, Proto);
        }
        else m_PortMappingProtocol = NULL;

        if (ExtIP)
        {
                m_ExternalIP = new char[strlen(ExtIP)+1];
                strcpy(m_ExternalIP, ExtIP);
        }
        else m_ExternalIP = NULL;

        m_ExternalPort = ExtPort;

        if (IntIP)
        {
                m_InternalClient = new char[strlen(IntIP)+1];
                strcpy(m_InternalClient, IntIP);
        }
        else m_InternalClient = NULL;

        m_InternalPort = IntPort;

        if (Desc)
        {
                m_PortMappingDescription = new char[strlen(Desc)+1];
                strcpy(m_PortMappingDescription, Desc);
        }
        else m_PortMappingDescription = NULL;

        m_PortMappingLeaseDuration = LeaseDuration;
}

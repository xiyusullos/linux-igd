///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000 Intel Corporation 
// All rights reserved. 
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met: 
//
// * Redistributions of source code must retain the above copyright notice, 
// this list of conditions and the following disclaimer. 
// * Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation 
// and/or other materials provided with the distribution. 
// * Neither name of Intel Corporation nor the names of its contributors 
// may be used to endorse or promote products derived from this software 
// without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////
//
// $Revision: 1.1.1.1 $
// $Date: 2002/12/13 22:16:30 $
//


#include "sample_util.h"
#include "string.h"


/********************************************************************************
 * SampleUtil_GetElementValue
 *
 * Description: 
 *       Given a DOM node such as <Channel>11</Channel>, this routine
 *       extracts the value (e.g., 11) from the node and returns it as 
 *       a string.
 *
 * Parameters:
 *   node -- The DOM node from which to extract the value
 *
 ********************************************************************************/
char* SampleUtil_GetElementValue(Upnp_Node node)
{
  Upnp_Node child = UpnpNode_getFirstChild(node);
  Upnp_DOMException err;
  char * temp;

 
  if ( (child!=0) && (UpnpNode_getNodeType(child)==TEXT_NODE))
    {
      temp= (char *) UpnpNode_getNodeValue(child,&err);
      UpnpNode_free(child);
     
      return temp;
    }
  else
    {
      
    return NULL;
    }
}

/********************************************************************************
 * SampleUtil_GetFirstServiceList
 *
 * Description: 
 *       Given a DOM node representing a UPnP Device Description Document,
 *       this routine parses the document and finds the first service list
 *       (i.e., the service list for the root device).  The service list
 *       is returned as a DOM node list.
 *
 * Parameters:
 *   node -- The DOM node from which to extract the service list
 *
 ********************************************************************************/
Upnp_NodeList SampleUtil_GetFirstServiceList(Upnp_Node node) 
{
    Upnp_NodeList ServiceList=NULL;
    Upnp_NodeList servlistnodelist=NULL;
    Upnp_Node servlistnode=NULL;
	
    servlistnodelist = UpnpDocument_getElementsByTagName(node, "serviceList");
    if(servlistnodelist && UpnpNodeList_getLength(servlistnodelist)) {

	/* we only care about the first service list, from the root device */
	servlistnode = UpnpNodeList_item(servlistnodelist, 0);

	/* create as list of DOM nodes */
	ServiceList = UpnpElement_getElementsByTagName(servlistnode, "service");
    }

    if (servlistnodelist) UpnpNodeList_free(servlistnodelist);
    if (servlistnode) UpnpNode_free(servlistnode);

    return ServiceList;
}

/********************************************************************************
 * SampleUtil_GetFirstDocumentItem
 *
 * Description: 
 *       Given a DOM node, this routine searches for the first element
 *       named by the input string item, and returns its value as a string.
 *
 * Parameters:
 *   node -- The DOM node from which to extract the value
 *   item -- The item to search for
 *
 ********************************************************************************/
char* SampleUtil_GetFirstDocumentItem(Upnp_Node node, char *item) 
{
    Upnp_NodeList NodeList=NULL;
    Upnp_Node textNode=NULL;
    Upnp_Node tmpNode=NULL;
    Upnp_DOMException err; 
    char *ret=NULL;
    int len;
	
    NodeList = UpnpDocument_getElementsByTagName(node, item);
    if (NodeList == NULL) {
	printf("Error finding %s in XML Node\n", item);
    } else {
	if ((tmpNode = UpnpNodeList_item(NodeList, 0)) == NULL) {
	    printf("Error finding %s value in XML Node\n", item);
	} else {
	    textNode = UpnpNode_getFirstChild(tmpNode);
	    len = strlen(UpnpNode_getNodeValue(textNode, &err));
	    if (err != NO_ERR) {
		printf("Error getting node value for %s in XML Node\n", item);
		if (NodeList) UpnpNodeList_free(NodeList);
		if (tmpNode) UpnpNode_free(tmpNode);
		if (textNode) UpnpNode_free(textNode);
		return ret;
	    }
	    ret = new char[len+1]; //(char *) malloc(len+1);
	    if (!ret) {
		printf("Error allocating memory for %s in XML Node\n", item);
	    } else {
		strcpy(ret, UpnpNode_getNodeValue(textNode, &err));
	    }
	}
    }
    if (NodeList) UpnpNodeList_free(NodeList);
    if (tmpNode) UpnpNode_free(tmpNode);
    if (textNode) UpnpNode_free(textNode);
    return ret;
}

/********************************************************************************
 * SampleUtil_GetFirstElementItem
 *
 * Description: 
 *       Given a DOM element, this routine searches for the first element
 *       named by the input string item, and returns its value as a string.
 *
 * Parameters:
 *   node -- The DOM node from which to extract the value
 *   item -- The item to search for
 *
 ********************************************************************************/
char* SampleUtil_GetFirstElementItem(Upnp_Element node, char *item) 
{
    Upnp_NodeList NodeList=NULL;
    Upnp_Node textNode=NULL;
    Upnp_Node tmpNode=NULL;
    Upnp_DOMException err; 
    char *ret=NULL;
    int len;
	
    NodeList = UpnpElement_getElementsByTagName(node, item);
    if (NodeList == NULL) {
	printf("Error finding %s in XML Node\n", item);
    } else {
	if ((tmpNode = UpnpNodeList_item(NodeList, 0)) == NULL) {
	    printf("Error finding %s value in XML Node\n", item);
	} else {
	    textNode = UpnpNode_getFirstChild(tmpNode);
	    len = strlen(UpnpNode_getNodeValue(textNode, &err));
	    if (err != NO_ERR) {
		printf("Error getting node value for %s in XML Node\n", item);
		if (NodeList) UpnpNodeList_free(NodeList);
		if (tmpNode) UpnpNode_free(tmpNode);
		if (textNode) UpnpNode_free(textNode);
		return ret;
	    }
	    ret = new char[len+1]; //(char *) malloc(len+1);
	    if (!ret) {
		printf("Error allocating memory for %s in XML Node\n", item);
	    } else {
		strcpy(ret, UpnpNode_getNodeValue(textNode, &err));
	    }
	}
    }
    if (NodeList) UpnpNodeList_free(NodeList);
    if (tmpNode) UpnpNode_free(tmpNode);
    if (textNode) UpnpNode_free(textNode);
    return ret;
}



/********************************************************************************
 * SampleUtil_PrintEventType
 *
 * Description: 
 *       Prints a callback event type as a string.
 *
 * Parameters:
 *   S -- The callback event
 *
 ********************************************************************************/
void SampleUtil_PrintEventType(Upnp_EventType S)
{
    switch(S) {

    case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
	printf("UPNP_DISCOVERY_ADVERTISEMENT_ALIVE\n");
	break;
    case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
	printf("UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE\n");
	break;
    case UPNP_DISCOVERY_SEARCH_RESULT:
	printf("UPNP_DISCOVERY_SEARCH_RESULT\n");
	break;
    case UPNP_DISCOVERY_SEARCH_TIMEOUT:
	printf("UPNP_DISCOVERY_SEARCH_TIMEOUT\n");
	break;


	/* SOAP Stuff */
    case UPNP_CONTROL_ACTION_REQUEST:
	printf("UPNP_CONTROL_ACTION_REQUEST\n");
	break;
    case UPNP_CONTROL_ACTION_COMPLETE:
	printf("UPNP_CONTROL_ACTION_COMPLETE\n");
	break;
    case UPNP_CONTROL_GET_VAR_REQUEST:
	printf("UPNP_CONTROL_GET_VAR_REQUEST\n");
	break;
    case UPNP_CONTROL_GET_VAR_COMPLETE:
	printf("UPNP_CONTROL_GET_VAR_COMPLETE\n");
	break;

	/* GENA Stuff */
    case UPNP_EVENT_SUBSCRIPTION_REQUEST:
	printf("UPNP_EVENT_SUBSCRIPTION_REQUEST\n");
	break;
    case UPNP_EVENT_RECEIVED:
	printf("UPNP_EVENT_RECEIVED\n");
	break;
    case UPNP_EVENT_RENEWAL_COMPLETE:
	printf("UPNP_EVENT_RENEWAL_COMPLETE\n");
	break;
    case UPNP_EVENT_SUBSCRIBE_COMPLETE:
	printf("UPNP_EVENT_SUBSCRIBE_COMPLETE\n");
	break;
    case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
	printf("UPNP_EVENT_UNSUBSCRIBE_COMPLETE\n");
	break;

    case UPNP_EVENT_AUTORENEWAL_FAILED:
	printf("UPNP_EVENT_AUTORENEWAL_FAILED\n");
	break;
    case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
	printf("UPNP_EVENT_SUBSCRIPTION_EXPIRED\n");
	break;

    }
}

/********************************************************************************
 * SampleUtil_PrintEvent
 *
 * Description: 
 *       Prints callback event structure details.
 *
 * Parameters:
 *   EventType -- The type of callback event
 *   Event -- The callback event structure
 *
 ********************************************************************************/
int SampleUtil_PrintEvent(Upnp_EventType EventType, 
	       void *Event)
{

    printf("\n\n\n======================================================================\n");
    printf("----------------------------------------------------------------------\n");
    SampleUtil_PrintEventType(EventType);
  
    switch ( EventType) {
      
	/* SSDP Stuff */
    case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
    case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
    case UPNP_DISCOVERY_SEARCH_RESULT:
    {
	struct Upnp_Discovery *d_event = (struct Upnp_Discovery * ) Event;
        
	printf("ErrCode     =  %d\n",d_event->ErrCode);
	printf("Expires     =  %d\n",d_event->Expires);
	printf("DeviceId    =  %s\n",d_event->DeviceId); 
	printf("DeviceType  =  %s\n",d_event->DeviceType);
	printf("ServiceType =  %s\n",d_event->ServiceType);
	printf("ServiceVer  =  %s\n",d_event->ServiceVer);
	printf("Location    =  %s\n",d_event->Location);
	printf("OS          =  %s\n",d_event->Os);
	printf("Ext         =  %s\n",d_event->Ext);
	
    }
    break;
      
    case UPNP_DISCOVERY_SEARCH_TIMEOUT:
	// Nothing to print out here
	break;

    /* SOAP Stuff */
    case UPNP_CONTROL_ACTION_REQUEST:
    {
	struct Upnp_Action_Request *a_event = (struct Upnp_Action_Request * ) Event;
	char *xmlbuff=NULL;
        
	printf("ErrCode     =  %d\n",a_event->ErrCode);
	printf("ErrStr      =  %s\n",a_event->ErrStr); 
	printf("ActionName  =  %s\n",a_event->ActionName); 
	printf("UDN         =  %s\n",a_event->DevUDN);
	printf("ServiceID   =  %s\n",a_event->ServiceID);
	if (a_event->ActionRequest) {
	    xmlbuff = UpnpNewPrintDocument(a_event->ActionRequest);
	    if (xmlbuff) printf("ActRequest  =  %s\n",xmlbuff);
	    if (xmlbuff) free(xmlbuff);
	    xmlbuff=NULL;
	} else {
	    printf("ActRequest  =  (null)\n");
	}
	if (a_event->ActionResult) {
	    xmlbuff = UpnpNewPrintDocument(a_event->ActionResult);
	    if (xmlbuff) printf("ActResult   =  %s\n",xmlbuff);
	    if (xmlbuff) free(xmlbuff);
	    xmlbuff=NULL;
	} else {
	    printf("ActResult   =  (null)\n");
	}
    }
    break;
      
    case UPNP_CONTROL_ACTION_COMPLETE:
    {
	struct Upnp_Action_Complete *a_event = (struct Upnp_Action_Complete * ) Event;
	char *xmlbuff=NULL;
        
	printf("ErrCode     =  %d\n",a_event->ErrCode);
	printf("CtrlUrl     =  %s\n",a_event->CtrlUrl);
	if (a_event->ActionRequest) {
	    xmlbuff = UpnpNewPrintDocument(a_event->ActionRequest);
	    if (xmlbuff) printf("ActRequest  =  %s\n",xmlbuff);
	    if (xmlbuff) free(xmlbuff);
	    xmlbuff=NULL;
	} else {
	    printf("ActRequest  =  (null)\n");
	}
	if (a_event->ActionResult) {
	    xmlbuff = UpnpNewPrintDocument(a_event->ActionResult);
	    if (xmlbuff) printf("ActResult   =  %s\n",xmlbuff);
	    if (xmlbuff) free(xmlbuff);
	    xmlbuff=NULL;
	} else {
	    printf("ActResult   =  (null)\n");
	}
    }
    break;
      
    case UPNP_CONTROL_GET_VAR_REQUEST:
    {
	struct Upnp_State_Var_Request *sv_event = (struct Upnp_State_Var_Request * ) Event;
        
	printf("ErrCode     =  %d\n",sv_event->ErrCode);
	printf("ErrStr      =  %s\n",sv_event->ErrStr); 
	printf("UDN         =  %s\n",sv_event->DevUDN); 
	printf("ServiceID   =  %s\n",sv_event->ServiceID); 
	printf("StateVarName=  %s\n",sv_event->StateVarName); 
	printf("CurrentVal  =  %s\n",sv_event->CurrentVal);
    }
    break;
      
    case UPNP_CONTROL_GET_VAR_COMPLETE:
    {
	struct Upnp_State_Var_Complete *sv_event = (struct Upnp_State_Var_Complete * ) Event;
        
	printf("ErrCode     =  %d\n",sv_event->ErrCode);
	printf("CtrlUrl     =  %s\n",sv_event->CtrlUrl); 
	printf("StateVarName=  %s\n",sv_event->StateVarName); 
	printf("CurrentVal  =  %s\n",sv_event->CurrentVal);
    }
    break;
      
    /* GENA Stuff */
    case UPNP_EVENT_SUBSCRIPTION_REQUEST:
    {
	struct Upnp_Subscription_Request *sr_event = (struct Upnp_Subscription_Request * ) Event;
        
	printf("ServiceID   =  %s\n",sr_event->ServiceId);
	printf("UDN         =  %s\n",sr_event->UDN); 
	printf("SID         =  %s\n",sr_event->Sid);
    }
    break;
      
    case UPNP_EVENT_RECEIVED:
    {
	struct Upnp_Event *e_event = (struct Upnp_Event * ) Event;
	char *xmlbuff=NULL;
        
	printf("SID         =  %s\n",e_event->Sid);
	printf("EventKey    =  %d\n",e_event->EventKey);
	xmlbuff = UpnpNewPrintDocument(e_event->ChangedVariables);
	printf("ChangedVars =  %s\n",xmlbuff);
	free(xmlbuff);
	xmlbuff=NULL;
    }
    break;

    case UPNP_EVENT_RENEWAL_COMPLETE:
    {
	struct Upnp_Event_Subscribe *es_event = (struct Upnp_Event_Subscribe * ) Event;
        
	printf("SID         =  %s\n",es_event->Sid);
	printf("ErrCode     =  %d\n",es_event->ErrCode);
	printf("TimeOut     =  %d\n",es_event->TimeOut);
    }
    break;

    case UPNP_EVENT_SUBSCRIBE_COMPLETE:
    case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
    {
	struct Upnp_Event_Subscribe *es_event = (struct Upnp_Event_Subscribe * ) Event;
        
	printf("SID         =  %s\n",es_event->Sid);
	printf("ErrCode     =  %d\n",es_event->ErrCode);
	printf("PublisherURL=  %s\n",es_event->PublisherUrl);
	printf("TimeOut     =  %d\n",es_event->TimeOut);
    }
    break;

    case UPNP_EVENT_AUTORENEWAL_FAILED:
    case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
    {
	struct Upnp_Event_Subscribe *es_event = (struct Upnp_Event_Subscribe * ) Event;
        
	printf("SID         =  %s\n",es_event->Sid);
	printf("ErrCode     =  %d\n",es_event->ErrCode);
	printf("PublisherURL=  %s\n",es_event->PublisherUrl);
	printf("TimeOut     =  %d\n",es_event->TimeOut);
    }
    break;



    }
    printf("----------------------------------------------------------------------\n");
    printf("======================================================================\n\n\n\n");

    return(0);
}




/********************************************************************************
 * SampleUtil_FindAndParseService
 *
 * Description: 
 *       This routine finds the first occurance of a service in a DOM representation
 *       of a description document and parses it.  Note that this function currently
 *       assumes that the eventURL and controlURL values in the service definitions
 *       are full URLs.  Relative URLs are not handled here.
 *
 * Parameters:
 *   DescDoc -- The DOM description document
 *   location -- The location of the description document
 *   serviceSearchType -- The type of service to search for
 *   serviceId -- OUT -- The service ID
 *   eventURL -- OUT -- The event URL for the service
 *   controlURL -- OUT -- The control URL for the service
 *
 ********************************************************************************/
int SampleUtil_FindAndParseService (Upnp_Document DescDoc, char* location, char *serviceSearchType, char **serviceId, char **eventURL, char **controlURL) 
{
    int i, length, found=0;
    int ret;
    char *serviceType=NULL;
    char *baseURL=NULL;
    char *base;
    char *relcontrolURL=NULL, *releventURL=NULL;
    Upnp_NodeList serviceList=NULL;
    Upnp_Node service=NULL;
  
    baseURL = SampleUtil_GetFirstDocumentItem(DescDoc, "URLBase");

    if (baseURL) 
	base = baseURL;
    else
	base = location;

    serviceList = SampleUtil_GetFirstServiceList(DescDoc);
    length = UpnpNodeList_getLength(serviceList);
    for (i=0;i<length;i++) { 
	service = UpnpNodeList_item(serviceList, i);
	serviceType = SampleUtil_GetFirstElementItem(service, "serviceType");
	if (strcmp(serviceType, serviceSearchType) == 0) {
	    printf("Found service: %s\n", serviceType);
	    *serviceId = SampleUtil_GetFirstElementItem(service, "serviceId");

	    relcontrolURL = SampleUtil_GetFirstElementItem(service, "controlURL");
	    releventURL = SampleUtil_GetFirstElementItem(service, "eventSubURL");
	    
	    *controlURL = new char[strlen(base)+strlen(relcontrolURL)+1]; //(char*)(malloc(strlen(base) + strlen(relcontrolURL) + 1));
	    if (*controlURL) {
		ret = UpnpResolveURL(base, relcontrolURL, *controlURL);
		if (ret!=UPNP_E_SUCCESS)
		    printf("Error generating controlURL from %s + %s\n", base, relcontrolURL);
	    }
		
	    *eventURL = new char[strlen(base)+strlen(releventURL)+1]; //(char*)(malloc(strlen(base) + strlen(releventURL) + 1));
	    if (*eventURL) {
		ret = UpnpResolveURL(base, releventURL, *eventURL);
		if (ret!=UPNP_E_SUCCESS)
		    printf("Error generating eventURL from %s + %s\n", base, releventURL);
	    }

	    if (relcontrolURL) free(relcontrolURL);
	    if (releventURL) free(releventURL);
	    relcontrolURL = releventURL = NULL;
		
	    found=1;
	    break;
	}
	if (service) UpnpNode_free(service);
	service=NULL;
	if (serviceType) free(serviceType);
	serviceType=NULL;
    }

    if (service) UpnpNode_free(service);
    if (serviceType) free(serviceType);
    if (serviceList) UpnpNodeList_free(serviceList);
    if (baseURL) free(baseURL);

    return(found);
}



















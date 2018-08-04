/* soapws_USCOREaccountServiceSoapProxy.cpp
   Generated by gSOAP 2.7.15 from Services.h
   Copyright(C) 2000-2009, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
*/

#include "soapws_USCOREaccountServiceSoapProxy.h"

ws_USCOREaccountServiceSoapProxy::ws_USCOREaccountServiceSoapProxy()
{	ws_USCOREaccountServiceSoapProxy_init(SOAP_IO_DEFAULT, SOAP_IO_DEFAULT);
}

ws_USCOREaccountServiceSoapProxy::ws_USCOREaccountServiceSoapProxy(const struct soap &_soap) :soap(_soap)
{ }

ws_USCOREaccountServiceSoapProxy::ws_USCOREaccountServiceSoapProxy(soap_mode iomode)
{	ws_USCOREaccountServiceSoapProxy_init(iomode, iomode);
}

ws_USCOREaccountServiceSoapProxy::ws_USCOREaccountServiceSoapProxy(soap_mode imode, soap_mode omode)
{	ws_USCOREaccountServiceSoapProxy_init(imode, omode);
}

void ws_USCOREaccountServiceSoapProxy::ws_USCOREaccountServiceSoapProxy_init(soap_mode imode, soap_mode omode)
{	soap_imode(this, imode);
	soap_omode(this, omode);
	soap_endpoint = NULL;
	static const struct Namespace namespaces[] =
{
	{"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
	{"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
	{"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
	{"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
	{"ns1", "urn:hpccsystems:ws:ws_account", NULL, NULL},
	{"ns2", "urn:hpccsystems:ws:wsattributes", NULL, NULL},
	{"ns3", "urn:hpccsystems:ws:wsdfu", NULL, NULL},
	{"ns4", "urn:hpccsystems:ws:wssmc", NULL, NULL},
	{"ns5", "urn:hpccsystems:ws:wstopology", NULL, NULL},
	{"ns6", "urn:hpccsystems:ws:wsworkunits", NULL, NULL},
	{NULL, NULL, NULL, NULL}
};
	if (!this->namespaces)
		this->namespaces = namespaces;
}

ws_USCOREaccountServiceSoapProxy::~ws_USCOREaccountServiceSoapProxy()
{ }

void ws_USCOREaccountServiceSoapProxy::destroy()
{	soap_destroy(this);
	soap_end(this);
}

void ws_USCOREaccountServiceSoapProxy::soap_noheader()
{	header = NULL;
}

const SOAP_ENV__Fault *ws_USCOREaccountServiceSoapProxy::soap_fault()
{	return this->fault;
}

const char *ws_USCOREaccountServiceSoapProxy::soap_fault_string()
{	return *soap_faultstring(this);
}

const char *ws_USCOREaccountServiceSoapProxy::soap_fault_detail()
{	return *soap_faultdetail(this);
}

int ws_USCOREaccountServiceSoapProxy::soap_close_socket()
{	return soap_closesock(this);
}

void ws_USCOREaccountServiceSoapProxy::soap_print_fault(FILE *fd)
{	::soap_print_fault(this, fd);
}

#ifndef WITH_LEAN
void ws_USCOREaccountServiceSoapProxy::soap_stream_fault(std::ostream& os)
{	::soap_stream_fault(this, os);
}

char *ws_USCOREaccountServiceSoapProxy::soap_sprint_fault(char *buf, size_t len)
{	return ::soap_sprint_fault(this, buf, len);
}
#endif

int ws_USCOREaccountServiceSoapProxy::MyAccount(_ns1__MyAccountRequest *ns1__MyAccountRequest, _ns1__MyAccountResponse *ns1__MyAccountResponse)
{	struct soap *soap = this;
	struct __ns1__MyAccount soap_tmp___ns1__MyAccount;
	const char *soap_action = NULL;
	if (!soap_endpoint)
		soap_endpoint = "http://192.168.3.22:8010/ws_account?ver_=1.03";
	soap_action = "ws_account/MyAccount?ver_=1.03";
	soap->encodingStyle = NULL;
	soap_tmp___ns1__MyAccount.ns1__MyAccountRequest = ns1__MyAccountRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__MyAccount(soap, &soap_tmp___ns1__MyAccount);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___ns1__MyAccount(soap, &soap_tmp___ns1__MyAccount, "-ns1:MyAccount", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__MyAccount(soap, &soap_tmp___ns1__MyAccount, "-ns1:MyAccount", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__MyAccountResponse)
		return soap_closesock(soap);
	ns1__MyAccountResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__MyAccountResponse->soap_get(soap, "ns1:MyAccountResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

int ws_USCOREaccountServiceSoapProxy::UpdateUser(_ns1__UpdateUserRequest *ns1__UpdateUserRequest, _ns1__UpdateUserResponse *ns1__UpdateUserResponse)
{	struct soap *soap = this;
	struct __ns1__UpdateUser soap_tmp___ns1__UpdateUser;
	const char *soap_action = NULL;
	if (!soap_endpoint)
		soap_endpoint = "http://192.168.3.22:8010/ws_account?ver_=1.03";
	soap_action = "ws_account/UpdateUser?ver_=1.03";
	soap->encodingStyle = NULL;
	soap_tmp___ns1__UpdateUser.ns1__UpdateUserRequest = ns1__UpdateUserRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__UpdateUser(soap, &soap_tmp___ns1__UpdateUser);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___ns1__UpdateUser(soap, &soap_tmp___ns1__UpdateUser, "-ns1:UpdateUser", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__UpdateUser(soap, &soap_tmp___ns1__UpdateUser, "-ns1:UpdateUser", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__UpdateUserResponse)
		return soap_closesock(soap);
	ns1__UpdateUserResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__UpdateUserResponse->soap_get(soap, "ns1:UpdateUserResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

int ws_USCOREaccountServiceSoapProxy::UpdateUserInput(_ns1__UpdateUserInputRequest *ns1__UpdateUserInputRequest, _ns1__UpdateUserInputResponse *ns1__UpdateUserInputResponse)
{	struct soap *soap = this;
	struct __ns1__UpdateUserInput soap_tmp___ns1__UpdateUserInput;
	const char *soap_action = NULL;
	if (!soap_endpoint)
		soap_endpoint = "http://192.168.3.22:8010/ws_account?ver_=1.03";
	soap_action = "ws_account/UpdateUserInput?ver_=1.03";
	soap->encodingStyle = NULL;
	soap_tmp___ns1__UpdateUserInput.ns1__UpdateUserInputRequest = ns1__UpdateUserInputRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__UpdateUserInput(soap, &soap_tmp___ns1__UpdateUserInput);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___ns1__UpdateUserInput(soap, &soap_tmp___ns1__UpdateUserInput, "-ns1:UpdateUserInput", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__UpdateUserInput(soap, &soap_tmp___ns1__UpdateUserInput, "-ns1:UpdateUserInput", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__UpdateUserInputResponse)
		return soap_closesock(soap);
	ns1__UpdateUserInputResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__UpdateUserInputResponse->soap_get(soap, "ns1:UpdateUserInputResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

int ws_USCOREaccountServiceSoapProxy::VerifyUser(_ns1__VerifyUserRequest *ns1__VerifyUserRequest, _ns1__VerifyUserResponse *ns1__VerifyUserResponse)
{	struct soap *soap = this;
	struct __ns1__VerifyUser soap_tmp___ns1__VerifyUser;
	const char *soap_action = NULL;
	if (!soap_endpoint)
		soap_endpoint = "http://192.168.3.22:8010/ws_account?ver_=1.03";
	soap_action = "ws_account/VerifyUser?ver_=1.03";
	soap->encodingStyle = NULL;
	soap_tmp___ns1__VerifyUser.ns1__VerifyUserRequest = ns1__VerifyUserRequest;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___ns1__VerifyUser(soap, &soap_tmp___ns1__VerifyUser);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___ns1__VerifyUser(soap, &soap_tmp___ns1__VerifyUser, "-ns1:VerifyUser", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___ns1__VerifyUser(soap, &soap_tmp___ns1__VerifyUser, "-ns1:VerifyUser", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!ns1__VerifyUserResponse)
		return soap_closesock(soap);
	ns1__VerifyUserResponse->soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	ns1__VerifyUserResponse->soap_get(soap, "ns1:VerifyUserResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}
/* End of client proxy code */

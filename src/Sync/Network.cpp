#include "Network.h"
#include <stdexcept>
#include <string.h>
//LCOV_EXCL_START
#include <MessageCenter.h>
#include <strstream>
#include <glibmm/i18n.h>

INetwork::~INetwork()
{

}

size_t receive_data(void *ptr, size_t size, size_t nmemb, Network *caller)
{
	caller->receiveData(ptr, size, nmemb);
	return size * nmemb;
}

size_t send_data(void *ptr, size_t size, size_t nmemb, Network *caller)
{
	return caller->sendData(ptr, size, nmemb);
}

Network::Network(std::shared_ptr<Utils::MessageCenter> mc)
{
	messageCenter = mc;
	curl_global_init(CURL_GLOBAL_ALL);
	curSendPosition = 0;
}

Network::~Network()
{
	curl_global_cleanup();
}

CURL* Network::init(bool ignoreCertificateErrors)
{
	CURL* curl = curl_easy_init();

	if (curl == nullptr)
	{
		throw new std::runtime_error("Unable to initialise network");
	}
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(curl, CURLOPT_PUT, 1L);
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receive_data);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, send_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(curl, CURLOPT_READDATA, this);

	if(ignoreCertificateErrors)
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	}

	return curl;
}

void Network::close(CURL* curl)
{
	curl_easy_cleanup(curl);
}

void Network::receiveData(void *ptr, size_t size, size_t nmemb)
{
	int length = size * nmemb;
	char *data = new char[length + 1];
	strncpy(data, (const char*) ptr, length);
	data[length] = 0;
	receivedData.append(data);
}

size_t Network::sendData(void *ptr, size_t size, size_t nmemb)
{
	size_t dataLength = dataToSend.size();
	size_t bufferLength = size * nmemb;
	size_t charactersToSend = dataLength - curSendPosition;
	if (charactersToSend > bufferLength)
	{
		charactersToSend = bufferLength;
	}

	strncpy((char*) ptr, &(dataToSend.c_str()[curSendPosition]), charactersToSend);
	curSendPosition += charactersToSend;
	return charactersToSend;
}

std::string Network::request(const std::string& url, std::string data, std::string username, std::string password,
		bool ignoreCertificateErrors)
{
	dataToSend = data;
	receivedData = "";
	curSendPosition = 0;
	CURL* curl = init(ignoreCertificateErrors);
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_USERPWD, (username + ":" + password).c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_BASIC);

	/* Perform the request, res will get the return code */
	CURLcode res = curl_easy_perform(curl);
	long httpCode = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

	/* Check for errors */
	if (res != CURLE_OK)
	{
		std::stringstream text;
		text << _("Failed connection to ");
		text << url << ":\n";
		text << curl_easy_strerror(res);
		Utils::Message message(Utils::ERROR_MESSAGE, _("Network error"), text.str());
		messageCenter->sendMessage(message);
	}
	else if (httpCode == 401)
	{
		std::stringstream text;
		text << _("Failed connection to ");
		text << url << ":\n";
		text << _("HTTP error ") << httpCode << " ";
		text << _("Username or password is wrong.");
		Utils::Message message(Utils::ERROR_MESSAGE, _("Network error"), text.str());
		messageCenter->sendMessage(message);
	}
	else if (httpCode != 200)
	{
		std::stringstream text;
		text << _("Failed connection to ");
		text << url << ":\n";
		text << _("HTTP error ") << httpCode << " ";
		text << curl_easy_strerror(res);
		Utils::Message message(Utils::ERROR_MESSAGE, _("Network error"), text.str());
		messageCenter->sendMessage(message);
	}
	fprintf(stderr, "%s", receivedData.c_str());
	close(curl);
	return receivedData;
}

//LCOV_EXCL_STOP
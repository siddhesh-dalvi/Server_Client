#pragma once
#ifndef HANDLER_H
#define HANDLER_H
#include <iostream>
#include <array>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/http_listener.h>              // HTTP server
#include <cpprest/json.h>                       // JSON library
#include <cpprest/uri.h>                        // URI library
//#include <cpprest/ws_client.h>                  // WebSocket client
#include <cpprest/containerstream.h>            // Async streams backed by STL containers
#include <cpprest/interopstream.h>              // Bridges for integrating Async streams with STL and WinRT streams
#include <cpprest/rawptrstream.h>               // Async streams backed by raw pointer to memory
#include <cpprest/producerconsumerstream.h>     // Async streams for producer consumer scenarios
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <thread>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;


class handler
{
public:
	handler();
	handler(utility::string_t url);
	virtual ~handler();

	pplx::task<void> open() { return m_listner.open(); }
	pplx::task<void> close() { return m_listner.close(); }

protected:

private:
	void handle_get(http_request message);
	void handle_post(http_request message);
	http_listener m_listner;
};

void respond(const http_request& request, const status_code& status, const json::value& response);
int create_client();
char* status_for_client(SOCKET ClientSocket, const::utility::string_t request_client, const::utility::string_t request_status);
#endif // HANDLER_H



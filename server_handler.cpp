#include "server_header.h"

// cpprest provides macros for all streams but std::clog in basic_types.h
#ifdef _UTF16_STRINGS
// On Windows, all strings are wide
#define uclog std::wclog
#endif

handler::handler() {}

handler::handler(utility::string_t url) :m_listner(url) {
	m_listner.support(methods::GET, std::bind(&handler::handle_get, this, std::placeholders::_1));
	m_listner.support(methods::POST, std::bind(&handler::handle_post, this, std::placeholders::_1));
	uclog << "In handler constructor" << endl;
}

handler::~handler() {}

void respond(const http_request& request, const status_code& status, const json::value& response) {
	json::value resp;
	resp[U("status")] = json::value::number(status);
	resp[U("response")] = response;
	request.reply(status, resp);
}

void handler::handle_get(http_request message) {
	//ucout <<"Client Message"<< message.to_string() << endl;
    int flag = 0;
	auto http_get_vars = uri::split_query(message.request_uri().query());

	auto found_client = http_get_vars.find(U("client"));
	auto found_start = http_get_vars.find(U("start"));

	if (found_client != end(http_get_vars) && found_start != end(http_get_vars)) {
		const::utility::string_t request_client = found_client->second;
		const::utility::string_t request_status = found_start->second;
		if (request_client == U("1") && request_status == U("ALL"))
		{
			uclog << U("Received request: ") << request_client << U(" with status: ") << request_status << endl;
			respond(message, status_codes::OK, json::value::string(U("Request received for CKIM: ") + request_client));
		}
	}
	else
	{
		json::value jsonObject;
		jsonObject[U("Data")][U("IPAddress")] = json::value::string(U("MKIM"));
		jsonObject[U("Data")][U("IPAddress2")] = json::value::string(U("CKIM1"));

		jsonObject[U("Data")][U("Port")][0] = json::value::string(U("BIM1"));
		jsonObject[U("Data")][U("Port")][1] = json::value::string(U("TRIM1"));
		jsonObject[U("Data")][U("Port")][2] = json::value::string(U("RIM1"));

		respond(message, status_codes::OK, jsonObject);
	}

	//message.reply(status_codes::OK, jsonObject);
	return;
}

void handler::handle_post(http_request message) {
	uclog << "In handle_post" << endl;
	message.extract_json()
		//Get the data field
		.then([](json::value jsonObject) {
		return jsonObject;
			})

		.then([=](json::value jsonObject) {
				json::value jsonUpdate;
				jsonUpdate[U("Data")][U("IPAddress")] = jsonObject[U("Data")][U("IPAddress")];
				jsonUpdate[U("Data")][U("IPAddress2")] = jsonObject[U("Data")][U("IPAddress2")];

				jsonUpdate[U("Data")][U("Port")][0] = jsonObject[U("Data")][U("Port")][0];
				jsonUpdate[U("Data")][U("Port")][1] = jsonObject[U("Data")][U("Port")][1];
				jsonUpdate[U("Data")][U("Port")][2] = jsonObject[U("Data")][U("Port")][2];

				// message.reply(status_codes::Created, jsonUpdate);

				respond(message, status_codes::Created, jsonUpdate);
			});
			return;
}

SOCKET create_server() {
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    cout << "Initialising Winsock..." << endl;
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    cout << "Initialised." << endl;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, "27015", &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    cout << "Socket created." << endl;
    // Setup the TCP listening socket
    iResult = ::bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    uclog << "Bind done" << endl;
    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    uclog << "Waiting for incoming connections..." << endl;

    return ListenSocket;
}
    
void accept_connection(SOCKET ListenSocket)
{
    SOCKET ClientSocket = INVALID_SOCKET;
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int iSendResult;
    int recvbuflen = DEFAULT_BUFLEN;
    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }

    // No longer need server socket
    closesocket(ListenSocket);

    // Receive until the peer shuts down the connection
    do {

        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            cout << "Data received : " << recvbuf << endl;
            //printf("Bytes received: %d\n", iResult);
            strcpy_s(recvbuf,"Test Paased");
            // Echo the buffer back to the sender
            cout << "Send Data to Client : " << endl;
            //cin >> recvbuf;
            iSendResult = send(ClientSocket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return;
            }
            //printf("Bytes sent: %d\n", iSendResult);
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return;
        }

    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

}
#include "server_header.h"

// cpprest provides macros for all streams but std::clog in basic_types.h
#ifdef _UTF16_STRINGS
// On Windows, all strings are wide
#define uclog std::wclog
#endif

SOCKET ConnectSocket = INVALID_SOCKET;
char status[50];

handler::handler() {}

handler::handler(utility::string_t url) :m_listner(url) {
	m_listner.support(methods::GET, std::bind(&handler::handle_get, this, std::placeholders::_1));
	m_listner.support(methods::POST, std::bind(&handler::handle_post, this, std::placeholders::_1));
}

handler::~handler() {}

void respond(const http_request& request, const status_code& status, const json::value& response) {
	json::value resp;
	resp[U("status")] = json::value::number(status);
	resp[U("response")] = response;
	request.reply(status, resp);
}

void handler::handle_get(http_request message) {
	int flag = 0;
	char* update;
	auto http_get_vars = uri::split_query(message.request_uri().query());

	auto found_client = http_get_vars.find(U("client"));
	auto found_start = http_get_vars.find(U("start"));

	if (found_client != end(http_get_vars) && found_start != end(http_get_vars)) {
		const::utility::string_t request_client = found_client->second;
		const::utility::string_t request_status = found_start->second;
		if (request_client == U("1"))
		{
			uclog << U("Received request: ") << request_client << U(" with status: ") << request_status << endl;
			update = status_for_client(ConnectSocket, request_client, request_status);
			//respond(message, status_codes::OK, json::value::string(U("Status received for CKIM: ") ));
			message.reply(status_codes::OK,update);
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

int create_client() {
	WSADATA wsaData;
	struct addrinfo* result = NULL,
				   * ptr = NULL,
				     hints;
	int iResult;    
	int recvbuflen = DEFAULT_BUFLEN;
	
	cout << "\nInitialising Winsock..." << endl;
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	cout << "Initialised.\n" << endl;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		cout << "Socket created." << endl;
		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			cout << "Connect failed with error: " << WSAGetLastError() << endl;
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
}

char* status_for_client(SOCKET ConnectSocket, const::utility::string_t request_client, const::utility::string_t request_status) {
	int iResult;
	if (request_client == U("1"))
	{
		if (request_status == U("1"))
		{
			strcpy_s(status, "start");
			uclog << "Sending status to client 1" << endl;
			iResult = send(ConnectSocket, status, (int)strlen(status), 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				strcpy_s(status, "Send failed with error");
				closesocket(ConnectSocket);
				WSACleanup();
				return status;
			}

			iResult = recv(ConnectSocket, status, (int)strlen(status), 0);
			if (iResult > 0) {
				return status;
			}
			else if (iResult == 0)
			{
				strcpy_s(status, "Connection closed in status_for_client RECV");
				return status;
			}
			else
			{ 
				strcpy_s(status, "recv failed with error in status_for_client RECV");
				return status;
			}
		}
		else
		{
			strcpy_s(status, "stops");
			uclog << "Sending status to client 1" << endl;
			iResult = send(ConnectSocket, status, (int)strlen(status), 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				strcpy_s(status, "Send failed with error");
				closesocket(ConnectSocket);
				WSACleanup();
				return status;
			}

			iResult = recv(ConnectSocket, status, (int)strlen(status), 0);
			if (iResult > 0) {
				return status;
			}
			else if (iResult == 0)
			{
				strcpy_s(status, "Connection closed");
				return status;
			}
			else
			{
				strcpy_s(status, "recv failed with error");
				return status;
			}
		}
	}
}
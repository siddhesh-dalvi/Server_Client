#include "client_header.h"

void printJson(json::value jsonObject) {
	ucout << jsonObject.serialize().c_str() << std::endl;
}

pplx::task<void> GetAll()
{
	http_client client(U("http://127.0.0.1:34568/params?client=1&start=ALL"));
	//http_client client(U("http://127.0.0.1:34568/"));
	http_request request(methods::GET);
	pplx::task<void> task = client.request(request)
		//Get the response
		.then([](http_response response) {
		//Check the status code
		if (response.status_code() != 200) {
			throw std::runtime_error("Returned " + std::to_string(response.status_code()));
		}

		//Convert the response body to json object
		return response.extract_json();
			})

		.then([](json::value jsonObject) {
				printJson(jsonObject);
			});

			try {
				task.wait();
			}
			catch (const std::exception& e) {
				printf("Error exception:%s\n", e.what());
			}
}

pplx::task<void> Post()
{
	auto postJson = pplx::create_task([]
		{
			json::value jsonObject;

			jsonObject[U("Data")][U("IPAddress")] = json::value::string(U("1.2.3.4"));
			jsonObject[U("Data")][U("IPAddress2")] = json::value::string(U("5.6.7.8"));

			jsonObject[U("Data")][U("Port")][0] = json::value::string(U("1234"));
			jsonObject[U("Data")][U("Port")][1] = json::value::string(U("5678"));
			jsonObject[U("Data")][U("Port")][2] = json::value::string(U("9123"));

			http_client client(U("http://127.0.0.1:34568/"));
			return	client.request(methods::POST, U(""),
				//uri_builder(U("api")).append_path(U("users")).to_string(),
				jsonObject.serialize(), U("application/json"));
		})
		.then([](http_response response) {
			// Check the status code.
			if (response.status_code() != 201) {
				throw std::runtime_error("Returned " + std::to_string(response.status_code()));
			}
			// Convert the response body to JSON object.
			return response.extract_json();
			})
			.then([](json::value jsonObject) {
				printJson(jsonObject);
				});

			try {
				postJson.wait();
			}
			catch (const std::exception& e) {
				printf("Error exception:%s\n", e.what());
			};
}
#pragma once
#include <iostream>
#include <array>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/uri.h>
#include <cpprest/json.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <thread>

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

void printJson(json::value dataObj);
pplx::task<void> GetAll();
pplx::task<void> Post();
//void GetAll();
//void Post();
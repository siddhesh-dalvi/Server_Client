#include "client_header.h"

int main() {
		/*while (true) {
			ucout << "In Client Get" << std::endl;
			pplx::task<void> get = GetAll();
			//get.wait();
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		}*/
	pplx::task<void> get = GetAll();
	pplx::task<void> post = Post();

	return 0;
}
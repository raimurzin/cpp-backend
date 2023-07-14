#include <boost/asio.hpp>
#include <array>
#include <iostream>
#include <string>
#include <string_view>

namespace net = boost::asio;

using namespace std::literals;

/*
	Отметим следующие отличия UDP от TCP:
		1. Для общения с разными клиентами можно использовать один и тот же сокет.
		   Так как соединение не устанавливается, это вполне законно.
		2. Данные приходят порциями, а не единым потоком. Можно не передавать конец строки:
		   размер данных будет известен. При этом нужно уложиться в максимальный размер полезных данных UDP — 65507 байт
		3. При каждой отправке данных нужно указывать endpoint.
*/

int main() {
    static const int port = 3333;
    static const size_t max_buffer_size = 1024;

    try {
	net::io_context io_context;
	net::ip::udp::socket socket(io_context, net::ip::udp::endpoint(net::ip::udp::v4(), port));

	//Запустим сервер в цикле, чтобы можно было работать со многими клиентами
	for (;;) {
            // Создаём буфер достаточного размера, чтобы вместить датаграмму.
            std::array<char, max_buffer_size> recv_buf;
	    net::ip::udp::endpoint remote_endpoint;

            // Получаем не только данные, но и endpoint клиента
            auto size = socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);

            std::cout << "Client said "sv << std::string_view(recv_buf.data(), size) << std::endl;

            // Отправляем ответ на полученный endpoint, игнорируя ошибку.
            // На этот раз не отправляем перевод строки: размер датаграммы будет получен автоматически.
            boost::system::error_code ignored_error;
            socket.send_to(boost::asio::buffer("Hello from UDP-server"sv), remote_endpoint, 0, ignored_error);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    /*
	Сервер может получить endpoint от клиента при получении датаграммы. Однако с использованием endpoint нужно быть осторожным:
	он будет просрочен довольно быстро. Стандартное время хранения адреса в таблице NAT для UDP — около минуты.
    */
}

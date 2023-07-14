#include <iostream>
#include <string>
#include <string_view>

#include <boost/asio.hpp>

namespace net = boost::asio;

int main(int args, char** argv) {
    /*
	Теперь подключаемся к серверу. Акцептор на этот раз не нужен, так как клиент не принимает
	подключений. Он отправляет запрос — и для этого используется сокет, инициализированный контекстом
    */

    static const int port = 3333;

    if (args != 2) {
        std::cout << "Usage: "sv << argv[0] << " <server IP>"sv << std::endl;
        return 1;
    }

    net::io_context io_context;
    net::ip::tcp::socket socket{io_context};
    socket.connect(endpoint, ec);

    if (ec) {
        std::cout << "Can't connect to server"sv << std::endl;
        return 1;
    }

    //Повторим теперь процедуру обмена данными, но в обратном порядке: если сервер сначала ждёт данные, то клиент должен первым их отправить.

    //Отправляем данные и проверяем, что нет ошибки.

    socket.write_some(net::buffer("Hello, I'm client!\n"sv), ec);
    if (ec) {
	std::cout << "Error sending data"sv << std::endl;
        return 1;
    }

    /*
	write_some - промежуточная функция между синхронной(boost::asio::write ждет пока все данные будут отправлены)
	и асинхронной(boost::asio::async_write/async_write_some добавляют данные в очередь и не ждут начала их передачи),
	она завершается когда передан хотя бы 1 байт информации, а дальнейшая информация будет передаваться на фоне
    */

    net::streambuf stream_buf;
    net::read_until(socket, stream_buf, '\n', ec);
    std::string server_data{std::istreambuf_iterator<char>(&stream_buf),
                        std::istreambuf_iterator<char>()};

    if (ec) {
        std::cout << "Error reading data"sv << std::endl;
        return 1;
     }

    std::cout << "Server responded: "sv << server_data << std::endl;
}

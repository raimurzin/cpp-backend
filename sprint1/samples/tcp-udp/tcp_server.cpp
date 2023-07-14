#include <iostream>
#include <string>
#include <string_view>

#include <boost/asio.hpp>

namespace net = boost::asio;

int main() {
    //Чтобы сервер мог принимать подключения клиентов он должен создать акцептор
    //то есть обьект типа tcp::acceptor. Он слушает порт и может принимать 
    //входящие соодинения(Необходимо брать порты >1024, так как ниже зарезервированы).

    static const int port = 3333;
    net::io_context io_context;

    net::ip::tcp::acceptor acceptor(io_context, net::ip::tcp::endpoint(net::ip::tcp::v4(), port));
    std::cout << "Waiting for connection..."sv << std::endl;

    //При запуске программы ОС забьет тревогу, так как такое поведение свойственно
    //вредносным программам, под Windows - просто разрешить, под Linux - необходимо
    //либо внести порт в список разрешенных или замени точку доступа endpoint на локальную
    //при этом потеряв возможность примимать входящие сообщения от других компьютеров

    boost::system::error_code ec; //Объект для сохранения ошибки - альтернатива выбрасыванию исключения
    net::ip::tcp::socket socket{io_context};
    acceptor.accept(socket, ec); //Вызов метода accept заставит программу ждать, пока кто-то не подключится к серверу по указанному порту.

    if (ec) {
        std::cout << "Can't accept connection"sv << std::endl;
        return 1;
    }

    //После принятия соединения сокет можно использовать для получения и отправления данных

    net::streambuf stream_buf;
    net::read_until(socket, stream_buf, '\n', ec);
    std::string client_data{std::istreambuf_iterator<char>(&stream_buf),
                        std::istreambuf_iterator<char>()};
    //Операция синхронная — она будет ждать пока все нужные данные не будут прочитаны.

    if (ec) {
        std::cout << "Error reading data"sv << std::endl;
        return 1;
    }

    std::cout << "Client said: "sv << client_data << std::endl;

    /*
	Чтобы отправить данные через сокет противоположной стороне, мы использовали метод write_some.
	В него передаётся буфер, который можно сконструировать из разных объектов.
	Самый простой и понятный способ — из std::string_view. Буфер конструирует функция boost::asio::buffer.
	Помимо std::string_view она может принимать const void*, снабжённый количеством передаваемых байт.
	Этот способ допустимо использовать для передачи данных произвольного объекта.
    */

    socket.write_some(net::buffer("Hello, I'm server!\n"sv), ec);

    if (ec) {
        std::cout << "Error sending data"sv << std::endl;
        return 1;
    }

    return 0;
}

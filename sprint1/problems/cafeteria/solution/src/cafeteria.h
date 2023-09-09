#pragma once
#ifdef _WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/bind_executor.hpp>

#include <memory>
#include <syncstream>
#include <chrono>
#include <string>
#include <iostream>

#include "hotdog.h"
#include "result.h"

namespace net = boost::asio;
namespace sys = boost::system;
using namespace std::literals;
using namespace std::chrono;

class ThreadChecker { //Вспомогательный класс для проверки состояния гонки
public:
    explicit ThreadChecker(std::atomic_int& counter) : counter_{ counter } {}

    ThreadChecker(const ThreadChecker&) = delete;
    ThreadChecker& operator=(const ThreadChecker&) = delete;

    ~ThreadChecker() {
        // assert выстрелит, если между вызовом конструктора и деструктора
        // значение expected_counter_ изменится
        assert(expected_counter_ == counter_);
    }

private:
    std::atomic_int& counter_;
    int expected_counter_ = ++counter_;
};

class Logger { //Вспомогательный класс для корректорного вывода ошибок в поток
public:
    explicit Logger(std::string id) : id_(id) {}

    template <typename... Args>
    void LogMessage(Args ...args) const {
        std::osyncstream os{ std::cout };
        os << id_ << "> ["sv << duration<double>(steady_clock::now() - start_time_).count() << "s] "sv;
        ((os << args), ...);
        os << std::endl;
    }

private:
    std::string id_;
    steady_clock::time_point start_time_{ steady_clock::now() };
};

using Timer = net::steady_timer;
using HotDogHandler = std::function<void(Result<HotDog> hot_dog)>; // Функция-обработчик операции приготовления хот-дога

class HotDogOrder : public std::enable_shared_from_this<HotDogOrder> {
public:
    explicit HotDogOrder(net::io_context& io, int id, HotDogHandler handler,
        std::shared_ptr<GasCooker> gas_cooker, std::shared_ptr<Sausage> sausage, std::shared_ptr<Bread> bread)
        : io_{ io }, id_{ id }, handler_{ std::move(handler) },
        gas_cooker_{ gas_cooker }, sausage_{ sausage }, bread_{ bread }, logger_(std::to_string(id)) {}

    void StartCooking() {
        logger_.LogMessage("Srart Cooking order"sv);
        BakeBread(); // Запускаем процесс запекания
        FrySausage(); // Запускаем процесс прожарки
    }

private:
    void FrySausage() {
        logger_.LogMessage("Start baking bread");
        sausage_->StartFry(*gas_cooker_, [self = shared_from_this()]() {
            self->sausage_timer_.expires_from_now(Milliseconds{ 1500 });

        self->sausage_timer_.async_wait(
            net::bind_executor(self->strand_, [self = std::move(self)](sys::error_code ec) {
                self->OnFried(ec);
                }));
            });
    }

    void OnFried(sys::error_code ec) {
        sausage_->StopFry();
        if (ec) {
            logger_.LogMessage("Fry error : "s + ec.what());
        }
        else {
            logger_.LogMessage(
                "Sausage has been fried in "sv,
                std::chrono::duration_cast<std::chrono::duration<double>>(sausage_->GetCookDuration()).count(),
                " seconds"
            );
            sausage_fried_ = true;
        }
        CheckReadiness();
    }

    void BakeBread() {
        logger_.LogMessage("Start baking bread"sv);
        bread_->StartBake(*gas_cooker_, [self = shared_from_this()]() {
            self->bread_timer_.expires_from_now(Milliseconds{ 1000 });
        
        self->bread_timer_.async_wait(
            net::bind_executor(self->strand_, [self = std::move(self)](sys::error_code ec) {
                self->OnBaked(ec);
                }));
            });
    }

    void OnBaked(sys::error_code ec) {
        bread_->StopBaking();
        if (ec) {
            logger_.LogMessage("Bake error : "s + ec.what());
        }
        else {
            logger_.LogMessage(
                "Breed has been baked in "sv,
                std::chrono::duration_cast<std::chrono::duration<double>>(bread_->GetBakingDuration()).count(),
                " seconds"
            );
            bread_baked_ = true;
        }
        CheckReadiness();
    }

    void CheckReadiness() {
        if (delivered_) {
            logger_.LogMessage("Order has been delivered already"sv);
            return;
        }
        if (IsReadyToDelieve()) {
            Delive();
        }
    }

    void Delive() {
        delivered_ = true;
        handler_(Result{ HotDog{ id_, sausage_, bread_ } });
    }

    bool IsReadyToDelieve() const {
        return sausage_->IsCooked() && bread_->IsCooked() && bread_baked_ && sausage_fried_;
    }

private:
    net::io_context& io_;
    int id_;
    net::strand<net::io_context::executor_type> strand_{ net::make_strand(io_) };
    HotDogHandler handler_;
    Logger logger_;

private:
    std::shared_ptr<Sausage> sausage_; //Указатель на объект "сосиска"
    std::shared_ptr<Bread> bread_; //Указатель на объект "хлеб"
    std::shared_ptr<GasCooker> gas_cooker_; //Указатель на объект "Эль Диабло Плита"

    bool sausage_fried_ = false; //Обжарена ли сосиска?
    bool bread_baked_ = false; //Запечен ли хлеб?
    bool delivered_ = false; //Заказ готов и доставлен?

private:
    Timer bread_timer_{ io_, Milliseconds(1000) };
    Timer sausage_timer_{ io_, Milliseconds(1500) };
};

// Класс "Кафетерий". Готовит хот-доги
class Cafeteria {
public:
    explicit Cafeteria(net::io_context& io) : io_{ io } {}

    // Асинхронно готовит хот-дог и вызывает handler, как только хот-дог будет готов.
    // Этот метод может быть вызван из произвольного потока
    void OrderHotDog(HotDogHandler handler) {
        // TODO: Реализуйте метод самостоятельно
        // При необходимости реализуйте дополнительные классы
        const int order_id = next_order_id_.fetch_add(1);
        std::make_shared<HotDogOrder>
            (io_, order_id, std::move(handler), gas_cooker_, store_.GetSausage(), store_.GetBread())->StartCooking();
    }

private:
    net::io_context& io_;
    std::atomic_int next_order_id_ = 0; //Нужна ли здесь атомарная переменная?

    Store store_; // Используется для создания ингредиентов хот-дога
    // Газовая плита. По условию задачи в кафетерии есть только одна газовая плита на 8 горелок
    // Используйте её для приготовления ингредиентов хот-дога.
    // Плита создаётся с помощью make_shared, так как GasCooker унаследован от enable_shared_from_this.
    std::shared_ptr<GasCooker> gas_cooker_ = std::make_shared<GasCooker>(io_);
};

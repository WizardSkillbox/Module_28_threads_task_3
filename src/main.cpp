#include <iostream>
#include <cassert>
#include <string>
#include <queue>
#include <thread>
#include <mutex>

#include <random_int_generator.h>

using namespace std;
#define NUM_OF_ORDERS (10)
#define ONLINE_ORDERS_TIME_MIN (5)
#define ONLINE_ORDERS_TIME_MAX (10)

#define COOkING_TIME_MIN (5)
#define COOKING_TIME_MAX (15)

#define COURIER_PERIOD (15)


enum CourseType_e {
    COURSE_TYPE_NONE,
    COURSE_TYPE_FIRST = 1,
    COURSE_PIZZA = COURSE_TYPE_FIRST,
    COURSE_SOUP,
    COURSE_STEAK,
    COURSE_SALAD,
    COURSE_SUSHI,
    COURSE_TYPE_LAST = COURSE_SUSHI
};

const string coursesStr[COURSE_TYPE_LAST] = {
        "PIZZA", "SOUP", "STEAK", "SALAD", "SUSHI"
};

class OrdersQueue {
    queue<CourseType_e> readyOrders;
    mutex dataAccess;
public:
    void PushReadyOrder(CourseType_e course) {
        dataAccess.lock();
        readyOrders.push(course);
        dataAccess.unlock();
    }

    CourseType_e GetReadyOrder() {
        CourseType_e order = COURSE_TYPE_NONE;
        dataAccess.lock();
        if (!readyOrders.empty()) {
            order = readyOrders.front();
            readyOrders.pop();
        }
        dataAccess.unlock();
        return order;
    }
};

static const string &CourseTypeToStr(CourseType_e course) {
    assert(course >= COURSE_TYPE_FIRST && course <= COURSE_TYPE_LAST);
    return coursesStr[course - 1];
}

static void NewOrder(CourseType_e course, mutex *kitchen, OrdersQueue *readyOrders) {
    assert(course >= COURSE_TYPE_FIRST && course <= COURSE_TYPE_LAST);
    assert(nullptr != kitchen);
    assert(nullptr != readyOrders);
    auto id = this_thread::get_id();
    IntRandom generator(chrono::high_resolution_clock::now().time_since_epoch().count());

    cout << id << " New order. " << CourseTypeToStr(course) << " waiting for the kitchen..." << endl;

    kitchen->lock();

    auto cookingDelay = generator.generateRandom(COOkING_TIME_MIN, COOKING_TIME_MAX);
    cout << id << " New order. " << CourseTypeToStr(course) << " passes to the kitchen. Cooking for "
         << to_string(cookingDelay)
         << " s" << endl;
    this_thread::sleep_for(chrono::seconds(cookingDelay));

    readyOrders->PushReadyOrder(course);
    cout << id << " New order. " << CourseTypeToStr(course) << " is ready." << endl;

    kitchen->unlock();
}

static void OnlineOrders(int numOrders, mutex *kitchen, OrdersQueue *readyOrders) {
    IntRandom generator(chrono::high_resolution_clock::now().time_since_epoch().count());

    for (int i = 0; i < numOrders; ++i) {
        auto course = (CourseType_e) generator.generateRandom(COURSE_TYPE_FIRST, COURSE_TYPE_LAST);
        cout << "Online orders. New order " << CourseTypeToStr(course) << " detected." << endl;
        thread call(NewOrder, course, kitchen, readyOrders);
        call.detach();

        auto delay = generator.generateRandom(ONLINE_ORDERS_TIME_MIN, ONLINE_ORDERS_TIME_MAX);
        this_thread::sleep_for(chrono::seconds(delay));
    }
}

static void Courier(int numOrders, OrdersQueue *readyOrders) {
    assert(nullptr != readyOrders);

    for (int i = 0; i < numOrders; ++i) {
        bool delivered = false;
        do {
            this_thread::sleep_for(chrono::seconds(COURIER_PERIOD));
            auto readyOrder = readyOrders->GetReadyOrder();
            if (readyOrder == COURSE_TYPE_NONE) {
                cout << "Courier. No ready orders" << endl;
            } else {
                delivered = true;
                cout << "Courier. Order " << CourseTypeToStr(readyOrder) << " has delivered" << endl;
            }
        } while (!delivered);
    }
}

int main() {
    mutex kitchen;
    OrdersQueue readyOrders;
    thread onlineOrders(OnlineOrders, NUM_OF_ORDERS, &kitchen, &readyOrders);
    thread courier(Courier, NUM_OF_ORDERS, &readyOrders);

    onlineOrders.join();
    courier.join();
    return 0;
}

#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> ul(_mtx);
    _condition.wait(ul, [this] { return !_queue.empty(); });
    T message = std::move(_queue.back());
    _queue.pop_back();

    return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lg(_mtx);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    _msgLight = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    std::lock_guard<std::mutex> lg(_mtx);
    while(true)
    {
        TrafficLightPhase light_s = _msgLight->receive();
        if(light_s == TrafficLightPhase::green)
        {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

    // start time
    auto start = std::chrono::system_clock::now();

    // generate random number
    std::random_device rd;
    std::mt19937 e(rd());
    std::uniform_int_distribution<int> dist(4000, 6000);
    int rd_time = dist(e);

    while(true) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        // using std::this_thread::sleep_for to wait for updating phase
        // this method can cause higher value of waiting time
        // because this thread is not always active
        // if(_currentPhase == TrafficLightPhase::red)
        // {
        //     std::this_thread::sleep_for(std::chrono::milliseconds(rd_time));
        //     _currentPhase = TrafficLightPhase::green;
        // }
        // else
        // {
        //     std::this_thread::sleep_for(std::chrono::milliseconds(rd_time));
        //     _currentPhase = TrafficLightPhase::red;
        // }

        int elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now() - start).count();
        if (elapsedTime >= rd_time)
        {
            if (_currentPhase == TrafficLightPhase::red)
            {
                _currentPhase = TrafficLightPhase::green;
            }
            else
            {
                _currentPhase = TrafficLightPhase::red;
            }
        
            _msgLight->send(std::move(_currentPhase));

            start = std::chrono::system_clock::now();
            std::mt19937 e(rd());
            rd_time = dist(e);
        }
    }
}

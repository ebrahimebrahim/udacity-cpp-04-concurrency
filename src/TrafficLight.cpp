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
    std::unique_lock<std::mutex> lock(_mutex);
    _condvar.wait(lock, [this](){ return !this->_queue.empty(); } );

    T msg = std::move(_queue.front());
    _queue.pop_front();
    return msg; // RVO should help avoid copying after return
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.push_back(std::move(msg));
    _condvar.notify_one();
}

/* Implementation of class "TrafficLight" */

/* 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}
*/

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true) {
        TrafficLightPhase phase = _messageQueue.receive();
        if (phase==TrafficLightPhase::GREEN)
            break;
    }
}




void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(&TrafficLight::cycleThroughPhases,this);
}



// function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    // set cycle duration randomly
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator (seed);
    std::uniform_real_distribution<double> distribution(4000,6000); // cycle duration will be between 4000 and 6000 ms
    const double cycleDuration = distribution(generator);

    // init stop watch
    auto lastUpdate = std::chrono::system_clock::now();
    while (true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= cycleDuration)
        {
            // toggle the current phase of the traffic light between red and green
            _currentPhase = (_currentPhase==TrafficLightPhase::GREEN) ? TrafficLightPhase::RED : TrafficLightPhase::GREEN;

            // send an update method to the message queue using move semantics
            _messageQueue.send(TrafficLightPhase(_currentPhase));

            // reset stop watch for next cycle
            lastUpdate = std::chrono::system_clock::now();

            // { // This block is to test things. TODO delete this block
            //     std::lock_guard<std::mutex> lock(_mtx);
            //     std::cout << "PHASE SWITCH! It's now " << ((_currentPhase==TrafficLightPhase::GREEN) ? "green" : "red") << std::endl;
            // }
        }
    }

}
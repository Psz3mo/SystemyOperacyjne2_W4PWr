#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <queue>
#include <mutex>
#include <string>

std::mutex coutMutex; // mutex for cout to avoid overwriting output

class Waiter { // class to manage forks and requests
    public:
        Waiter(int numForks) : numForks(numForks), forks(numForks, false) {} // constructor to initialize forks vector with false values
    
        bool requestForks(int id, int leftFork, int rightFork){ // function to request forks
            std::unique_lock<std::mutex> lock(m);
            waitingQueue.push(id);
            while(waitingQueue.front() != id || forks[leftFork] || forks[rightFork]){ // check if the philosopher is at the front of the queue and if the forks are available
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                lock.lock();
            }
            waitingQueue.pop();
            forks[leftFork] = true;
            forks[rightFork] = true;
            return true;
        }
    
        void releaseForks(int leftFork, int rightFork){ // function to release forks
            std::unique_lock<std::mutex> lock(m);
            forks[leftFork] = false;
            forks[rightFork] = false;
            lock.unlock();
        }
    
    private:
        std::mutex m; // mutex for the waiter
        int numForks;
        std::vector<bool> forks;
        std::queue<int> waitingQueue; // queue to keep track of the philosophers waiting for forks (FIFO)
    };

class Philosopher { // class to represent a philosopher
public:
    Philosopher(int id, int leftFork, int rightFork, Waiter& waiter) : id(id), leftFork(leftFork), rightFork(rightFork), waiter(waiter) {} // constructor to initialize philosopher with id, left fork, right fork, and waiter

    void think(){
        std::unique_lock<std::mutex> lock(coutMutex);
        std::cout << "\033[33mPhilisopher " << id << " is thinking\033[0m\n";
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // sleep for 1 second to simulate thinking
    }

    void eat(){
        while(!waiter.requestForks(id, leftFork, rightFork)){ // request forks until they are available to eat
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::unique_lock<std::mutex> lock(coutMutex);
        std::cout << "\033[32mPhilisopher " << id << " is eating\033[0m\n";
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        waiter.releaseForks(leftFork, rightFork);
        lock.lock();
        std::cout << "Philisopher " << id << " finished eating\n";
        lock.unlock();
    }

    void run(){ // function to run the philosopher
        while(true){
            think();
            eat();
        }
    }

private:
    int id;
    int leftFork;
    int rightFork;
    Waiter& waiter;
};

int main(int argc, char *argv[]) {
    
    if(argc != 2){ // check if the number of arguments is correct
        std::cerr << "Usage: " << argv[0] << " <number_of_philosophers>\n";
        return 1;
    }

    int numPhilosophers = std::stoi(argv[1]); // get the number of philosophers from the command line argument
    std::cout << "Number of philosophers: " << numPhilosophers << std::endl;

    Waiter waiter(numPhilosophers);
    std::vector<std::thread> philosophers; // vector to store philosopher threads
    std::vector<Philosopher> philosopherObjects; // vector to store philosopher objects

    for(int i = 0; i < numPhilosophers; i++){
        philosopherObjects.emplace_back(i+1, i, (i+1)%numPhilosophers, waiter); // create philosopher objects and add them to the vector
    }

    for(int i = 0; i < numPhilosophers; i++){
        philosophers.emplace_back(&Philosopher::run, &philosopherObjects[i]); // create philosopher threads and add them to the vector
    }

    for(auto &philosopher : philosophers){
        philosopher.join(); // join philosopher threads
    }

    return 0;
}
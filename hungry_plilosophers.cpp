#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>

using namespace std;
random_device rd;

string get_color(int id){
    // Multiplying by a prime-like number for better distribution
    int r = (id * 123) % 256;  
    int g = (id * 321) % 256;
    int b = (id * 213) % 256;

    return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";

}

const string reset_color = "\033[0m"; // Reset to default color

enum class ViewType { CONSOLE_TABLE, CONSOLE };
ViewType view_type = ViewType::CONSOLE_TABLE;

class Philosopher {
public:
    enum class State { THINKING, EATING };

    Philosopher(int id, vector<mutex>& forks, mutex& cout_mutex) :
        id(id),
        forks(forks),
        cout_mutex(cout_mutex),
        left_fork_id(id),
        right_fork_id((id + 1) % forks.size()),
        gen(rd()),
        dist(1, 5),
        state(State::THINKING)
        {}

    void operator()() {
        while (true)
        {

            // Check if the simulation should stop
            {
            lock_guard<mutex> lock(run_mutex);
            if (!run) break;
            }
        
            think();
            eat();
        }
    }

    void stop() { 
        run = false; 
        state = State::THINKING;
    }

    State get_state(){
        lock_guard<mutex> lock(state_mutex);
        return state;
    }

    int get_eat_count() { return eat_count; }
    int get_id() { return id; }
    int get_is_running() { return run; }

private:
    void think() {
        {
            lock_guard<mutex> lock(cout_mutex);
            if(view_type == ViewType::CONSOLE){
                cout << get_color(id) << "Philosopher " << id << " is thinking." << reset_color << endl;
            }
            state = State::THINKING;
            
        }
        this_thread::sleep_for(chrono::seconds(get_random_duration()));
        // this_thread::sleep_for(chrono::milliseconds(300));
    }

    void eat() {
        // Fetch the forks next to the philosopher
        auto& left_fork = forks[left_fork_id];
        auto& right_fork = forks[right_fork_id];

        // Determine fork order to avoid deadlock - first grab the fork with the lower id
        if (left_fork_id < right_fork_id) {
            lock(left_fork, right_fork);
        } else {
            lock(right_fork, left_fork);
        }

        lock_guard<mutex> left_lock(left_fork, adopt_lock);
        lock_guard<mutex> right_lock(right_fork, adopt_lock);

        //---------CRITICAL SECTION---------
        {
            lock_guard<mutex> lock(cout_mutex);
            if (view_type == ViewType::CONSOLE){
                cout << get_color(id) << "Philosopher " << id << " is eating." << reset_color << endl;
            }
            state = State::EATING;
        }
        //---------CRITICAL SECTION---------

        this_thread::sleep_for(chrono::seconds(get_random_duration()));
        // this_thread::sleep_for(chrono::seconds(300));

        //---------CRITICAL SECTION---------
        {
            lock_guard<mutex> lock(cout_mutex);
            if (view_type == ViewType::CONSOLE){
                cout << get_color(id) << "Philosopher " << id << " has finished eating." << reset_color << endl;
            }
            eat_count++;
        }
        //---------CRITICAL SECTION---------
    }

    int get_random_duration() {
        return dist(gen);
    }

    int id;
    vector<mutex>& forks;
    mutex& cout_mutex;
    int left_fork_id;
    int right_fork_id;
    mt19937 gen;
    uniform_int_distribution<> dist;
    bool run = true;
    mutex run_mutex;
    int eat_count = 0;
    State state;
    mutex state_mutex;
};


// Function to display the table of philosopher states
void display_table(Philosopher* philosophers, int philosophers_num) {
    while (philosophers[0].get_is_running()) {
        // Clear the screen and move the cursor to the top-left corner
        cout << "\033[2J\033[H";

        // Display the table header
        cout << "Philosopher ID | State     | Eat Count\n";
        cout << "---------------|-----------|----------\n";

        // Display the state of each philosopher
        for (int i = 0; i < philosophers_num; i++) {
            string state_str;
            switch (philosophers[i].get_state()) {
                case Philosopher::State::THINKING: state_str = "THINKING"; break;
                case Philosopher::State::EATING:   state_str = "EATING";   break;
            }

            printf("%sPhilosopher %d | %11s | %d%s\n",
                get_color(i).c_str(),  // ANSI color code for philosopher i
                i,                     // Philosopher ID
                state_str.c_str(),     // State string (e.g., "THINKING" or "EATING")
                philosophers[i].get_eat_count(), // Eat count
                reset_color.c_str());  // Reset color to default
        }

        // Wait for a short time before updating the table again
        this_thread::sleep_for(chrono::milliseconds(200));
    }
}


int main() {
    int num_philosophers;
    cout << "Enter the number of philosophers: ";
    cin >> num_philosophers;

    int view_type_int;
    cout << "Enter the view type (0 for console table, 1 for console): ";
    cin >> view_type_int;

    if (view_type_int == 0) {
        view_type = ViewType::CONSOLE_TABLE;
    } else {
        view_type = ViewType::CONSOLE;
    }

    if (num_philosophers <= 0) {
        cerr << "Number of philosophers must be positive." << endl;
        return 1;
    }

    vector<mutex> forks(num_philosophers);
    mutex cout_mutex;

    // Allocate space for philosopher objects
    Philosopher* philosophers = static_cast<Philosopher*>(operator new[](num_philosophers * sizeof(Philosopher)));
    for (int i = 0; i < num_philosophers; ++i) {
        new (&philosophers[i]) Philosopher(i, forks, cout_mutex); // create philosophers 
    }


    // Start all threads
    vector<thread> threads;
    threads.reserve(num_philosophers + 1);
    for (int i = 0; i < num_philosophers; i++)
    {
        threads.emplace_back(ref(philosophers[i]));
    }
    // Start the thread that displays the table of philosopher states if the view type is set to CONSOLE_TABLE
    if (view_type == ViewType::CONSOLE_TABLE) threads.emplace_back(display_table, ref(philosophers), num_philosophers);

    // Let the simulation run for a while
    this_thread::sleep_for(chrono::seconds(120));

    // Stop all philosophers
    for (int i = 0; i < num_philosophers; i++)
    {
        philosophers[i].stop();
    }

    // Join all threads
    for (auto& t : threads) {
        t.join();
    }

    // Print the number of times each philosopher ate
    cout << "\nEating counts:\n";
    for (int i = 0; i < num_philosophers; i++) {
        cout << get_color(i) << "Philosopher " << i << " ate " << philosophers[i].get_eat_count() << " times." << reset_color << endl;
    }

    // Deallocate space for philosopher objects
    for (int i = 0; i < num_philosophers; i++) {
        philosophers[i].~Philosopher();
    }
    delete[](philosophers);

    return 0;
}
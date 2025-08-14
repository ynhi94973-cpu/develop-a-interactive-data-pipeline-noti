#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class DataPipelineNotifier {
private:
    std::vector<std::string> dataPipeline;
    std::atomic<bool> dataAvailable;
    std::mutex mtx;
    std::condition_variable cv;

public:
    DataPipelineNotifier() : dataAvailable(false) {}

    void addData(const std::string& data) {
        std::lock_guard<std::mutex> lock(mtx);
        dataPipeline.push_back(data);
        dataAvailable = true;
        cv.notify_all();
    }

    void startNotifier() {
        std::thread t([this] {
            while (true) {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this] { return dataAvailable.load(); });

                while (!dataPipeline.empty()) {
                    std::cout << "New data available: " << dataPipeline.front() << std::endl;
                    dataPipeline.erase(dataPipeline.begin());
                }

                dataAvailable = false;
            }
        });
        t.detach();
    }
};

int main() {
    DataPipelineNotifier notifier;
    notifier.startNotifier();

    while (true) {
        std::string data;
        std::cout << "Enter data (or 'exit' to quit): ";
        std::getline(std::cin, data);

        if (data == "exit") break;

        notifier.addData(data);
    }

    return 0;
}
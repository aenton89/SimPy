//
// Created by patryk on 13.02.26.
//

#ifndef PYTHONKERNEL_H
#define PYTHONKERNEL_H

#pragma once
#include <string>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>

namespace fs = std::filesystem;

class PythonKernel {
public:
    PythonKernel(const std::string &pythonPath, const std::string &kernelScript);
    virtual ~PythonKernel();

    bool start(fs::path workingDirectory);
    void stop();
    void reset(fs::path workingDirectory);

    // Wykonanie kodu (blokująco – używane wewnątrz kolejki)
    std::string executeCode(const std::string &code);
    std::string getPlot();
    std::string fetchNamespace();
    void flushPipeUntilEndMarker();

    // Uruchamianie kolejki zadań
    void startThread();
    void stopThread();

    // Dodawanie zadania do kolejki (zwrot future)
    template<typename F>
    auto enqueue(F&& func) -> std::future<decltype(func())> {
        using R = decltype(func());
        auto task = std::make_shared<std::packaged_task<R()>>(std::forward<F>(func));
        auto fut = task->get_future();
        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push([task]() { (*task)(); });
        }
        cv.notify_one();
        return fut;
    }

private:
    int pipe_in[2];
    int pipe_out[2];
    pid_t pid;

    std::string readOutput();

    std::string pythonPath;
    std::string kernelScript;

    // wątek + kolejka
    std::thread worker;
    std::atomic<bool> running{false};
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;

    void threadLoop();
};

#endif //PYTHONKERNEL_H

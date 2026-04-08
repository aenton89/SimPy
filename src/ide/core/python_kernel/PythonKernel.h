//
// Created by patryk on 13.02.26.
//
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
#include <filesystem>
#include <fstream>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/wait.h>
    #include <csignal>
#endif

namespace fs = std::filesystem;



class PythonKernel {
public:
    PythonKernel(const fs::path &pythonPath);
    virtual ~PythonKernel();

    bool start(fs::path workingDirectory);
    void stop();
    void reset(fs::path workingDirectory);

    std::string executeCode(const std::string &code);
    std::string getPlot();
    std::string fetchNamespace();
    void flushPipeUntilEndMarker();

    void startThread();
    void stopThread();

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
    fs::path pythonPath;
    fs::path kernelScript = "kernel.py";

    std::thread worker;
    std::atomic<bool> running{false};
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;

    void threadLoop();
    std::string readOutput();

    // platform-specific handles
#ifdef _WIN32
    HANDLE hProcess = INVALID_HANDLE_VALUE;
    // parent reads stdout/stderr of child
    HANDLE hPipeInRd = INVALID_HANDLE_VALUE;
    // child writes stdout/stderr
    HANDLE hPipeInWr = INVALID_HANDLE_VALUE;
    // child reads stdin
    HANDLE hPipeOutRd = INVALID_HANDLE_VALUE;
    // parent writes stdin of child
    HANDLE hPipeOutWr = INVALID_HANDLE_VALUE;

    bool writeToChild(const std::string &data);
    std::string readFromChild();
#else
    // child stdout → parent reads [0]
    int pipe_in[2] = {-1, -1};
    // parent writes [1] → child stdin
    int pipe_out[2] = {-1, -1};
    pid_t pid = -1;
#endif
};
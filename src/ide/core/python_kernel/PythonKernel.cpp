//
// Created by patryk on 13.02.26.
//

#include "PythonKernel.h"
#include <iostream>
#include <stdexcept>



PythonKernel::PythonKernel(const fs::path &pythonPath)
    : pythonPath(pythonPath) {}

PythonKernel::~PythonKernel() {
    stopThread();
    stop();
}

bool PythonKernel::start(fs::path workingDirectory) {
    std::cout << "Starting Python Kernel..." << std::endl;

#ifdef _WIN32
    // ---- Create two anonymous pipes with inheritable child ends ----
    SECURITY_ATTRIBUTES sa{};
    sa.nLength              = sizeof(sa);
    sa.bInheritHandle       = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    // Pipe: parent writes → child reads (stdin of child)
    if (!CreatePipe(&hPipeOutRd, &hPipeOutWr, &sa, 0)) {
        std::cerr << "CreatePipe (stdin) failed: " << GetLastError() << '\n';
        return false;
    }
    // Make the parent-write end non-inheritable
    SetHandleInformation(hPipeOutWr, HANDLE_FLAG_INHERIT, 0);

    // Pipe: child writes → parent reads (stdout/stderr of child)
    if (!CreatePipe(&hPipeInRd, &hPipeInWr, &sa, 0)) {
        std::cerr << "CreatePipe (stdout) failed: " << GetLastError() << '\n';
        return false;
    }
    // Make the parent-read end non-inheritable
    SetHandleInformation(hPipeInRd, HANDLE_FLAG_INHERIT, 0);

    // ---- Build STARTUPINFO ----
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    // child reads from here
    si.hStdInput = hPipeOutRd;
    // child writes here
    si.hStdOutput = hPipeInWr;
    // redirect stderr too
    si.hStdError = hPipeInWr;

    PROCESS_INFORMATION pi{};

    // Build command line: pythonPath kernelScript workingDirectory
    std::wstring cmdLine =
        L"\"" + pythonPath.wstring()    + L"\" "
        L"\"" + kernelScript.wstring()  + L"\" "
        L"\"" + workingDirectory.wstring() + L"\"";

    std::wstring wWorkDir = workingDirectory.wstring();

    if (!CreateProcessW(nullptr, cmdLine.data(), nullptr, nullptr, TRUE, 0, nullptr, wWorkDir.c_str(), &si, &pi)) {
        std::cerr << "CreateProcess failed: " << GetLastError() << '\n';
        return false;
    }

    hProcess = pi.hProcess;
    CloseHandle(pi.hThread);

    // Close child ends in parent – we no longer need them
    CloseHandle(hPipeOutRd); hPipeOutRd = INVALID_HANDLE_VALUE;
    CloseHandle(hPipeInWr);  hPipeInWr  = INVALID_HANDLE_VALUE;

    // Optional: send plot_config.py
    std::fstream file("../plot_config.py", std::ios::in);
    if (file.is_open()) {
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        content += "\n[[END]]\n";
        writeToChild(content);
    }

    std::cout << "Kernel process started (Windows).\n";
    return true;

#else
    // ---- POSIX: pipe + fork ----
    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
        perror("pipe");
        return false;
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        return false;
    }

    if (pid == 0) {
        // --- child ---
        dup2(pipe_out[0], STDIN_FILENO);
        dup2(pipe_in[1],  STDOUT_FILENO);
        dup2(pipe_in[1],  STDERR_FILENO);

        close(pipe_out[0]); close(pipe_out[1]);
        close(pipe_in[0]);  close(pipe_in[1]);

        execl(pythonPath.c_str(), pythonPath.c_str(), kernelScript.c_str(), workingDirectory.c_str(), (char *)nullptr);

        perror("execl failed");
        exit(1);
    } else {
        // --- parent ---
        close(pipe_out[0]);
        close(pipe_in[1]);

        std::fstream file("../plot_config.py", std::ios::in);
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            content += "\n[[END]]\n";
            write(pipe_out[1], content.c_str(), content.size());
        }

        std::cout << "Kernel process started with PID: " << pid << '\n';
        return true;
    }
#endif
}

void PythonKernel::stop() {
#ifdef _WIN32
    if (hProcess != INVALID_HANDLE_VALUE) {
        TerminateProcess(hProcess, 0);
        WaitForSingleObject(hProcess, INFINITE);
        CloseHandle(hProcess);
        hProcess = INVALID_HANDLE_VALUE;
    }
    auto closeH = [](HANDLE &h) {
        if (h != INVALID_HANDLE_VALUE) { CloseHandle(h); h = INVALID_HANDLE_VALUE; }
    };
    closeH(hPipeOutWr);
    closeH(hPipeInRd);
    closeH(hPipeOutRd);
    closeH(hPipeInWr);
#else
    if (pid > 0) {
        kill(pid, SIGTERM);
        waitpid(pid, nullptr, 0);
        pid = -1;
    }

    if (pipe_out[1] != -1)
        close(pipe_out[1]); pipe_out[1] = -1;
    if (pipe_in[0]  != -1)
        close(pipe_in[0]);  pipe_in[0]  = -1;
#endif
}

void PythonKernel::reset(fs::path workingDirectory) {
    stop();
    start(workingDirectory);
}

#ifdef _WIN32
bool PythonKernel::writeToChild(const std::string &data) {
    DWORD written = 0;
    return WriteFile(hPipeOutWr, data.c_str(), static_cast<DWORD>(data.size()), &written, nullptr) != FALSE;
}

std::string PythonKernel::readFromChild() {
    char buf[256];
    DWORD bytesRead = 0;
    std::string output;

    while (true) {
        if (!ReadFile(hPipeInRd, buf, sizeof(buf) - 1, &bytesRead, nullptr) || bytesRead == 0)
            break;

        buf[bytesRead] = '\0';
        output += buf;

        if (output.find("[[OK]]")    != std::string::npos || output.find("[[ERROR]]") != std::string::npos)
            break;
    }
    return output;
}
#endif

std::string PythonKernel::readOutput() {
#ifdef _WIN32
    return readFromChild();
#else
    char buf[256];
    std::string output;

    while (true) {
        ssize_t n = read(pipe_in[0], buf, sizeof(buf) - 1);
        if (n <= 0) break;

        buf[n] = '\0';
        output += buf;

        if (output.find("[[OK]]")    != std::string::npos || output.find("[[ERROR]]") != std::string::npos)
            break;
    }
    return output;
#endif
}

std::string PythonKernel::executeCode(const std::string &code) {
    std::string payload = code + "\n[[END]]\n";
#ifdef _WIN32
    writeToChild(payload);
#else
    write(pipe_out[1], payload.c_str(), payload.size());
#endif
    return readOutput();
}

void PythonKernel::flushPipeUntilEndMarker() {
    // Reads and discards everything until [[OK]] is seen.
    // Reuses readOutput() – the result is just thrown away.
    readOutput();
}

std::string PythonKernel::fetchNamespace() {
    std::string vars = "__vars__\n[[END]]\n";
#ifdef _WIN32
    writeToChild(vars);
#else
    write(pipe_out[1], vars.c_str(), vars.size());
#endif
    return readOutput();
}

std::string PythonKernel::getPlot() {
    std::string plot = "'__pltmap__'\n[[END]]\n";
    std::cout << plot << std::endl;
#ifdef _WIN32
    writeToChild(plot);
#else
    write(pipe_out[1], plot.c_str(), plot.size());
#endif
    return readOutput();
}

void PythonKernel::startThread() {
    running = true;
    worker  = std::thread([this] {
        this->threadLoop();
    });
}

void PythonKernel::stopThread() {
    running = false;
    cv.notify_all();
    if (worker.joinable()) worker.join();
}

void PythonKernel::threadLoop() {
    while (running) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return !tasks.empty() || !running; });
            if (!running && tasks.empty()) break;
            task = std::move(tasks.front());
            tasks.pop();
        }

        task();
    }
}
//
// Created by patryk on 13.02.26.
//

#include "PythonKernel.h"

PythonKernel::PythonKernel(const std::string &pythonPath, const std::string &kernelScript)
    : pythonPath(pythonPath), kernelScript(kernelScript), pid(-1) {
    pipe_in[0] = pipe_in[1] = -1;
    pipe_out[0] = pipe_out[1] = -1;
}

PythonKernel::~PythonKernel() {
    stopThread();
    stop();
}

bool PythonKernel::start(fs::path workingDirectory) {
    std::cout <<"start" << std::endl;
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
        // proces dziecka
        dup2(pipe_out[0], STDIN_FILENO);
        dup2(pipe_in[1], STDOUT_FILENO);

        close(pipe_out[1]);
        close(pipe_in[0]);

        execl(pythonPath.c_str(), pythonPath.c_str(), kernelScript.c_str(), workingDirectory.c_str(), (char *)nullptr);
        perror("execl");

        exit(1);
    } else {
        char buffer[32];
        std::fstream file("/home/patryk/CLionProjects/SimPy_py_gui/src/python_kernel/plot_config.py", std::ios::in);
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        content = content + "\n[[END]]\n";
        write(pipe_out[1], content.c_str(), content.size());
        flushPipeUntilEndMarker();

        // proces rodzica
        close(pipe_out[0]);
        close(pipe_in[1]);

        return true;
    }
}


void PythonKernel::stop() {
    if (pid > 0) {
        kill(pid, SIGTERM);
        waitpid(pid, nullptr, 0);
        pid = -1;
    }

    if (pipe_out[1] != -1) { close(pipe_out[1]); pipe_out[1] = -1; }
    if (pipe_in[0]  != -1) { close(pipe_in[0]);  pipe_in[0]  = -1; }
}

void PythonKernel::reset(fs::path workingDirectory) {
    stop();
    start(workingDirectory);
}

std::string PythonKernel::executeCode(const std::string &code) {
    // Dodajemy kod + znacznik końca
    std::string payload = code + "\n[[END]]\n";
    write(pipe_out[1], payload.c_str(), payload.size());


    // Odczyt wyników
    return readOutput();
}

void PythonKernel::flushPipeUntilEndMarker() {
    const size_t BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];
    std::string dummy_output;

    while (true) {
        ssize_t n = read(pipe_in[0], buffer, BUFFER_SIZE - 1);
        if (n <= 0) {
            // Brak danych albo błąd
            break;
        }

        buffer[n] = '\0'; // zakończ string
        dummy_output += buffer;

        // Jeśli znaleźliśmy znacznik końca – kończymy
        if (dummy_output.find("[[OK]]") != std::string::npos) {
            break;
        }
    }

    // Nie zwracamy dummy_output – po prostu ignorujemy
}



std::string PythonKernel::fetchNamespace() {
    std::string vars = "__vars__\n[[END]]\n";
    write(pipe_out[1], vars.c_str(), vars.size());
    return readOutput();
}

std::string PythonKernel::getPlot()
{
    std::string plot = "'__pltmap__'\n[[END]]\n";
    std::cout<<plot<<std::endl;
    write(pipe_out[1], plot.c_str(), plot.size());
    return readOutput();
}

std::string PythonKernel::readOutput() {
    char buffer[256];
    std::string output;

    while (true) {
        ssize_t n = read(pipe_in[0], buffer, sizeof(buffer) - 1);
        if (n <= 0) break;  // Jeśli nie ma danych, kończymy

        buffer[n] = '\0';
        output += buffer;

        // Dodajemy warunek, aby przerwać tylko, gdy znajdziesz marker zakończenia
        if (output.find("[[OK]]") != std::string::npos || output.find("[[ERROR]]") != std::string::npos) {
            break;
        }
    }

    return output;
}



// ================== WĄTEK KOLEJKI ==================

void PythonKernel::startThread() {
    running = true;
    worker = std::thread([this] { this->threadLoop(); });
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

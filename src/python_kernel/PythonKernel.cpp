//
// Created by patryk on 23.07.25.
//

#include "PythonKernel.h"
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>

PythonKernel::PythonKernel(const std::string &pythonPath, const std::string &kernelScript)
    : pythonPath(pythonPath), kernelScript(kernelScript), pid(-1) {}

PythonKernel::~PythonKernel() {
    stop();
}

bool PythonKernel::start() {
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
        dup2(pipe_out[0], STDIN_FILENO);
        dup2(pipe_in[1], STDOUT_FILENO);

        close(pipe_out[1]);
        close(pipe_in[0]);

        execl(pythonPath.c_str(), pythonPath.c_str(), kernelScript.c_str(), (char *)nullptr);
        perror("execl");
        exit(1);
    } else {
        close(pipe_out[0]);
        close(pipe_in[1]);
        return true;
    }
}

void PythonKernel::stop() {
    if (pid > 0) {
        kill(pid, SIGTERM); // zamiast tylko czekać
        waitpid(pid, nullptr, 0);
        pid = -1;
    }

    // ZAMYKANIE potoków tylko raz
    if (pipe_out[1] != -1) { close(pipe_out[1]); pipe_out[1] = -1; }
    if (pipe_in[0]  != -1) { close(pipe_in[0]);  pipe_in[0]  = -1; }
}


std::string PythonKernel::readOutput() {
    char buffer[256];
    std::string output;
    ssize_t n;
    while ((n = read(pipe_in[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        output += buffer;
        if (output.find("[[OK]]") != std::string::npos) break;
        if (output.find("[[ERROR]]") != std::string::npos) break;
    }
    return output;
}

void PythonKernel::reset()
{
    stop();
    start();
}

std::string PythonKernel::executeCode(const std::string &code) {
    std::string payload = code + "\n[[END]]\n";
    write(pipe_out[1], payload.c_str(), payload.size());
    return readOutput();
}

std::string PythonKernel::fetchNamespace() {
    std::string vars = "__vars__\n[[END]]\n";
    write(pipe_out[1], vars.c_str(), vars.size());
    return readOutput();
}

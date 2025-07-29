//
// Created by patryk on 23.07.25.
//

#ifndef PYTHONKERNEL_H
#define PYTHONKERNEL_H

#pragma once
#include <string>

class PythonKernel {
public:
    PythonKernel(const std::string &pythonPath, const std::string &kernelScript);
    ~PythonKernel();

    bool start();
    void stop();
    void reset();
    std::string executeCode(const std::string &code);
    std::string fetchNamespace();

private:
    int pipe_in[2];
    int pipe_out[2];
    pid_t pid;

    std::string readOutput();
    std::string pythonPath;
    std::string kernelScript;
};

#endif //PYTHONKERNEL_H

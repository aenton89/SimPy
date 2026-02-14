#ifndef CODECELL_H
#define CODECELL_H

#include "baseCell.h"
#include "../../../core/python_kernel/PythonKernel.h"
#include "../../../include/TextEditor.h"
#include <future>
#include <atomic>

class CodeCell : public BaseCell
{
public:
    explicit CodeCell(PythonKernel& kernel);
    CodeCell(CodeCell&& other) noexcept;
    CodeCell& operator=(CodeCell&& other) noexcept;

    // Interfejs BaseCell
    bool Draw(int id) override;
    CellType getType() const override { return CellType::CodeCell; }
    void setInputText(const std::string& input) override;
    std::string getInputText() const override;

    // API specyficzne dla CodeCell
    void setKernel(PythonKernel* kernel);
    void setOutputText(const std::string& output);
    std::string getOutputText() const;
    std::string getBase64() const;

private:
    TextEditor inputEditor;
    PythonKernel* kernel = nullptr;

    std::future<std::string> exec_future;
    std::atomic<bool> is_executing{ false };
    bool execution_requested = false;

    void requestExecution();
    void startExecution();
    void pollExecutionResult();

    void processPlot(const std::string& rawPlot);
    void updateTextureFromBase64(const std::string& base64Data);
    void clearTexture();

    unsigned int texture = 0; // GLuint
    int texture_width = 0;
    int texture_height = 0;
    std::string base64;

    char output_buffer[4 * 4096]{};
};

#endif //CODECELL_H
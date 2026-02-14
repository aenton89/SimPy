//
// Created by patryk on 10.02.26.
//

#ifndef CODECELL_H
#define CODECELL_H

#include "baseCell.h"
#include "../../core/python_kernel/PythonKernel.h"
#include "../../../include/TextEditor.h"


class CodeCell : public BaseCell
{
public:
    explicit CodeCell(PythonKernel& kernel);
    CodeCell(CodeCell&& other) noexcept;
    CodeCell& operator=(CodeCell&& other) noexcept;

    // ===== GŁÓWNY INTERFEJS =====
    bool Draw(int id) override;
    CellType getType() const override { return CellType::CodeCell; }

    // ===== API DLA NOTEBOOKA =====
    void setKernel(PythonKernel* kernel);

    void setCodeInput(const std::string& input);
    void setCodeOutput(const std::string& output);

    std::string getCodeInput() const;
    std::string getCodeOutput() const;
    std::string getBase64() const;

private:
    // =============================
    // ======= UI STATE ============
    // =============================
    TextEditor inputEditor;
    bool focused = false;

    // =============================
    // ===== EXECUTION STATE =======
    // =============================
    PythonKernel* kernel = nullptr;

    std::future<std::string> exec_future;
    std::atomic<bool> is_executing{ false };
    bool execution_requested = false;

    void requestExecution();
    void startExecution();
    void pollExecutionResult();


    // =============================
    // ===== OUTPUT / PLOT =========
    // =============================
    void processPlot(const std::string& rawPlot);
    void updateTextureFromBase64(const std::string& base64Data);
    void clearTexture();

    GLuint texture = 0;
    int texture_width = 0;
    int texture_height = 0;
    std::string base64;

    // =============================
    // ===== INTERNAL STORAGE ======
    // =============================
    char output_buffer[4 * 4096]{};
};

#endif //CODECELL_H

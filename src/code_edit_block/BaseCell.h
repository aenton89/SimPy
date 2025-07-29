//
// Created by patryk on 23.07.25.
//

#ifndef BASECELL_H
#define BASECELL_H

// forward declaration, zamiast includowania całego nagłówka
class PythonKernel;

class BaseCell {
public:
    virtual ~BaseCell() = default;
    virtual bool Draw(int id) = 0;

    // zostawiamy wskaźnik, bo to forward declaration
    virtual void setKernel(PythonKernel* kernel) {}

    virtual std::string getType() const = 0;
};

#endif // BASECELL_H


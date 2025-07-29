//
// Created by patryk on 27.07.25.
//


#ifndef OPEN_IPYNB_H
#define OPEN_IPYNB_H

#include <string>
#include <map>
#include <BaseCell.h>
#include <vector>
#include <memory>

class OpenFile
{
public:
    struct CellData
    {
        std::string cell_type;
        std::string source;
        std::string outputs;
    };

    std::map<int, CellData> openIpynb(const std::string& path);
    std::map<int, CellData> createIpynb_map(const std::vector<std::unique_ptr<BaseCell>>& cells);
    void writeIpynb(const std::string& path, const std::map<int, CellData>& cells);
    void openPyfile(const std::string& path);

};

#endif // OPEN_IPYNB_H
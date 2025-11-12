//
// Created by tajbe on 24.03.2025.
//
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <cmath>
#include <imgui.h>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/complex.hpp>
// dla serializacji - pod polimorfizm
#include <cereal/types/polymorphic.hpp>
// dla serializacji pointer'ów
#include <cereal/types/memory.hpp>
#include "../../data/math/solvers/SolverManager.h"



/* klasa z której dziedziczą wszystkie bloki, odpowiada za:
- procesowanie danych
- przechowywanie danych wejściowych i wyjściowych
- liczbe wejść i wyjść
- nazwe bloku
 */
class Block {
protected:
    int numInputs;
    int numOutputs;
    bool has_menu;
    // TODO: potem wrócić to do protected
    std::vector<double> inputValues;
    std::vector<double> outputValues;
public:
    // TODO: GUI - zmienne od GUI
    int id;
    ImVec2 position = ImVec2(100, 100);
    ImVec2 size = ImVec2(120, 120);
    bool open = true;
    std::vector<int> connections;
    int numConnected = 0;

    Block(int _id, int _numInputs, int _numOutputs, bool _has_menu = false);
    virtual ~Block() = default;

    // TODO: GUI - metoda do rysowania
    virtual void drawContent();
    // tu stricte odpowiada za to co będzie narysowane w menu
    virtual void drawMenu() {};
    void drawIcon();

    // metoda dla kopiowania bloczków przez CTRL+D
    virtual std::unique_ptr<Block> clone() const = 0;

    // metoda do przetwarzania danych wejściowych i generowania danych wyjściowych
    virtual void process() = 0;

    // gettery i settery
    // const std::string& getName() const;
    void setInput(int port, double value);
    double getOutput(int port) const;

    int getNumInputs() const;
    int getNumOutputs() const;

    // TODO: spoza GUI
    int getId() const;

    // resetowanie stanu bloku - jeśli jest potrzebne
    virtual void resetBefore() {};
    virtual void resetAfter() {};

    #ifdef UNIT_TESTS
    public:
        std::vector<double>& getInputValues() { return inputValues; }
    #endif

    template<class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(id),
           CEREAL_NVP(numInputs),
           CEREAL_NVP(numOutputs),
           CEREAL_NVP(has_menu),
           CEREAL_NVP(inputValues),
           CEREAL_NVP(outputValues),
           CEREAL_NVP(connections),
           CEREAL_NVP(numConnected),
           CEREAL_NVP(open));

        std::vector<float> pos = {position.x, position.y};
        std::vector<float> sz  = {size.x, size.y};
        ar(cereal::make_nvp("position", pos), cereal::make_nvp("size", sz));

        if constexpr (Archive::is_loading::value) {
            position = ImVec2(pos[0], pos[1]);
            size = ImVec2(sz[0], sz[1]);
        }
    }
};



/* klasa pomocnicza do klonowania bloczków
- używa CRTP czy coś (Curiously Recurring Template Pattern)
- dziedziczy konstruktor po Block
- (note to self):
- szablony muszą być definiowane w pliku .h, bo kompilator musi widzieć ich definicję przy instancjonowaniu
 */
template <typename Derived>
class BlockCloneable : public Block {
public:
    // dziedziczymy konstruktory z Block
    using Block::Block;

    // dla serializacji
    BlockCloneable() : Block(-1, 0, 0, false) {}

    std::unique_ptr<Block> clone() const override {
        return std::make_unique<Derived>(static_cast<const Derived&>(*this));
    }
};
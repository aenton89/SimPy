//
// Created by tajbe on 24.03.2025.
//
#pragma once

// #include <imgui.h>
// #include <vector>
// #include <memory>
// #include <cereal/cereal.hpp>



/* klasa z której dziedziczą wszystkie bloki, odpowiada za:
- procesowanie danych
- przechowywanie danych wejściowych i wyjściowych
- liczbe wejść i wyjść
- nazwe bloku
 */
class Block {
protected:
    int maxNumInputs;
    int maxNumOutputs;
    // TODO: pod różne porty, ale narazie dodajemy do każdego następnego
    int numOutputs = 0;
    int numInputs = 0;
    bool has_menu;

    std::vector<double> inputValues;
    std::vector<double> outputValues;
public:
    // TODO: GUI - zmienne od GUI
    int id;
    ImVec2 position = ImVec2(100, 100);
    ImVec2 size = ImVec2(120, 120);
    bool open = true;

    Block(int _id, int _numInputs, int _numOutputs, bool _has_menu = false);
    virtual ~Block() = default;

    // TODO: GUI - metoda do rysowania
    virtual void drawContent();
    // tu stricte odpowiada za to co będzie narysowane w menu
    virtual void drawMenu() {};
    void drawIcon();

    // metoda dla kopiowania bloczków przez CTRL+D
    [[nodiscard]]
    virtual std::unique_ptr<Block> clone() const = 0;

    // metoda do przetwarzania danych wejściowych i generowania danych wyjściowych
    virtual void process() = 0;

    // gettery i settery
    void setInput(int port, double value);
    void setCurrentNumOutputs(int num);
    void setCurrentNumInputs(int num);
    [[nodiscard]]
    int getCurrentNumInputs() const;
    [[nodiscard]]
    int getCurrentNumOutputs() const;
    [[nodiscard]]
    double getOutput(int port) const;
    [[nodiscard]]
    int getNumInputs() const;
    [[nodiscard]]
    int getNumOutputs() const;
    [[nodiscard]]
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
           CEREAL_NVP(maxNumInputs),
           CEREAL_NVP(numOutputs),
           CEREAL_NVP(has_menu),
           CEREAL_NVP(inputValues),
           CEREAL_NVP(outputValues),
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

    [[nodiscard]]
    std::unique_ptr<Block> clone() const override {
        return std::make_unique<Derived>(static_cast<const Derived&>(*this));
    }
};
#ifndef TURINGMACHINE_TURINGMACHINEINTERFACE_H
#define TURINGMACHINE_TURINGMACHINEINTERFACE_H

#include <set>
#include <utility>
#include <vector>
#include <map>
#include <memory>

struct Command;

typedef std::map<char, Command> State;

typedef std::vector<State> Q;

typedef std::set<char> A;       // External alphabet
class Tape;
class TuringMachine;
class TuringMachineBuilder;

enum Shift: int8_t
{
    kLeft = -1,
    kRight = 1,
    kStop = 0
};

struct Command
{
    char replacement;
    Shift shift;
    size_t nextStateIndex;

    explicit Command()
    {}
    explicit Command(char replacement, Shift shift, size_t nextStateIndex)
    {
        this->replacement = replacement;
        this->shift = shift;
        this->nextStateIndex = nextStateIndex;
    }
};

class Tape
{
private:
    std::string data;
public:
    Tape(std::string str = "")
        : data(std::move(str))
    {}

    char &operator[](size_t index)
    {
        if (index >= data.size()) {
            data.resize(index + 1, ' ');
        }
        return data[index];
    }

    [[nodiscard]] const std::string &str() const
    {
        return data;
    }
};

class TuringMachine
{
private:
    const A alphabet;
    const Q states;
    Tape tape; // aka char []

    size_t carriage;
    [[nodiscard]] size_t execCommand(const Command &cmd)
    {
        tape[carriage] = cmd.replacement;
        carriage += cmd.shift;
        return cmd.nextStateIndex;
    }
public:
    explicit TuringMachine() = delete;
    explicit TuringMachine(A a, Q q)
        :
        alphabet(std::move(a)), states(std::move(q)), carriage(0)
    {}

    void fillTape(std::string tape)
    {
        this->tape = tape;
    }

    std::string getTape()
    {
        return this->tape.str();
    }

    bool isValidTape(const std::string &str)
    {
        for (char c : str) {
            if (alphabet.find(c) == alphabet.end()) {
                return false;
            }
        }
        return true;
    }

    void run()
    {
        auto currentState = states[0];
        Command currentCommand;
        do {
            currentCommand = currentState[tape[carriage]];
            currentState = states[execCommand(currentCommand)];
        }
        while (currentCommand.shift != Shift::kStop);
    }
};

class TuringMachineBuilder
{
private:
    Q q;
    A a;
public:
    std::shared_ptr<TuringMachineBuilder> addState(const State &state)
    {
        this->q.push_back(state);
        return std::make_shared<TuringMachineBuilder>(*this);
    }
    std::shared_ptr<TuringMachineBuilder> setAlphabet(const A &a)
    {
        this->a = a;
        return std::make_shared<TuringMachineBuilder>(*this);
    }
    TuringMachine get()
    {
        return TuringMachine(this->a, this->q);
    }

};

#endif //TURINGMACHINE_TURINGMACHINEINTERFACE_H

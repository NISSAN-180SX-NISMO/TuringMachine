#ifndef POSTSYSTEMEMULATOR_H
#define POSTSYSTEMEMULATOR_H

#include <set>
#include <vector>
#include <memory>
#include <regex>
#include <unordered_map>
#include <iomanip>

typedef std::set<char> A;       // Constant alphabet
typedef std::set<char> X;       // Variables alphabet
typedef std::set<char> A1;      // Axioms alphabet
class RegexableRuleGenerator;
struct Rule;

typedef std::vector<Rule> R;

class DynamicRuleSearcher;
class PostSystemEmulator;
class PostSystemSourceDataParser;
class PostSystemEmulatorBuilder;

struct Rule
{
    std::string pattern;
    std::string placeholder;
};

class RegexableRuleGenerator
{
private:
    std::unordered_map<char, std::pair<int, std::string>> variableGroupMap;
    int groupCounter;
    std::string regexPattern;

    bool isSpecialCharacter(char ch)
    {
        static const std::string specialChars = R"(\.^$|()[]{}*+?/-)";
        return specialChars.find(ch) != std::string::npos;
    }

    std::string createConstGroup(char var)
    {
        std::string group;
        if (isSpecialCharacter(var))
            group += R"(\)";
        group += var;
        return group;
    }

    std::string createVarGroup(char var, const X &x, const A1 &a1)
    {
        if (variableGroupMap.find(var) == variableGroupMap.end()) {
            std::string group = "[";
            for (char axiom : a1) {
                if (isSpecialCharacter(axiom))
                    group += R"(\)";
                group += axiom;
            }
            group += "]+";
            variableGroupMap[var] = {groupCounter++, group};
            return group;
        }
        else {
            return R"(\)" + std::to_string(variableGroupMap[var].first);
        }
    }

public:
    std::string createRegPattern(const A &a, const X &x, const A1 &a1, const std::string &pattern)
    {
        groupCounter = 1;
        for (char ch : pattern) {
            regexPattern += (x.count(ch) ?
                             "(" + createVarGroup(ch, x, a1) + ")" :
                             createConstGroup(ch));
        }

        return regexPattern;
    }

    std::string processPlaceholder(const X &x, const std::string &placeholder, const std::string &match)
    {
        std::string processedPlaceholder;
        for (char ch : placeholder) {
            if (x.count(ch)) {
                std::regex reg(variableGroupMap[ch].second);
                std::smatch placeholderMatch;
                if (std::regex_search(match, placeholderMatch, reg)) {
                    std::string value = placeholderMatch.str();
                    processedPlaceholder += value;
                }
            }
            else processedPlaceholder += ch;
        }
        return processedPlaceholder;
    }

};

class PostSystemEmulator
{
private:
    A alphabet;
    X variables;
    A1 axioms;
    R rules;

    std::string line;
    Rule lastRule;
    bool applyRule(const Rule &rule)
    {
        RegexableRuleGenerator regRuleGen;
        std::regex regRule(regRuleGen.createRegPattern(alphabet, variables, axioms, rule.pattern));
        std::smatch match;
        if (std::regex_search(line, match, regRule)) {
            std::string x = match[1].str();
            line.replace(match.position(), match.length(), regRuleGen.processPlaceholder(variables, rule.placeholder, match.str()));
            return true;
        }
        return false;
    }

    bool applyAnyRule()
    {
        for (const auto& rule : rules)
            if (applyRule(rule))
            {
                lastRule = rule;
                return true;
            }
        return false;
    }

public:
    explicit PostSystemEmulator(A alphabet, X variables, A1 axioms, R rules)
        :
        alphabet(std::move(alphabet)),
        variables(std::move(variables)),
        axioms(std::move(axioms)),
        rules(std::move(rules))
    {}

    void setLine(const std::string& line)
    {
        this->line = line;
    }

    std::string run()
    {
        const int lineWidth = 30;
        const int ruleWidth = 20;

        std::cout << std::left
                  << std::setw(lineWidth) << "Line Before"
                  << std::setw(ruleWidth) << "Rule Applied"
                  << "Line After" << std::endl;
        std::cout << std::string(60, '-') << std::endl;

        std::string lineBefore = line;

        while (applyAnyRule())
        {
            std::string ruleApplied = lastRule.pattern + " -> " + lastRule.placeholder;

            std::cout << std::left
                      << std::setw(lineWidth) << lineBefore
                      << std::setw(ruleWidth) << ruleApplied
                      << line << std::endl; // `line` после применения правила

            lineBefore = line;
        }
        return line;
    }
};

class PostSystemSourceDataParser
{
private:
    A a;
    X x;
    A1 a1;
    R r;
    std::string sourceData;
    std::string line;

    void parseLine()
    {
        std::regex aRegex(R"(.*)");
        std::smatch match;
        if (std::regex_search(sourceData, match, aRegex)) {
            line = match.str();
        }
    }

    void parseA()
    {
        std::regex aRegex(R"(A\s*=\s*\{([^}]*)\};)");
        std::smatch match;
        if (std::regex_search(sourceData, match, aRegex)) {
            for (const char &ch : match[1].str()) {
                if (!isspace(ch) && ch != ',') {
                    a.insert(ch);
                }
            }
        }
    }

    void parseX()
    {
        std::regex xRegex(R"(X\s*=\s*\{([^}]*)\};)");
        std::smatch match;
        if (std::regex_search(sourceData, match, xRegex)) {
            for (const char &ch : match[1].str()) {
                if (!isspace(ch) && ch != ',') {
                    x.insert(ch);
                }
            }
        }
    }

    void parseA1()
    {
        std::regex a1Regex(R"(A1\s*=\s*\{([^}]*)\};)");
        std::smatch match;
        if (std::regex_search(sourceData, match, a1Regex)) {
            for (const char &ch : match[1].str()) {
                if (!isspace(ch) && ch != ',') {
                    a1.insert(ch);
                }
            }
        }
    }

    void parseR()
    {
        std::regex rRegex(R"(R\s*=\s*\{([^}]*)\};)");
        std::smatch match;
        if (std::regex_search(sourceData, match, rRegex)) {
            std::string rulesData = match[1].str();
            std::regex ruleRegex(R"(\"([^\"]+)->([^\"]+)\")");
            auto rulesBegin = std::sregex_iterator(rulesData.begin(), rulesData.end(), ruleRegex);
            auto rulesEnd = std::sregex_iterator();

            for (auto it = rulesBegin; it != rulesEnd; ++it) {
                std::smatch ruleMatch = *it;
                Rule rule = {ruleMatch[1].str(), ruleMatch[2].str()};
                r.push_back(rule);
            }
        }
    }
public:
    explicit PostSystemSourceDataParser(const std::string &data)
    {
        sourceData = data;
        parseLine();
        parseA();
        parseX();
        parseA1();
        parseR();
    }

    [[nodiscard]] const A &getA() const { return a; }
    [[nodiscard]] const X &getX() const { return x; }
    [[nodiscard]] const A1 &getA1() const { return a1; }
    [[nodiscard]] const R &getR() const { return r; }
    [[nodiscard]] const std::string &getLine() const { return line; }
};


class PostSystemEmulatorBuilder
{
private:
    A a;
    X x;
    A1 a1;
    R r;
public:
    std::shared_ptr<PostSystemEmulatorBuilder> addRule(const Rule &rule)
    {
        this->r.push_back(rule);
        return std::make_shared<PostSystemEmulatorBuilder>(*this);
    }
    std::shared_ptr<PostSystemEmulatorBuilder> setAlphabet(const A &a)
    {
        this->a = a;
        return std::make_shared<PostSystemEmulatorBuilder>(*this);
    }
    std::shared_ptr<PostSystemEmulatorBuilder> setAxioms(const A1 &a1)
    {
        this->a1 = a1;
        return std::make_shared<PostSystemEmulatorBuilder>(*this);
    }
    std::shared_ptr<PostSystemEmulatorBuilder> setVariables(const X &x)
    {
        this->x = x;
        return std::make_shared<PostSystemEmulatorBuilder>(*this);
    }
    PostSystemEmulator get()
    {
        return PostSystemEmulator(this->a, this->x, this->a1, this->r);
    }
};

#endif //POSTSYSTEMEMULATOR_H

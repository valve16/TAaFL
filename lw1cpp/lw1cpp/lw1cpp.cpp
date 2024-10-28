#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <windows.h>
#include <iostream>
#include <map>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <queue>

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}



struct Trans
{
    std::string inputSym;
    std::string nextPos;
    std::string outSym;
};

struct MealyState
{ 
    std::string curr;
    std::vector<Trans> transitions;
};

struct MooreState 
{
    std::string state;
    std::string newState;
    std::string output;
    std::unordered_map<std::string, std::string> transitions; // input -> next state
};

std::unordered_set<std::string> findReachableStates(const std::vector<MooreState>& mooreAutomaton, const std::string& startState) 
{
    std::unordered_set<std::string> reachableStates;
    std::queue<std::string> statesQueue;

    // Добавляем начальное состояние
    statesQueue.push(startState);
    reachableStates.insert(startState);
    while (!statesQueue.empty()) 
    {
        std::string currentState = statesQueue.front();
        statesQueue.pop();

        // Находим состояние в автомате
        for (const auto& mooreState : mooreAutomaton) 
        {
            if (mooreState.newState == currentState) 
            {
                // Добавляем все состояния, достижимые через переходы
                for (const auto& transition : mooreState.transitions) 
                {
                    const std::string& nextState = transition.second;
                    if (reachableStates.find(nextState) == reachableStates.end()) 
                    {
                        reachableStates.insert(nextState);
                        statesQueue.push(nextState);
                    }
                }
            }
        }
    }
    return reachableStates;
}


std::vector<MooreState> convertToMoore(const std::vector<MealyState>& mealyAutomaton) 
{

    std::vector<MooreState> mooreAutomaton;
    std::set<std::string> stateOutputSet; // словарь
    std::unordered_map<std::string, std::string> eqNewStateOld; // first - old; second - new(q1, q2);

    std::string startState = mealyAutomaton.begin()->curr; 
    std::string mooreStartState;
    // Создаем состояния Мура на основе состояний Милли

    int i = 0;
    for (const auto& pos : mealyAutomaton) 
    {
        for (const auto& trans : pos.transitions) 
        {
            std::string mooreState = trans.nextPos + "_" + trans.outSym;
            if (stateOutputSet.insert(mooreState).second) // Проверка на наличие этого состояния в автомате Мура
            {
                MooreState newMooreState;

                newMooreState.state = mooreState;
                newMooreState.output = trans.outSym;
                //newMooreState.newState = NewMooreStateName;
                std::cout << mooreState << " ";
                mooreAutomaton.push_back(newMooreState);

                //eqNewStateOld[mooreState] = NewMooreStateName;
                i++;
            }
        }
    }


    std::sort(mooreAutomaton.begin(), mooreAutomaton.end(), // сортировка вектора мура по состояниям
        [](const MooreState& a, const MooreState& b) {
            return a.state < b.state; // Сравнение по возрастанию
        });


    int j = 0;
    for (auto& mooreState : mooreAutomaton)
    {
        std::string newState = 'q' + std::to_string(j);
        mooreState.newState = newState;
        //std::cout << mooreState.newState << " ";
        eqNewStateOld[mooreState.state] = newState;
        j++;
    }

    // Добавляем переходы для состояний Мура
    for (auto& mooreState : mooreAutomaton) 
    {
        std::string baseState = mooreState.state.substr(0, mooreState.state.find('_'));
        for (const auto& pos : mealyAutomaton) 
        {
            if (pos.curr == baseState) 
            {
                for (const auto& trans : pos.transitions) 
                {
                    std::string nextState = trans.nextPos + "_" + trans.outSym;
                    std::string nextMooreState = eqNewStateOld[nextState];
                    mooreState.transitions[trans.inputSym] = nextMooreState;
                }
            }
        }
    }

    //for (const auto& state : eqNewStateOld)
    //{
    //    std::cout << state.first.substr(0, state.first.find('_')) << " -> " << state.second << "\n";
    //};
    //for (auto& mooreState : mooreAutomation)
    //{
    //    std::cout << mooreState.state << " -> " << mooreState.newState << "\n";
    //}

    std::unordered_set<std::string> reachableStates = findReachableStates(mooreAutomaton, "q0"); // Начальное состояние

    // Создаем новый вектор, который содержит только достижимые состояния
    std::vector<MooreState> cleanedMooreAutomaton;
    for (const auto& mooreState : mooreAutomaton) 
    {
        if (reachableStates.find(mooreState.newState) != reachableStates.end()) 
        {
            cleanedMooreAutomaton.push_back(mooreState);
        }
    }

    return cleanedMooreAutomaton;

    //return mooreAutomation;
}

std::vector<MealyState> ReadMealyToVec(std::vector<MealyState>& positions,std::ifstream& file)
{
   
    std::string line;
    std::getline(file, line);
    std::vector<std::string> tempRow = split(line, ';');


    for (size_t i = 1; i < tempRow.size(); ++i) // Пропускаем первый элемент (пустой)
    {
        MealyState currPos;
        currPos.curr = tempRow[i]; // Текущая позиция - имя состояния
        positions.push_back(currPos);
    }

    std::vector<std::string> tRow;
    while (std::getline(file, line))
    {
        tempRow = split(line, ';');
        std::string input_sym = tempRow[0];
        for (int i = 0; i < positions.size(); i++)
        {

            Trans trans;
            tRow = split(tempRow[i + 1], '/');
            trans.inputSym = input_sym;
            trans.nextPos = tRow[0];
            trans.outSym = tRow[1];
            positions[i].transitions.push_back(trans);
        }
    };
    file.close();

    return positions;
}

void ConvertMooreToMealy(std::ifstream& file, std::ofstream& outFile)
{
    std::string line;
    std::getline(file, line);
    std::vector<std::string> tempRowOut = split(line, ';');
    std::getline(file, line);
    std::vector<std::string> tempRowStates = split(line, ';');


    std::unordered_map<std::string, std::string> stateToOut;

    for (size_t i = 1; i < tempRowOut.size(); ++i) 
    {
        stateToOut[tempRowStates[i]] = tempRowOut[i]; // выходной символ соответсвующий состоянию
        outFile << ";" << tempRowStates[i];
    }
    outFile << "\n";

    while (std::getline(file, line))
    {
        std::vector<std::string> rowNextSt = split(line, ';');
        outFile << rowNextSt[0] << ";";
        for (int i = 1; i < rowNextSt.size(); i++)
        {
            outFile << rowNextSt[i] << "/" << stateToOut[rowNextSt[i]] << ";";
        }
        outFile << "\n";
    }
    file.close();
    outFile.close();
}

void WriteMooreToFile(std::vector<MooreState>& mooreAutomaton, std::ofstream& outFile)
{
    if (outFile.is_open())
    {
    std::cout << " convert";
        // Выходы состояний Мура (outputs)
        outFile << ";";
        for (const auto& state : mooreAutomaton)
        {
            outFile << state.output << ";";
        }
        outFile << "\n";

        // Названия состояний Мура (newState)
        outFile << ";";
        for (const auto& state : mooreAutomaton)
        {
            outFile << state.newState << ";";
        }
        outFile << "\n";

        // Транзакции по каждому символу входа
        for (const auto& trans : mooreAutomaton[0].transitions)
        {
            outFile << trans.first << ";"; // Символ входа (inputSym)
            for (const auto& state : mooreAutomaton)
            {
                // Записываем соответствующие переходы для каждого состояния
                outFile << state.transitions.at(trans.first) << ";";

            }
            outFile << "\n";
        }

        outFile.close();
        std::cout << "CSV file has been created successfully.\n";
    }
    else
    {
        std::cerr << "Failed to open file for writing.\n";
    }

}

int main(int argc, char* argv[]) 
{
    SetConsoleOutputCP(1251);

    std::string nameOperation;

    nameOperation = "mealy-to-moore";
    std::string inputFileName = "1_mealy.csv";
    std::string outputFileName = "1moore.csv";


    std::ifstream file(inputFileName);
    std::ofstream outFile(outputFileName);

    if (!file.is_open())
    {
        std::cerr << "Ошибка при открытии файла!" << std::endl;
        return 1;
    }


    if (nameOperation == "mealy-to-moore")
    {
        std::vector<MealyState> mealyStates;
        mealyStates = ReadMealyToVec(mealyStates, file);

        std::vector<MooreState> mooreAutomaton = convertToMoore(mealyStates);
        WriteMooreToFile(mooreAutomaton, outFile);


    }
    else if (nameOperation == "moore-to-mealy")
    {
        ConvertMooreToMealy(file, outFile);
    }
    return 0;
}


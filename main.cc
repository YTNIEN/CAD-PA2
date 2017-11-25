/*2017CAP PA2 - Technology Mapping with the Minimum Critical Path Delay
*/
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <list>
#include <vector>
#include <bitset>
#include <map>
#include <fstream>
#include <string>
#include <algorithm>
#include "pause.h"
#include <sstream>
using namespace std;

enum GATEFUNC {G_PI, G_PO, G_PPI, G_PPO, G_NOT, G_AND, G_NAND, G_OR, G_NOR, G_DFF, G_BUF, G_BAD};
//enum FLAGS {ALL_ASSIGNED, MARKED, FAULT_INJECTED, FAULTY, SCHEDULED, OUTPUT, NumFlag};

class GATE;
class MappingCELL
{
    /*(Class that used to) represent a mapping cell (instantiated from cell library).
    */
    private:
        string Name;
        int ID;
        int Level;
        int Delay;
        int ArriTime;
        int ReqTime;
        int Slack;
        int Count;
        bool Schedule;
        string Func;
        vector<MappingCELL*> InputList;
        vector<MappingCELL*> OutputList;
        GATE* _MappedGatePtr;
          
        explicit MappingCELL(MappingCELL&);
        MappingCELL& operator=(MappingCELL& rhs);
    public:
        MappingCELL(): Name(), ID(-1), Level(-1), Delay(-1), ArriTime(-1), ReqTime(-1), Slack(-1), 
                       Count(0) ,Schedule(false), Func() , _MappedGatePtr(0) 
        {
            InputList.reserve(4);
            OutputList.reserve(4);
        }
        ~MappingCELL() {
        }
        //Modifier
        void SetName(string name) {Name = name;}
        void SetID(int id) {ID = id;}
        void SetLevel(int level) {Level = level;}
        void SetDelay(int delay) {Delay = delay;}
        void SetArriTime(int time) {ArriTime = time;}
        void SetReqTime(int time) {ReqTime = time;}
        void CalcSlack() {Slack = ReqTime - ArriTime;}
        void SetCount(int i) {Count = i;}
        void SetFunc(string func) {Func = func;}
        void AddInputList(MappingCELL* gptr) {InputList.push_back(gptr);}
        void AddOutputList(MappingCELL* gptr) {OutputList.push_back(gptr);}
        void IncCount() {++Count;}
        void DecCount() {--Count;}
        void ResetCount() {Count = 0;}
        void SetSchedule() {Schedule = true;}
        void ResetSchedule() {Schedule = false;}
        void SetMappedGate(GATE* gptr) {_MappedGatePtr = gptr;} 
        //Accesser
        string GetName() const {return Name;}
        int GetID() const {return ID;}
        int GetDelay() const {return Delay;}
        string GetFunc() const {return Func;}
        int No_Fanin() const {return InputList.size();}
        int No_Fanout() const {return OutputList.size();}
        MappingCELL* Fanin(unsigned i) {return InputList[i];}
        MappingCELL* Fanout(unsigned i) {return OutputList[i];}
        int GetLevel() const {return Level;}
        int GetArriTime() const {return ArriTime;}
        int GetReqTime() const {return ReqTime;}
        int GetSlack() const {return Slack;}
        bool Is_Schedule() const {return Schedule;}
        int GetCount() const {return Count;}
        GATE* GetMappedGate() {return _MappedGatePtr;}
        //Method
        void SortInputList();
};

class GATE
{
    /*Building blocks of a subject circuit. A GATE object only falls into two categoraies
    (NAND & NOT), which only denote simple logic functions to express the function of a
    subject circuit, and is not a real cell instance. In DAGON, the mapping from a set of
    GATEs to a MappingCELL depends on both what kind of GATEs a MappingCELL consists of
    and the topology of some part of subject circuit.
    */
    private:
        string Name;
        unsigned ID;
        unsigned Level;
        int ArriTime;
        int ReqTime;
        int Slack;
        unsigned Count;
        bool Schedule;
        GATEFUNC Func;
        vector<GATE*> InputList;
        vector<GATE*> OutputList;
        MappingCELL* CellPtr;
          
        explicit GATE(GATE&);
        GATE& operator=(GATE& rhs);
    public:
        GATE(): Name(), ID(0), Level(0), ArriTime(-1), ReqTime(-1), Slack(0), Count(0) ,Schedule(false), Func(G_BAD) , 
                CellPtr(0)
        {
            InputList.reserve(4);
            OutputList.reserve(4);
        }
        ~GATE() {
        }
        void SetName(string name) {Name = name;}
        void SetID(unsigned id) {ID = id;}
        void SetLevel(unsigned level) {Level = level;}
        void SetArriTime(int time) {ArriTime = time;}
        void SetReqTime(int time) {ReqTime = time;}
        void SetSlack() {Slack = ReqTime - ArriTime;}
        void SetCount(unsigned i) {Count = i;}
        void SetFunc(GATEFUNC func) {Func = func;}
        void AddInputList(GATE* gptr) {InputList.push_back(gptr);}
        void AddOutputList(GATE* gptr) {OutputList.push_back(gptr);}
        void IncCount() {++Count;}
        void DecCount() {--Count;}
        void ResetCount() {Count = 0;}
        void SetSchedule() {Schedule = true;}
        void ResetSchedule() {Schedule = false;}
        string GetName() const {return Name;}
        unsigned GetID() const {return ID;}
        GATEFUNC GetFunc() const {return Func;}
        int No_Fanin() const {return InputList.size();}
        int No_Fanout() const {return OutputList.size();}
        GATE* Fanin(unsigned i) {return InputList[i];}
        GATE* Fanout(unsigned i) {return OutputList[i];}
        void ChangeFanin(unsigned i, GATE* gptr) {InputList[i] = gptr;}
        void ChangeFanout(unsigned i, GATE* gptr) {OutputList[i] = gptr;}
        unsigned GetLevel() const {return Level;}
        int GetArriTime() const {return ArriTime;}
        int GetReqTime() const {return ReqTime;}
        int GetSlack() const {return Slack;}
        bool GetSchedule() const {return Schedule;}
        unsigned GetCount() const {return Count;}
        void SetMCell(MappingCELL* cellptr) {CellPtr = cellptr;}
        MappingCELL* GetMCell() {return CellPtr;}
};

class CELL
{
    /*Standard cell used to map a subject circuit into. Logic function of CELL is also described
    via a bunch of GATE objects.
    */
    private:
        string Name;
        unsigned ID;
        unsigned Count;
        int Delay;
        bool Schedule;
        vector<GATE*> PIList;
        vector<GATE*> POList;
        vector<GATE*> GateList;
        map<string, GATE*> GateMap;
        //
        explicit CELL(CELL&);
        CELL& operator=(CELL& rhs);

    public:
        CELL(int inputNum): Name(), ID(0), Count(0), Delay(-1), Schedule(false) {
            PIList.reserve(inputNum);
            POList.reserve(1);
            GateList.reserve(4);
        }
        ~CELL() {
            for(unsigned i = 0;i < GateList.size(); ++i) {delete GateList[i];}
        }
        //Setter
        void SetName(string name) {Name = name;}
        void SetID(unsigned id) {ID = id;}
        void SetCount(unsigned i) {Count = i;}
        void SetDelay(int delay) {Delay = delay;}
        void AddInput(GATE* gptr) {PIList.push_back(gptr);}
        void AddOutput(GATE* gptr) {POList.push_back(gptr);}
        void AddGate(GATE* gptr) {GateList.push_back(gptr);}
        void IncCount() {++Count;}
        void DecCount() {--Count;}
        void ResetCount() {Count = 0;}
        void SetSchedule() {Schedule = true;}
        void ResetSchedule() {Schedule = false;}
        //Accessor 
        string GetName() {return Name;}
        unsigned GetID() {return ID;}
        unsigned GetCount() {return Count;}
        int GetDelay() {return Delay;}
        bool GetSchedule() {return Schedule;}
        int GetInputBound() {return PIList.capacity();}
        inline int No_Fanin() {return PIList.size();}
        inline int No_Fanout() {return POList.size();}
        inline int No_Gate() {return GateList.size();}
        GATE* Fanin(int i) {return PIList[i];}
        GATE* Fanout(int i) {return POList[i];}
        GATE* GetGate(int i) {return GateList[i];}
        map<string, GATE*>& GetMap() {return GateMap;}
        //Modifier
        void ChangeFanin(unsigned i, GATE* gptr) {PIList[i] = gptr;}
        void ChangeFanout(unsigned i, GATE* gptr) {POList[i] = gptr;}
};

class BFS
{
    /*Class used in DAGON to back-trace from a GATE in subject circuit. GATEs in
    back-tracing is stored in BFS. 
    */
    private:
        int MaxDelay;
        GATE* MaxDelayGate;
        CELL* Cell;
        list<GATE*> CellQueue;
        list<GATE*> SubjQueue;
        vector<GATE*> InputGateList; 
        BFS& operator=(BFS &);
    public:
        BFS(): MaxDelay(0), MaxDelayGate(0), Cell(0) {}
        BFS(BFS& rhs): MaxDelay(rhs.MaxDelay), MaxDelayGate(rhs.MaxDelayGate), Cell(rhs.Cell), 
                       CellQueue(rhs.CellQueue), SubjQueue(rhs.SubjQueue), InputGateList(rhs.InputGateList) {}
        ~BFS() {}
        //Modifier
        void SetCell(CELL* cellptr) {Cell = cellptr;}
        void CellPushBack(GATE* gptr) {CellQueue.push_back(gptr);}
        void SubjPushBack(GATE* gptr) {SubjQueue.push_back(gptr);}
        void CellPopFront() {CellQueue.pop_front();}
        void SubjPopFront() {SubjQueue.pop_front();}
        void AddInputGate(GATE* gptr) {InputGateList.push_back(gptr);}
        //Accesser
        int GetMaxDelay() {return MaxDelay;}
        GATE* GetMaxDelayGate() {return MaxDelayGate;}
        CELL* GetCell() {return Cell;}
        bool CellEmpty() {return CellQueue.empty();}
        bool SubjEmpty() {return SubjQueue.empty();}
        GATE* CellFront() {return CellQueue.front();}
        GATE* SubjFront() {return SubjQueue.front();}
        GATE* Fanin(int i) {return InputGateList[i];}
        int No_Fanin() {return InputGateList.size();}
        //Method
        void CalcMaxDelay() {
            MaxDelay = -1;
            for(unsigned i = 0;i < InputGateList.size();++i) {
                int cellArriTime = 0;
                //we only want to consider the delay of gates which are not PI nor Root
                //of other trees
                if(InputGateList[i]-> GetFunc() != G_PI && InputGateList[i] -> No_Fanout() == 1) {
                   cellArriTime = InputGateList[i] ->  GetMCell() -> GetArriTime();
                }
                if(MaxDelay < cellArriTime) {
                    MaxDelay = cellArriTime;
                    MaxDelayGate = InputGateList[i];
                }
            }
            MaxDelay += Cell -> GetDelay(); 
        }
};

inline bool CompareLevel(const GATE* first, const GATE* second) { return (first -> GetLevel() < second -> GetLevel()); }
inline bool CompareName(const MappingCELL* first, const MappingCELL* second)
{
    string::size_type firstPosNotAlphabet;
    string::size_type lastPosNotAlphabet;
    int firstNum, secondNum;
    stringstream ss;
    string firstName = first -> GetName(); 
    firstPosNotAlphabet = firstName.find_first_not_of("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ");
    if(firstPosNotAlphabet != string::npos) {
        firstName.erase(0, firstPosNotAlphabet);
    }
    lastPosNotAlphabet = firstName.find_last_not_of("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ");
    if(lastPosNotAlphabet != string::npos) {
        firstName.erase(lastPosNotAlphabet+1);
    }
    ss << firstName;
    ss >> firstNum;

    ss.str("");
    ss.clear();

    string secondName = second -> GetName();
    firstPosNotAlphabet = secondName.find_first_not_of("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ");
    if(firstPosNotAlphabet != string::npos) {
        secondName.erase(0, firstPosNotAlphabet);
    }
    lastPosNotAlphabet = secondName.find_last_not_of("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ");
    if(lastPosNotAlphabet != string::npos) {
        secondName.erase(lastPosNotAlphabet+1);
    }
    ss << secondName;
    ss >> secondNum;
    return (firstNum < secondNum);
}

void MappingCELL::SortInputList()
{
    std::sort(InputList.begin(), InputList.end(), CompareName);
}

void clean_ss(stringstream& ss)
{
    ss.str("");
    ss.clear();
}

typedef CELL* CELLPTR;
typedef GATE* GATEPTR;
typedef list<GATE*> ListofGate;

int main(int argc, char** argv)
{   
    clock_t time_init, time_end;
    cout << "//Compile Time: " << __TIME__ << endl;
    time_init = clock();
    //Setup File
    ifstream ifsCkt(argv[1], ifstream::in);
    ifstream ifsCell(argv[2], ifstream::in);
    ofstream ofs(argv[3], ofstream::out);
    ofstream ofs_path(argv[4], ofstream::out);

    if(argc != 5) {
        cerr << "Usage: map <subject_ckt> <cell_lib> <output_ckt> <output_path>" << endl;
        ifsCkt.close();
        ifsCell.close();
        ofs.close();
        ofs_path.close();
        exit(-1);
    }
    else if(!ifsCkt || !ifsCell || !ofs || !ofs_path) {
        if(!ifsCkt) cerr << "Cannot open file: " << argv[1] << endl;
        if(!ifsCell) cerr << "Cannot open file: " << argv[2] << endl;
        if(!ofs) cerr << "Cannot create file: " << argv[3] << endl;
        ifsCkt.close(); 
        ifsCell.close();
        ofs.close();
        ofs_path.close();
        exit(-1);
    }
    //Commencing mission
    unsigned gateNum = 0;
    string tempStr;
    string circuitName; 
    GATEPTR gateptr;
    vector<GATE*> gateList;
    vector<GATE*> PIList;
    vector<GATE*> POList;
    map<string, GATE*> gateMap;
    //
    vector<CELL*> cellList;
    map<string, CELL*> cellMap;
    
    circuitName = argv[1];
    string::size_type idx = circuitName.rfind('/');
    if(idx != string::npos) {circuitName = circuitName.substr(idx+1);}
    idx = circuitName.find(".bench");
    if(idx != string::npos) { circuitName = circuitName.substr(0, idx); }
    cout << "//Circuit Name: " << circuitName << endl;
    
    string gateName;
    string gateFunc;
    string::size_type firstPosNotBlank, lastPosNotBlank;

    while(!ifsCkt.eof()) {
        std::getline(ifsCkt, tempStr);
        if(tempStr[0] == '#' || tempStr[0] == '/') {
            cout << "Encounter '#', '/', pass this annotation in " << argv[1] << endl;
            continue;
        }
#ifdef DEBUG_P
        cout << ">" << tempStr << endl;
#endif
        idx = tempStr.find('(');
        if(idx != string::npos) { 
            idx = tempStr.find('=');
            //if current gate is INPUT or OUTPUT
            if(idx == string::npos) {
                idx = tempStr.find('(');
                gateName = tempStr.substr(idx+1);
                gateFunc = tempStr.substr(0, idx);
                idx = gateName.rfind(')');
                gateName.erase(idx);
                gateptr = new GATE();
                gateptr -> SetID(gateNum);
                ++ gateNum;
                if(gateFunc == "INPUT") {
                    gateptr -> SetFunc(G_PI);
                    gateptr -> SetName(gateName);
                    PIList.push_back(gateptr);
                    gateList.push_back(gateptr);
                    gateMap.insert(std::pair<string, GATE*>(gateName, gateptr));
                }
                else if(gateFunc == "OUTPUT") {
                    gateptr -> SetFunc(G_PO);
                    //PO denotes Primary Output
                    gateptr -> SetName("PO_" + gateName);
                    POList.push_back(gateptr);
                    gateList.push_back(gateptr);
                    gateMap.insert(std::pair<string, GATE*>("PO_" + gateName, gateptr));
                }
                else { cerr << "Error in parsing!!\n"; }
                //cout << "Name: --" << gateptr -> GetName() << "--" << endl
                //    << "Func: --" << gateptr -> GetFunc() << "--" << endl;
            }
            //if current gate is Boolean or F/F gate
            else {
                gateName = tempStr.substr(0, idx); 
                //erasing any blank front and back
                firstPosNotBlank = gateName.find_first_not_of(" ");
                if(firstPosNotBlank != string::npos) {
                    gateName.erase(0, firstPosNotBlank);
                }
                lastPosNotBlank = gateName.find_last_not_of(" ");
                if(lastPosNotBlank != string::npos) {
                    gateName.erase(lastPosNotBlank+1);
                }
                gateFunc = tempStr.substr(idx+2);
                idx = gateFunc.find('(');
                gateFunc = gateFunc.substr(0, idx);

                gateptr = new GATE;
                gateptr -> SetName(gateName);
                gateptr -> SetID(gateNum);
                gateList.push_back(gateptr);
                gateMap.insert(std::pair<string, GATE*>(gateName, gateptr));
                ++ gateNum;
                if(gateFunc == "NOT") {
                    gateptr -> SetFunc(G_NOT);
                }
                else if(gateFunc == "AND") {
                    gateptr -> SetFunc(G_AND);
                }
                else if(gateFunc == "NAND") {
                    gateptr -> SetFunc(G_NAND);
                }
                else if(gateFunc == "OR") {
                    gateptr -> SetFunc(G_OR);
                }
                else if(gateFunc == "NOR") {
                    gateptr -> SetFunc(G_NOR);
                }
                else if(gateFunc == "DFF") {
                    gateptr -> SetFunc(G_DFF);
                    //In normal situation, we have to break flip-flop,
                    //and add 
                    //a pseudo primary input(G_PPI) -> F/F output 
                    //a pseudo primary output <- F/F input
                    //But here, we just ignore that
                }
                else if(gateFunc == "BUFF") {
                    gateptr -> SetFunc(G_BUF);
                }
                //set default to BAD gate, meaning NO such gate in our consideration  
                else {    
                    gateptr -> SetFunc(G_BAD);
                }
#ifdef DEBUG_P
                cout << "Name: " << "--" << gateptr -> GetName() << "--" << endl
                     << "Func: " << "--" << gateptr -> GetFunc() << "--" << endl;
                PAUSE();
#endif
            }
        }
    }

    cout << "\n//Second Parsing" << endl;
    //PAUSE();
    string inputNameStr;
    string inputName;
    string::size_type inputStart;
    string::size_type inputEnd;
    GATEPTR inputGatePtr = 0;
    GATEPTR outputGatePtr = 0;

    ifsCkt.close();
    ifsCkt.open(argv[1], ifstream::in);
    while(!ifsCkt.eof()) {
        std::getline(ifsCkt, tempStr);
        if(tempStr[0] == '#' || tempStr[0] == '/') {
            cout << "Encounter '#', '/', pass this annotation in " << argv[1] << endl;
            continue;
        }
#ifdef DEBUG_P
        cout << ">" << tempStr << endl;
#endif
        idx = tempStr.find('(');
        if(idx != string::npos) { 
            idx = tempStr.find('=');
            //if current gate is Boolean gate, F/F gate or Buffer
            if(idx != string::npos) {
                inputGatePtr = 0;
                outputGatePtr = 0;
                int inputNum = 0;
                
                gateName = tempStr.substr(0, idx); 
                //erasing any blank front and back in gateName
                firstPosNotBlank = gateName.find_first_not_of(" ");
                if(firstPosNotBlank != string::npos) {
                    gateName.erase(0, firstPosNotBlank);
                }
                lastPosNotBlank = gateName.find_last_not_of(" ");
                if(lastPosNotBlank != string::npos) {
                    gateName.erase(lastPosNotBlank+1);
                }

                inputStart = tempStr.find('(');
                inputEnd = tempStr.find(')');
                inputNameStr = tempStr.substr(inputStart+1, inputEnd - inputStart - 1);
#ifdef DEBUG_P
                cout << gateName << ": --" << inputNameStr << "--" << endl;
                PAUSE();
#endif
                outputGatePtr = gateMap[gateName];
                idx = inputNameStr.find(',');
                while(idx != string::npos) {
                    ++inputNum;
                    inputName = inputNameStr.substr(0, idx);
                    inputNameStr.erase(0, idx+1);
                    //erasing any blank front and back
                    firstPosNotBlank = inputName.find_first_not_of(" ");
                    if(firstPosNotBlank != string::npos) {
                        inputName.erase(0, firstPosNotBlank);
                    }
                    lastPosNotBlank = inputName.find_last_not_of(" ");
                    if(lastPosNotBlank != string::npos) {
                        inputName.erase(lastPosNotBlank+1);
                    }
#ifdef DEBUG_P
                    cout << "Input" << inputNum << ": --" << inputName << "--" << endl;
                    cout << "=> --" << inputNameStr << "--" << endl;
#endif
                    inputGatePtr = gateMap[inputName];
                    inputGatePtr -> AddOutputList(outputGatePtr);
                    outputGatePtr -> AddInputList(inputGatePtr);

                    idx = inputNameStr.find(',');
                }
                //last input gate
                ++inputNum;
                //erasing any blank front and back
                firstPosNotBlank = inputNameStr.find_first_not_of(" ");
                if(firstPosNotBlank != string::npos) {
                    inputNameStr.erase(0, firstPosNotBlank);
                }
                lastPosNotBlank = inputNameStr.find_last_not_of(" ");
                if(lastPosNotBlank != string::npos) {
                    inputNameStr.erase(lastPosNotBlank+1);
                }
#ifdef DEBUG_P
                cout << "Input" << inputNum << ": --" << inputNameStr << "--" << endl;
#endif
                inputGatePtr = gateMap[inputNameStr];
                inputGatePtr -> AddOutputList(outputGatePtr);
                outputGatePtr -> AddInputList(inputGatePtr);
            }
            else {  
                inputGatePtr = 0;
                outputGatePtr = 0;
                idx = tempStr.find('(');
                gateName = tempStr.substr(idx+1);
                gateFunc = tempStr.substr(0, idx);
                idx = gateName.rfind(')');
                gateName = gateName.substr(0, idx);
                if(gateFunc == "OUTPUT") {
                    inputGatePtr = gateMap[gateName];
                    outputGatePtr = gateMap["PO_" + gateName];
                    inputGatePtr -> AddOutputList(outputGatePtr);
                    outputGatePtr -> AddInputList(inputGatePtr);
                }
            }
        }
    }

    //Parse the standard cell library
    cout << "\n//Parse cell library file: " << argv[2] << endl;
    int cellDelay = 0;
    int cellInputNum = 0;
    string cellName;
    stringstream ss;    //use stringstream to convert a string to an int
    CELLPTR cellPtr;

    while(!ifsCell.eof()) {
        std::getline(ifsCell, tempStr);
        if(tempStr[0] == '#' || tempStr[0] == '/') {
            cout << "Encounter '#', '/', pass this annotation in " << argv[2] << endl;
            continue;
        }

#ifdef DEBUG_P_C
        cout << ">" << tempStr << endl;
#endif
        idx = tempStr.find_first_of(" ");
        cellName = tempStr.substr(0, idx);
#ifdef DEBUG_P_C
        cout << "Cell Name: "<< "--" << cellName << "--" << endl;
#endif
        tempStr.erase(0, idx+1); 
            
        //Erasing any space front and rear in tempStr
        firstPosNotBlank = tempStr.find_first_not_of(" ");
        if(firstPosNotBlank != string::npos) {
            tempStr.erase(0, firstPosNotBlank);
        }
        lastPosNotBlank = tempStr.find_last_not_of(" ");
        if(lastPosNotBlank != string::npos) {
            tempStr.erase(lastPosNotBlank+1);
        }
            
        ss << tempStr;
        ss >> cellInputNum;
#ifdef DEBUG_P_C
        cout << "InputNum: " << cellInputNum << endl;
#endif
        //Must reset our stringstream object, both its Error Flags and its
        //string inside itself
        ss.str("");
        ss.clear();
            
        std::getline(ifsCell, tempStr);
#ifdef DEBUG_P_C
        cout << ">" << tempStr << endl;
#endif
        ss << tempStr;
        ss >> cellDelay;
#ifdef DEBUG_P_C
        cout << "CellDelay: " << cellDelay << endl;
#endif
        //Must reset stringstream object for next use
        ss.str("");
        ss.clear();

        cellPtr = new CELL(cellInputNum);
        cellPtr -> SetName(cellName);
        cellPtr -> SetDelay(cellDelay);
        cellList.push_back(cellPtr);
        cellMap.insert(std::pair<string, CELL*>(cellName, cellPtr));
        map<string, GATE*> &gateMapInCell = cellPtr -> GetMap();
#ifdef DEBUG_P_C
        cout << "//New Cell info: " << endl;
        cout << "cell name: " << cellPtr -> GetName() << endl;
        cout << "input num: " << cellPtr -> No_Fanin() << endl;
        cout << "input bound: " << cellPtr -> GetInputBound() << endl;
        cout << "const delay: " << cellPtr -> GetDelay() << endl;
        PAUSE();
#endif
            
        std::getline(ifsCell, tempStr);
#ifdef DEBUG_P_C
        cout << ">" << tempStr << endl;
        PAUSE();
#endif
        idx = tempStr.find('=');
        //Parsing structrual info inside a Cell
        while(idx != string::npos) {
            int inputNum = 0;
            gateName = tempStr.substr(0, idx); 
            //erasing any blank front and back
            firstPosNotBlank = gateName.find_first_not_of(" ");
            if(firstPosNotBlank != string::npos) {
                gateName.erase(0, firstPosNotBlank);
            }
            lastPosNotBlank = gateName.find_last_not_of(" ");
            if(lastPosNotBlank != string::npos) {
                gateName.erase(lastPosNotBlank+1);
            }
#ifdef DEBUG_P_C
            cout << "--" << gateName << "--" << endl;
#endif

            inputStart = tempStr.find('(');
            inputEnd = tempStr.find(')');

            gateFunc = tempStr.substr(idx+1, inputStart - idx - 1);
            //erasing any blank front and back
            firstPosNotBlank = gateFunc.find_first_not_of(" ");
            if(firstPosNotBlank != string::npos) {
                gateFunc.erase(0, firstPosNotBlank);
            }
            lastPosNotBlank = gateFunc.find_last_not_of(" ");
            if(lastPosNotBlank != string::npos) {
                gateFunc.erase(lastPosNotBlank+1);
            }
#ifdef DEBUG_P_C
            cout << "--" << gateFunc << "--" << endl;
#endif

            gateptr = new GATE();
            gateptr -> SetName(gateName);
            if(gateFunc == "NAND") gateptr -> SetFunc(G_NAND);
            else if (gateFunc == "NOT") gateptr -> SetFunc(G_NOT);
            cellPtr -> AddGate(gateptr);
            gateMapInCell.insert(std::pair<string, GATE*>(gateName, gateptr));
#ifdef DEBUG_P_C
            cout << "//New gate info: " << endl;
            cout << "gate name: " << gateptr -> GetName() << endl;
            cout << "gate func: " << gateptr -> GetFunc() << endl;
#endif

            inputNameStr = tempStr.substr(inputStart+1, inputEnd - inputStart - 1);
#ifdef DEBUG_P_C
            cout << gateName << " = --" << inputNameStr << "--" << endl;
#endif

            idx = inputNameStr.find(',');
            while(idx != string::npos) {
                inputName = inputNameStr.substr(0, idx);
                inputNameStr.erase(0, idx+1);
                //erasing any blank front and back
                firstPosNotBlank = inputName.find_first_not_of(" ");
                if(firstPosNotBlank != string::npos) {
                    inputName.erase(0, firstPosNotBlank);
                }
                lastPosNotBlank = inputName.find_last_not_of(" ");
                if(lastPosNotBlank != string::npos) {
                    inputName.erase(lastPosNotBlank+1);
                }

                ++ inputNum;
#ifdef DEBUG_P_C
                cout << "Input" << inputNum << ": " << inputName << endl;
#endif

                idx = inputNameStr.find(',');
            }
            //last input gate
            //erasing any blank front and back
            firstPosNotBlank = inputNameStr.find_first_not_of(" ");
            if(firstPosNotBlank != string::npos) {
                inputNameStr.erase(0, firstPosNotBlank);
            }
            lastPosNotBlank = inputNameStr.find_last_not_of(" ");
            if(lastPosNotBlank != string::npos) {
                inputNameStr.erase(lastPosNotBlank+1);
            }
            ++ inputNum;
#ifdef DEBUG_P_C
            cout << "Input" << inputNum << ": " << inputNameStr << endl;
#endif

            std::getline(ifsCell, tempStr);
#ifdef DEBUG_P_C
            cout << ">" << tempStr << endl;
            PAUSE();
#endif
            idx = tempStr.find('=');
            //last line of structural info in a Cell
            // presume last line of cell netlist in output gate
            if(idx == string::npos) {cellPtr -> AddOutput(gateptr);}
        }
         
#ifdef DEBUG_P_C
        cout << "//After adding gates: " << endl << "Gate: ";
        for(int i = 0;i < cellPtr -> No_Gate(); ++i) { 
            cout << cellPtr -> GetGate(i) -> GetName() << ", ";
        }
        cout << "\nOutput Gate: ";
        for(int i = 0;i < cellPtr -> No_Fanout(); ++i) { 
            cout << cellPtr -> Fanout(i) -> GetName() << ", ";
        }
        cout << endl << endl;
#endif
    }
    
    //Re-open input file
    ifsCell.clear();
    ifsCell.seekg(0, ios_base::beg);
    
    map<string, GATE*>::iterator gateIterInCell;
    
    cout << "\n//Re-Parse cell library file: " << argv[2] << endl;
    //PAUSE();
    // second parsing of cell library
    while(!ifsCell.eof()) {
        std::getline(ifsCell, tempStr);
        if(tempStr[0] == '#' || tempStr[0] == '/') {
            cout << "Encounter '#', '/', pass this annotation in " << argv[2] << endl;
            continue;
        }
#ifdef DEBUG_SP_C
        cout << ">" << tempStr << endl;
#endif

        idx = tempStr.find_first_of(" ");
        cellName = tempStr.substr(0, idx);
#ifdef DEBUG_SP_C
        cout << "Cell Name: "<< "--" << cellName << "--" << endl;
#endif

        cellPtr = cellMap[cellName];
#ifdef DEBUG_SP_C
        cout << "//Old Cell info: " << endl;
        cout << "Retrieve Cell: " << cellPtr -> GetName() << endl;
        PAUSE();
#endif
            
        //Read oen next line, without doing anything
        std::getline(ifsCell, tempStr);

        std::getline(ifsCell, tempStr);
#ifdef DEBUG_SP_C
        cout << ">" << tempStr << endl;
        PAUSE();
#endif
        idx = tempStr.find('=');
        //Parsing structrual info inside a Cell
        while(idx != string::npos) {
            int inputNum = 0;
            gateName = tempStr.substr(0, idx); 
            //erasing any blank front and back
            firstPosNotBlank = gateName.find_first_not_of(" ");
            if(firstPosNotBlank != string::npos) {
                gateName.erase(0, firstPosNotBlank);
            }
            lastPosNotBlank = gateName.find_last_not_of(" ");
            if(lastPosNotBlank != string::npos) {
                gateName.erase(lastPosNotBlank+1);
            }
#ifdef DEBUG_SP_C
            cout << "--" << gateName << "--" << endl;
#endif

            inputStart = tempStr.find('(');
            inputEnd = tempStr.find(')');

            map<string, GATE*> &gateMapInCell = cellPtr -> GetMap();
            outputGatePtr = gateMapInCell[gateName];
#ifdef DEBUG_SP_C
            cout << "//Old gate info: " << endl;
            cout << "gate name: " << outputGatePtr -> GetName() << endl;
            cout << "gate func: " << outputGatePtr -> GetFunc() << endl;
            PAUSE();
#endif

            inputNameStr = tempStr.substr(inputStart+1, inputEnd - inputStart - 1);
#ifdef DEBUG_SP_C
            cout << gateName << " = --" << inputNameStr << "--" << endl;
#endif
            idx = inputNameStr.find(',');
            while(idx != string::npos) {
                ++ inputNum;
                inputName = inputNameStr.substr(0, idx);
                inputNameStr.erase(0, idx+1);
                //erasing any blank front and back
                firstPosNotBlank = inputName.find_first_not_of(" ");
                if(firstPosNotBlank != string::npos) {
                    inputName.erase(0, firstPosNotBlank);
                }
                lastPosNotBlank = inputName.find_last_not_of(" ");
                if(lastPosNotBlank != string::npos) {
                    inputName.erase(lastPosNotBlank+1);
                }
#ifdef DEBUG_SP_C
                cout << "Input" << inputNum << ": " << inputName << endl;
#endif

                gateIterInCell = gateMapInCell.find(inputName);  
                if(gateIterInCell == gateMapInCell.end()) {
                    // a gate which is a input (PI) of cell
                    // TODO: change the way to find the order of input in a cell
                    int input_order;
                    ss << inputName;
                    ss >> input_order;
                    clean_ss(ss);

                    inputGatePtr = new GATE();
                    inputGatePtr -> SetName(inputName);
                    inputGatePtr -> SetFunc(G_PI);
                    inputGatePtr -> SetID(input_order); // use name as input order
                    cellPtr -> AddInput(inputGatePtr);
                    cellPtr -> AddGate(inputGatePtr);
                    gateMapInCell.insert(std::pair<string, GATE*>(inputName, inputGatePtr));
                    inputGatePtr = gateMapInCell[inputName];
                    //cout << ": " << inputGatePtr -> GetName() << endl;
                }
                else {
                    inputGatePtr = gateIterInCell -> second;
                    //cout << "OLD INPUT" << endl;
                    //cout << ": " << inputGatePtr -> GetName() << endl;
                }

                inputGatePtr -> AddOutputList(outputGatePtr);
                outputGatePtr -> AddInputList(inputGatePtr);
                idx = inputNameStr.find(',');
            }
            // last input gate
            // erase any blank front and back
            firstPosNotBlank = inputNameStr.find_first_not_of(" ");
            if(firstPosNotBlank != string::npos) {
                inputNameStr.erase(0, firstPosNotBlank);
            }
            lastPosNotBlank = inputNameStr.find_last_not_of(" ");
            if(lastPosNotBlank != string::npos) {
                inputNameStr.erase(lastPosNotBlank+1);
            }
            ++ inputNum;
#ifdef DEBUG_SP_C
            cout << "Input" << inputNum << ": " << inputNameStr << endl;
#endif

            gateIterInCell = gateMapInCell.find(inputNameStr);  
            if(gateIterInCell == gateMapInCell.end()) {
                // found a input (PI) of cell
                // presume (guess) that name of this unseen gate is input order of this cell
                int input_order;
                ss << inputNameStr;
                ss >> input_order;
                clean_ss(ss);
                
                inputGatePtr = new GATE();
                inputGatePtr -> SetName(inputNameStr);
                inputGatePtr -> SetFunc(G_PI);
                inputGatePtr -> SetID(input_order); // use name as input order
                cellPtr -> AddInput(inputGatePtr);
                cellPtr -> AddGate(inputGatePtr);
                gateMapInCell.insert(std::pair<string, GATE*>(inputNameStr, inputGatePtr));
                inputGatePtr = gateMapInCell[inputNameStr];
                //cout << ": " << inputGatePtr -> GetName() << endl;
            }
            else {
                inputGatePtr = gateIterInCell -> second;
                //cout << "OLD INPUT";
                //cout << ": " << inputGatePtr -> GetName() << endl;
            }
            
            inputGatePtr -> AddOutputList(outputGatePtr);
            outputGatePtr -> AddInputList(inputGatePtr);
           
            std::getline(ifsCell, tempStr);
#ifdef DEBUG_SP_C
            cout << ">" << tempStr << endl;
            PAUSE();
#endif
            idx = tempStr.find('=');
        }
         
#ifdef DEBUG_SP_C
        cout << "//After adding gates: " << endl << "Gate: ";
        for(int i = 0;i < cellPtr -> No_Gate(); ++i) { 
            cout << cellPtr -> GetGate(i) -> GetName() << ", ";
        }
        cout << "\nInput Gate: ";
        for(int i = 0;i < cellPtr -> No_Fanin(); ++i) {
            cout << cellPtr -> Fanin(i) -> GetName() << ", ";
        }
        cout << "\nOutput Gate: ";
        for(int i = 0;i < cellPtr -> No_Fanout(); ++i) { 
            cout << cellPtr -> Fanout(i) -> GetName() << ", ";
        }
        cout << endl << endl;
#endif
    }

    //Levelize, or Topalogy Sort
    list<GATE*> queue;
    list<GATE*>::iterator listItor;
    list<GATE*>* queuePtr = 0;
    unsigned maxLevel = 0;
    //set all level = 0
    for(unsigned i = 0;i < gateList.size(); ++i) {
        gateList[i] -> SetLevel(0);
        gateList[i] -> ResetSchedule();
    }
    //for(unsigned i = 0;i < gateList.size(); ++i) { cout << gateList[i] -> GetLevel() << endl;}
    for(unsigned i = 0;i < PIList.size(); ++i) {
        inputGatePtr = PIList[i];
        for(unsigned j = 0;j < inputGatePtr -> No_Fanout(); ++j) {
            outputGatePtr = inputGatePtr -> Fanout(j);
            if(outputGatePtr -> GetFunc() != G_PPI) {
                outputGatePtr -> IncCount();
                if(outputGatePtr -> GetCount() == outputGatePtr -> No_Fanin()) {
                    outputGatePtr -> SetLevel(1);
                    queue.push_back(outputGatePtr);
                }
            }
        }
    }
    //Levelize, or Topalogy Sort
    unsigned level1 = 0, level2 = 0;
    while(!queue.empty()) {
        inputGatePtr = queue.front();
        queue.pop_front();
        level2 = inputGatePtr -> GetLevel();
        for(unsigned j = 0;j < inputGatePtr->No_Fanout(); ++j) {
            outputGatePtr = inputGatePtr -> Fanout(j);
            if(outputGatePtr -> GetFunc() != G_PPI) {
                level1 = outputGatePtr -> GetLevel();
                if(level1 <= level2) { outputGatePtr -> SetLevel(level2 + 1);}
                outputGatePtr -> IncCount();
                if(outputGatePtr -> GetCount() == outputGatePtr -> No_Fanin()) {
                    queue.push_back(outputGatePtr);
                }
            }
        }
    } 
    //traverse every gate to find Max Level
    for(unsigned i = 0;i < gateList.size(); ++i) {
        if(maxLevel <= gateList[i] -> GetLevel()) {maxLevel = gateList[i] -> GetLevel();}
    }
    cout << "//MaxLevel: " << maxLevel << endl;

    //traverse every gate to collect the root of every tree
    list<GATE*> treeRoot;
    for(unsigned i = 0;i < gateList.size(); ++i) { 
        GATEPTR gatePtr = gateList[i];
        if(gatePtr -> GetFunc() == G_PO || gatePtr -> No_Fanout() > 1) {
            treeRoot.push_back(gatePtr);
        }
    }
    //sort these roots by their levels
    treeRoot.sort(CompareLevel);
#ifdef DEBUG_R
    cout << "//Root Number: " << treeRoot.size() << endl;
    for(listItor = treeRoot.begin();listItor != treeRoot.end(); ++listItor) {
        cout << (*listItor) -> GetName() << ", ";
    }
    cout << endl;
    //cout << CompareLevel(gateMap["21"], gateMap["20"]) << endl;
    PAUSE();
#endif
    
    queuePtr = new ListofGate[maxLevel + 1];
    list<BFS*> BFSQueue;
    list<BFS*> BFSOfGate; 
    list<MappingCELL*> MCellQueue;
    //NEW Mapped Cells
    vector<MappingCELL*> MCellPIList;
    vector<MappingCELL*> MCellPOList;
    vector<MappingCELL*> MCellList;
    //For each root gate of a tree
    for(listItor = treeRoot.begin();listItor != treeRoot.end(); ++listItor) { 
        GATE* gatePtr = 0;
        GATEPTR rootGatePtr = *listItor;
        queue.push_back(rootGatePtr);
        queuePtr[rootGatePtr->GetLevel()].push_back(rootGatePtr);
#ifdef DEBUG_DA
        cout << "Root_" << rootGatePtr -> GetName() << ": " << std::flush;
#endif
        //push every gate in the tree denoted by its root, rootGatePtr, into
        //queuePtr[]
        while(!queue.empty()) {
            gatePtr = queue.front();
            queue.pop_front();
            for(int i = 0;i < gatePtr->No_Fanin(); ++i) {
                outputGatePtr = gatePtr -> Fanin(i);
                if((outputGatePtr) -> No_Fanout() == 1) {
                    queue.push_back(outputGatePtr);
#ifdef DEBUG_DA
                    cout << outputGatePtr -> GetName() << ", " << std::flush;
#endif
                    queuePtr[outputGatePtr -> GetLevel()].push_back(outputGatePtr);
                }
            }
        }
    
        //DAGON
        list<BFS*> BFSList;
        BFS* BFSPtr = 0;
        GATEPTR cellGatePtr;
        for(int i = 0;i < maxLevel+1; ++i) {
            while(!queuePtr[i].empty()) {
                gatePtr = queuePtr[i].front();
                queuePtr[i].pop_front();
                if(gatePtr -> GetFunc() == G_PI) {
                    MappingCELL* PIMCellPtr = new MappingCELL();
                    PIMCellPtr -> SetName(gatePtr->GetName());
                    PIMCellPtr -> SetDelay(0);
                    PIMCellPtr -> SetMappedGate(gatePtr);
                    gatePtr -> SetMCell(PIMCellPtr);
                    PIMCellPtr -> SetArriTime(0);
                    PIMCellPtr -> SetFunc("G_PI");
                    MCellPIList.push_back(PIMCellPtr);
                    MCellList.push_back(PIMCellPtr);
                    continue;
                }
                if(gatePtr -> GetFunc() == G_PO) {
                    MappingCELL* POMCellPtr = new MappingCELL();
                    POMCellPtr -> SetName(gatePtr->GetName());
                    POMCellPtr -> SetDelay(0);
                    POMCellPtr -> SetMappedGate(gatePtr);
                    gatePtr -> SetMCell(POMCellPtr);
                    for(int j=0;j<gatePtr->No_Fanin();++j) {
                        POMCellPtr -> AddInputList(gatePtr->Fanin(j)->GetMCell());
                    }
                    POMCellPtr -> SetArriTime(0);
                    POMCellPtr -> SetFunc("G_PO");
                    MCellPOList.push_back(POMCellPtr);
                    MCellList.push_back(POMCellPtr);
                    continue;
                }
                // start Subtree MATCHING
                //cout << "To be mapped: " << gatePtr -> GetName() << endl;
                // brute-forcely, try to match every cell starting at gatePtr
                for(unsigned j = 0;j < cellList.size(); ++j) {
                    //get the only root in a cell
                    cellGatePtr = cellList[j] -> Fanout(0);
                    //cout << "Cell root: " << cellGatePtr -> GetName() << endl;

                    BFSPtr = new BFS();
                    BFSPtr -> CellPushBack(cellGatePtr);
                    BFSPtr -> SubjPushBack(gatePtr);
                    //cout << "Cell Gate: " << BFSPtr -> CellFront() -> GetName() << endl;
                    //cout << "Subj Gate: " << BFSPtr -> SubjFront() -> GetName() << endl;
                    BFSList.push_back(BFSPtr);
                    
                    while(!BFSList.empty()) {
                        BFSPtr = BFSList.front();
                        // found a match
                        if(BFSPtr -> CellEmpty() && BFSPtr -> SubjEmpty()) {
                            //cout << ">>CELL MATCH: " << cellList[j] -> GetName() << endl;
                            //cout << "INPUTS: ";
                            //for(int k = 0;k < BFSPtr->No_Fanin();++k) {
                            //    cout << endl << BFSPtr -> Fanin(k) -> GetName() << " =>";
                            //}
                            //cout << endl;
                            BFSPtr -> SetCell(cellList[j]);
                            BFSPtr -> CalcMaxDelay();
                            BFSOfGate.push_back(BFSPtr);
                            BFSList.pop_front();
                            continue;
                            /*below stops the discoveries of other
                            isomorphism form of current cell*/
                            //while(!BFSList.empty()) {
                            //    BFSPtr = BFSList.front();
                            //    BFSList.pop_front();
                            //    delete BFSPtr;
                            //}
                            //break;
                        }
                        GATEPTR subjGatePtr = BFSPtr -> SubjFront();
                        GATEPTR cellGatePtr = BFSPtr -> CellFront(); 
                        // reach a PI of cell circuit, go on to match other gates in current cell candidate
                        if(BFSPtr -> CellFront() -> GetFunc() == G_PI) {
                            BFSPtr -> AddInputGate(subjGatePtr);
                            BFSPtr -> CellPopFront();
                            BFSPtr -> SubjPopFront();
                            continue;
                        }
                        // NEVER mapping any other roots in our circuit or any PO in cell circuit
                        if(BFSPtr -> SubjFront() -> No_Fanout() != 1 && 
                                BFSPtr -> SubjFront() -> GetName() != rootGatePtr -> GetName()) {
                            BFSList.pop_front();
                            delete BFSPtr;
                            continue;
                        }
                        BFSPtr -> SubjPopFront();
                        BFSPtr -> CellPopFront();
                        if(subjGatePtr -> GetFunc() == cellGatePtr -> GetFunc()) {
                            // assume that Cell ckt consists of only NAND and NOT
                            if(subjGatePtr -> GetFunc() == G_NOT) {
                                BFSPtr -> SubjPushBack(subjGatePtr -> Fanin(0));
                                BFSPtr -> CellPushBack(cellGatePtr -> Fanin(0));
                            }
                            else if(subjGatePtr -> GetFunc() == G_NAND) {
                                // if both inputs of this NAND in Cell ckt are PIs, we ignore isomorphism
                                // so, if any of them is not of PI, duplicate a BFS due to isomorphism
                                if(cellGatePtr->Fanin(0) -> GetFunc() != G_PI ||
                                        cellGatePtr->Fanin(1) -> GetFunc() != G_PI) {
                                     BFS* isoBFSPtr = new BFS(*BFSPtr);
                                     isoBFSPtr -> SubjPushBack(subjGatePtr -> Fanin(0));
                                     isoBFSPtr -> SubjPushBack(subjGatePtr -> Fanin(1));
                                     isoBFSPtr -> CellPushBack(cellGatePtr -> Fanin(1));
                                     isoBFSPtr -> CellPushBack(cellGatePtr -> Fanin(0));
                                     BFSList.push_back(isoBFSPtr);
                                }
                                BFSPtr -> SubjPushBack(subjGatePtr -> Fanin(0));
                                BFSPtr -> SubjPushBack(subjGatePtr -> Fanin(1));
                                BFSPtr -> CellPushBack(cellGatePtr -> Fanin(0));
                                BFSPtr -> CellPushBack(cellGatePtr -> Fanin(1));
                            }
                            else {
                                cout << "> ERROR in Subtree Matching, not NAND nor NOT gate to be mapped" << endl;
                            }
                        }
                        // function not the same
                        else {
                            BFSList.pop_front();
                            delete BFSPtr;
                            continue;
                        }
                    }
                }
                //cout << "//End of Matching of Gate: " << gatePtr -> GetName() << endl;
                //cout << "BFSOFGate size: " << BFSOfGate.size() << endl;
                int minCellDelay = INT_MAX;
                BFS* minDelayBFSPtr = 0;
                for(list<BFS*>::iterator iter = BFSOfGate.begin();iter != BFSOfGate.end(); ++iter) {
                    BFSPtr = *iter;
                    if(minCellDelay > BFSPtr -> GetMaxDelay()) {
                        minCellDelay = BFSPtr -> GetMaxDelay();
                        minDelayBFSPtr = BFSPtr;
                    }
                    //cout << "Cell Name: " << BFSPtr -> GetCell() -> GetName() << endl;
                    //cout << "MaxInputDelay: " << BFSPtr -> GetMaxDelay() << endl;
                    //cout << "From Gate: " << BFSPtr -> GetMaxDelayGate() -> GetName() << endl;
                    //cout << "----" << endl;
                }
                
                // create a mapping cell, which leads to the shortest delay
                MappingCELL* MCellPtr = new MappingCELL();
                MCellPtr -> SetName(gatePtr->GetName());
                MCellPtr -> SetDelay(minDelayBFSPtr -> GetCell() -> GetDelay());
                MCellPtr -> SetMappedGate(gatePtr);
                MCellPtr -> SetArriTime(minCellDelay);
                MCellPtr -> SetFunc(minDelayBFSPtr -> GetCell() -> GetName());
                for(int j=0;j<minDelayBFSPtr->No_Fanin();++j) {
                    //To be revised
                    //if(minDelayBFSPtr -> Fanin(j) -> GetFunc() != G_PI) {
                        MCellPtr -> AddInputList(minDelayBFSPtr->Fanin(j)->GetMCell());
                    //}
                }
                gatePtr -> SetMCell(MCellPtr);
                MCellList.push_back(MCellPtr);
                //cout << ">>Cell Chosen: " << MCellPtr->GetFunc() << endl;
                //cout << "Delay: " << MCellPtr->GetArriTime() << endl;
                //for(int i = 0;i < MCellPtr->No_Fanin();++i) {
                //    cout<<MCellPtr->Fanin(i)->GetName()<< " =>"<< endl;
                //}

                while(!BFSOfGate.empty()) {
                    BFSPtr = BFSOfGate.front();
                    BFSOfGate.pop_front();
                    delete BFSPtr;
                }
                //PAUSE();
            }
        }
        //push back the MCell to root gate
        MCellQueue.push_back(rootGatePtr->GetMCell());
        while(!MCellQueue.empty()) {
            MappingCELL* MCellPtr = MCellQueue.front();
            MCellPtr -> ResetCount();
            MCellPtr -> SetSchedule();
            MCellQueue.pop_front();
            for(int i = 0;i < MCellPtr->No_Fanin(); ++i) {
                MCellPtr -> Fanin(i) -> AddOutputList(MCellPtr);
                //if some input MCell in not from the other trees, push it
                if(MCellPtr->Fanin(i)->GetMappedGate()->No_Fanout() == 1) {
                    MCellQueue.push_back(MCellPtr->Fanin(i));
                }
            }
        }
    }

    MappingCELL* inputMCellPtr = 0;
    MappingCELL* outputMCellPtr = 0;
    int inputLevel = 0, outputLevel = 0;
    //Levelize MCell CKT
    for(unsigned i = 0;i < MCellPIList.size(); ++i) {
        MCellPIList[i] -> SetLevel(0);
        MCellQueue.push_back(MCellPIList[i]);
    }
    while(!MCellQueue.empty()) {
        inputMCellPtr = MCellQueue.front();
        MCellQueue.pop_front();
        inputLevel = inputMCellPtr -> GetLevel();
        for(int i = 0;i < inputMCellPtr->No_Fanout(); ++i) {
            outputMCellPtr = inputMCellPtr -> Fanout(i);
            if(outputMCellPtr -> GetFunc() != "G_PPI") {
                outputLevel = outputMCellPtr -> GetLevel();
                if(inputLevel >= outputLevel) {
                    outputMCellPtr -> SetLevel(inputLevel+1);
                }
                outputMCellPtr -> IncCount();
                if(outputMCellPtr -> GetCount() == outputMCellPtr -> No_Fanin()) {
                    MCellQueue.push_back(outputMCellPtr);
                }
            }
        }
    }
    //traverse every MCell we collected, recording maximun level
    int maxMCellLevel = -1;
    for(unsigned i = 0;i < MCellList.size(); ++i) {
        if(MCellList[i] -> Is_Schedule() == true && maxMCellLevel < (MCellList[i]->GetLevel())) {
            maxMCellLevel = MCellList[i] -> GetLevel();
        }
    }
    cout << "//Max Level of MCell: " << maxMCellLevel << endl;

    list<MappingCELL*> *MCellQueuePtr = new list<MappingCELL*>[maxMCellLevel + 1];
    list<MappingCELL*>::iterator MCellQueueIter;
    for(unsigned i = 0;i < MCellList.size(); ++i) { 
        if(MCellList[i]->GetLevel() != -1) {
            MCellQueuePtr[MCellList[i]->GetLevel()].push_back(MCellList[i]);
        }
        else {MCellList[i] -> SetArriTime(-1);}
    }
    int maxMCellArriTime = 0;
    for(unsigned i = 0;i <= maxMCellLevel; ++i) {
        for(MCellQueueIter=MCellQueuePtr[i].begin();MCellQueueIter!=MCellQueuePtr[i].end();++MCellQueueIter) {
            int maxInputDelay = 0;
            outputMCellPtr = *MCellQueueIter;
            for(int j = 0;j < outputMCellPtr->No_Fanin(); ++j) {
                if(maxInputDelay < outputMCellPtr->Fanin(j)->GetArriTime()) {
                    maxInputDelay = outputMCellPtr->Fanin(j)->GetArriTime();
                }
            }

            outputMCellPtr -> SetArriTime(maxInputDelay + outputMCellPtr->GetDelay());
            if(maxMCellArriTime < outputMCellPtr -> GetArriTime()) {
                maxMCellArriTime = outputMCellPtr -> GetArriTime();
            }
        }
    }
    cout << "//Max ArriTime: " << maxMCellArriTime << endl;

    //Calculate require time
    int minMCellReqTime = INT_MAX;
    //set PO require time = maxMCellArriTime
    for(unsigned i = 0;i < MCellPOList.size();++i) { MCellPOList[i] -> SetReqTime(maxMCellArriTime);}
    for(int i = maxMCellLevel;i >= 0; --i) {
        for(MCellQueueIter=MCellQueuePtr[i].begin();MCellQueueIter!=MCellQueuePtr[i].end();++MCellQueueIter) {
            int minOutputReqTime = INT_MAX;
            inputMCellPtr = *MCellQueueIter;
            if(inputMCellPtr -> No_Fanout() > 0) {
                for(int j = 0;j < inputMCellPtr->No_Fanout(); ++j) {
                    outputMCellPtr = inputMCellPtr -> Fanout(j);
                    int outputReqTime = (outputMCellPtr -> GetReqTime()) - (outputMCellPtr -> GetDelay());
                    if(minOutputReqTime > outputReqTime) {
                        minOutputReqTime = outputReqTime;
                    }
                }
            }
            else {minOutputReqTime = maxMCellArriTime;}
            inputMCellPtr -> SetReqTime(minOutputReqTime);
            if(minMCellReqTime > (inputMCellPtr -> GetReqTime())) {
                minMCellReqTime = inputMCellPtr -> GetReqTime();
            }
        }
    }
    //cout << "MinReqTime: " << minMCellReqTime << endl;
    //PAUSE();

    //Calculate Slacks
    for(unsigned i = 0;i < MCellList.size(); ++i) {
        if(MCellList[i]->Is_Schedule() == true) {
            MCellList[i] -> CalcSlack();
        }   
    }
    //cout << endl;
    //cout << "//MCell Circuit: " << endl;
    //for(unsigned i=0;i < MCellList.size();++i) {
    //    cout << ">" << MCellList[i] -> GetName() << ": " << MCellList[i]->GetFunc() << "(" << MCellList[i]->GetDelay()<<")"<< endl;
    //    cout << "Count: " << MCellList[i]->GetCount() << endl;
    //    cout << "Schedule: " << MCellList[i]->Is_Schedule() << endl;
    //    cout << "Level: " << MCellList[i] -> GetLevel() << endl;
    //    cout << "ArriTime: " << MCellList[i] -> GetArriTime() << endl;
    //    cout << "ReqTime: " << MCellList[i] -> GetReqTime() << endl;
    //    cout << "Slack: " << MCellList[i] -> GetSlack() << endl;
    //    for(int j=0;j < MCellList[i]->No_Fanin();++j) {
    //        cout << MCellList[i]->Fanin(j)->GetName() << " =>" << endl;
    //    }
    //    for(int j=0;j < MCellList[i]->No_Fanout();++j) {
    //        cout << "=> " << MCellList[i]->Fanout(j)->GetName() << endl;
    //    }
    //    cout << "----\n";
    //}

    //cout << "\n//Pirmary INPUT: " << endl;
    //for(unsigned i = 0;i < MCellPIList.size(); ++i) {
    //    cout << MCellPIList[i] -> GetName() << endl;
    //}
    //cout << "\n//Pirmary OUTPUT: " << endl;
    //for(unsigned i = 0;i < MCellPOList.size(); ++i) {
    //    cout << MCellPOList[i] -> GetName() << endl;
    //}
    //cout << endl;
    
    //Calculate totoal net number
    int numNet = 0;
    for(unsigned i = 0;i < gateList.size(); ++i) {
        if(gateList[i] -> GetFunc() != G_PO) {
            if(gateList[i] -> No_Fanout() > 1) {
                ++ numNet;
            }
            numNet += gateList[i] -> No_Fanout();
        }
    }
    cout << "//Total Net: " << numNet << endl;


    //Output file
    ofs << maxMCellArriTime << endl; 
    ofs << endl;
    std::sort(MCellPIList.begin(), MCellPIList.end(), CompareName);
    for(unsigned i = 0;i < MCellPIList.size(); ++i) {ofs << "INPUT(" << MCellPIList[i] -> GetName() << ")" << endl;}
    ofs << endl;
    std::sort(MCellPOList.begin(), MCellPOList.end(), CompareName);
    for(unsigned i = 0;i < MCellPOList.size(); ++i) {
        //string::size_type firstPosNotAlphabet;
        //string::size_type lastPosNotAlphabet;
        //string POName = MCellPOList[i] -> GetName();
        ////below remove any alphabet front and back
        //firstPosNotAlphabet = POName.find_first_not_of("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ");
        //if(firstPosNotAlphabet != string::npos) {
        //    POName.erase(0, firstPosNotAlphabet);
        //}
        //lastPosNotAlphabet = POName.find_last_not_of("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ");
        //if(lastPosNotAlphabet != string::npos) {
        //    POName.erase(lastPosNotAlphabet+1);
        //}
        //ofs << "OUTPUT(" << POName << ")" << endl;
        ofs << "OUTPUT(" << MCellPOList[i] -> Fanin(0) -> GetName() << ")" << endl;
    }
    ofs << endl;
    std::sort(MCellList.begin(), MCellList.end(), CompareName);
    for(unsigned i = 0;i < MCellList.size(); ++i) {
        if(MCellList[i] -> GetFunc() != "G_PI" &&  MCellList[i] -> GetFunc() != "G_PO" &&
                MCellList[i] -> Is_Schedule() == true) {
            ofs << MCellList[i] -> GetName() << " = " << MCellList[i] -> GetFunc() << "(";
            //sort inputs of a MCell
            MCellList[i] -> SortInputList();
            ofs << MCellList[i] -> Fanin(0) -> GetName();
            for(int j = 1;j < MCellList[i]->No_Fanin(); ++j) {
                ofs << ", " << MCellList[i] -> Fanin(j) -> GetName();
            }
            ofs << ")" << endl;
        }
    }
    for(unsigned i = 0;i < MCellPOList.size(); ++i) {
        if(MCellPOList[i] -> GetSlack() == 0) {
            outputMCellPtr = MCellPOList[i];
            break;
        }
    }
    //cout << outputMCellPtr -> GetName() << endl;
    while(outputMCellPtr->No_Fanin() != 0) {
        for(int i = 0;i < outputMCellPtr -> No_Fanin(); ++i) {
            if(outputMCellPtr -> Fanin(i) -> GetSlack() == 0) {
                outputMCellPtr = outputMCellPtr -> Fanin(i);
                MCellQueue.push_back(outputMCellPtr);
                break;
            }
        }
    }

    ofs << endl;
    while(!MCellQueue.empty()) {
        outputMCellPtr = MCellQueue.back();
        MCellQueue.pop_back();
        ofs << outputMCellPtr -> GetName() << " ";
        //cout << outputMCellPtr -> GetName() << " ";
    }


    
#ifdef DEBUG
    ////Print zone
    cout << "//Here comes our print zone//" << endl;
    cout << "//Gate Number: " << gateList.size() << endl;
    cout << "//below shows PI info:" << endl
         << "//in the form, GateName: ID/Func/Level/ArriTime/Reqtime/Slack" << endl;
    for(unsigned i = 0;i < PIList.size(); ++i) {
        gateptr  = PIList[i];
        cout << gateptr -> GetName() << ": " << gateptr -> GetID() << " / " << gateptr -> GetFunc() 
             << " / "  << gateptr -> GetLevel() << endl;
    }
    cout << "//below shows PO info:" << endl;
    for(unsigned i = 0;i < POList.size(); ++i) {
        gateptr  = POList[i];
        cout << gateptr -> GetName() << ": " << gateptr -> GetID() << " / " << gateptr -> GetFunc() 
             << " / "  << gateptr -> GetLevel() << endl;

    }
    cout << "//below shows All gate info:" << endl;
    for(unsigned i = 0;i < gateList.size(); ++i) {
        gateptr  = gateList[i];
        cout << gateptr -> GetName() << ": " << gateptr -> GetID() << " / " << gateptr -> GetFunc() 
             << " / " << gateptr -> GetLevel() << " / " << gateptr ->  GetArriTime() 
             << " / " << gateptr -> GetReqTime() << " / " << gateptr -> GetSlack() << endl;
    }
    //
    cout << "//below shows structural info:" << endl;
    for(unsigned i = 0;i < gateList.size(); ++i) {
        cout << gateList[i] -> GetName() << ": " << endl;
        for(unsigned j = 0;j < gateList[i] -> No_Fanin(); ++j) {cout << gateList[i]->Fanin(j)->GetName() << " =>" << endl;}
        for(unsigned j = 0;j < gateList[i] -> No_Fanout(); ++j) {cout << "=> " << gateList[i]->Fanout(j)->GetName() << endl;}
        cout << endl;
    }
    //
    cout << "//below shows Map info: \n";
    for(map<string, GATE*>::iterator gateItor = gateMap.begin(); gateItor != gateMap.end(); ++gateItor) {
        cout << (gateItor) -> first << " -> " << (gateItor) -> second -> GetID() << "<" << gateItor -> second << ">"<< endl;
    }
    cout << endl;
    //
    cout << "//below shows Cell and its inner connections: \n";
    for(int i = 0;i < cellList.size(); ++i) {
        cellPtr = cellList[i];
        cout << cellPtr -> GetName() << ": " << cellPtr -> No_Fanin() << " / " << cellPtr -> GetDelay() << endl;
        cout << "Input: ";
        for(int j = 0;j < cellPtr -> No_Fanin(); ++j) {cout << cellPtr -> Fanin(j) -> GetName() << ", ";}
        cout << "\nOutput: ";
        for(int j = 0;j < cellPtr -> No_Fanout(); ++j) {cout << cellPtr -> Fanout(j) -> GetName() << ", ";}
        cout << "\nGate: " << endl;
        for(int j = 0;j < cellPtr -> No_Gate(); ++j) {
            GATEPTR gatePtr = cellPtr -> GetGate(j);
            cout << gatePtr -> GetName() << ": " << endl;
            for(int k = 0;k < gatePtr -> No_Fanin(); ++k) {cout << gatePtr -> Fanin(k) -> GetName() << " => " << endl;}
            for(int k = 0;k < gatePtr -> No_Fanout(); ++k) {cout << "=> " << gatePtr -> Fanout(k) -> GetName() <<  endl;}
        }

        cout << endl << endl;
    }
    cout << "Size of string: " << sizeof(string) << endl;
    cout << "Size of BFS: " << sizeof(BFS) << endl;
    cout << "Size of MappingCell: " << sizeof(MappingCELL) << endl;
    cout << "Size of GATE: " << sizeof(GATE) << endl;
    cout << "Size of CELL: " << sizeof(CELL) << endl;
#endif
    //Adieu, memory release!
    ifsCkt.close();
    ifsCell.close();
    ofs.close();
    delete[] queuePtr;
    delete[] MCellQueuePtr;
    for(unsigned i = 0;i < gateList.size(); ++i) {delete gateList[i];}
    for(unsigned i = 0;i < cellList.size(); ++i) {delete cellList[i];}
    for(unsigned i = 0;i < MCellList.size();++i) {delete MCellList[i];}
    //Print time profilling
    time_end = clock();
    cout << "\nTotal time = " << static_cast<double>(time_end - time_init) / CLOCKS_PER_SEC << "s" << endl;
    cout << endl;
    return 0;
}

#include <iostream> 
#include <string>
#include <cmath>
#include <climits>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iomanip>

// Detect OS for Clear Screen
#ifdef _WIN32
#define CLEAR_CMD "cls"
#else
#define CLEAR_CMD "clear"
#endif

#include <windows.h>
#include <conio.h>

const int COLOR_RED = 12;
const int COLOR_GREEN = 10;
const int COLOR_CYAN = 11;
const int COLOR_YELLOW = 14;
const int COLOR_WHITE = 7;
const int COLOR_BLUE = 9;

void setColor(int color) 
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void resetColor()
{
    setColor(COLOR_WHITE);
}

using namespace std;



const int STATUS_PICKUP_QUEUE = 0;
const int STATUS_WAREHOUSE = 1;
const int STATUS_LOADING = 2;
const int STATUS_IN_TRANSIT = 3;
const int STATUS_DELIVERY_ATTEMPT = 4;
const int STATUS_DELIVERED = 5;
const int STATUS_RETURNED = 6;
const int STATUS_MISSING = 7;
const int STATUS_CANCELLED = 8;
const int STATUS_UNDER_INVESTIGATION = 9;

void clearScreen() 
{
    system(CLEAR_CMD);
}

void printBanner()
{
    setColor(COLOR_CYAN);
    cout << "\n";
    cout << "   .-----------------------------------------------------------.\n";
    cout << "   |   _____  _    _  _____  ______  _______       ________    |\n";
    cout << "   |  / ____|| |  | ||_   _||  ____||__   __|     |  ____|\\  / |\n";
    cout << "   | | (___  | |  | |  | |  | |__      | | ______ | |__    \\/  |\n";
    cout << "   |  \\___ \\ | |  | |  | |  |  __|     | ||______||  __|   /\\  |\n";
    cout << "   |  ____) || |__| | _| |_ | |        | |        | |____ /  \\ |\n";
    cout << "   | |_____/  \\____/ |_____||_|        |_|        |______/_/\\_\\|\n";
    cout << "   '-----------------------------------------------------------'\n";
    cout << "            >> INTELLIGENT LOGISTICS ENGINE v2.0 <<             \n";
    cout << "\n";
    resetColor();
}

void pauseFunc() 
{
    cout << "\nPress Enter to continue...";
    cin.ignore();
    cin.get();
}

string getCurrentTimestamp() 
{
    time_t now = time(0);
    tm *ltm = localtime(&now);
    stringstream ss;
    ss << setfill('0') << setw(2) << ltm->tm_hour << ":" 
       << setw(2) << ltm->tm_min << ":" << setw(2) << ltm->tm_sec;
    return ss.str();
}

// Simplified Rider Struct
struct Rider 
{
    string name;
    int capacity;
    int currentLoad;
    int priorityLevel; // 1=Standard, 2=Express/Urgent
    
    Rider(string n="", int c=3, int p=1) : name(n), capacity(c), currentLoad(0), priorityLevel(p) {}
};

// Queue for Rider Objects
class RiderQueue 
{
// Node must be accessible to RiderQueue methods
private:
    struct Node { Rider value; Node* next; Node(Rider v) : value(v), next(nullptr) {} };
    Node* front;
    Node* rear;
    
public:
    RiderQueue() : front(nullptr), rear(nullptr) {}
    
    void enqueue(Rider val) 
    {
        Node* newNode = new Node(val);
        if (rear) rear->next = newNode;
        else front = newNode;
        rear = newNode;
    }
    
    Rider dequeue() 
    {
        if (!front) return Rider();
        Rider value = front->value;
        Node* temp = front;
        front = front->next;
        if (!front) rear = nullptr;
        delete temp;
        return value;
    }
    
    // Peek to see front rider
    Rider* peek() 
    {
        if(!front) return nullptr;
        return &(front->value);
    }
    
    bool isEmpty() { return front == nullptr; }
    
    // Rotate queue: pop front and push to back
    void rotate() 
    {
        if(!front || front == rear) return;
        Rider r = dequeue();
        enqueue(r);
    }

    // New: Simple display function
    void displayAll()
    {
        if(!front) { cout << "No riders avaliable.\n"; return; }
        
        cout << "\n----------------------------------------\n";
        cout << "            RIDER FLEET DETAILS         \n";
        cout << " [Types: Standard = Priority 1, Express = Priority 2] \n";
        cout << "----------------------------------------\n";
        cout << left << setw(20) << "Name" << setw(10) << "Type" << setw(15) << "Load Status" << endl;
        cout << "----------------------------------------\n";
        
        Node* curr = front;
        while(curr)
        {
            string type = (curr->value.priorityLevel == 2) ? "Express" : "Standard";
            cout << left << setw(20) << curr->value.name 
                 << setw(10) << type 
                 << curr->value.currentLoad << "/" << curr->value.capacity << " Parcels" << endl;
            curr = curr->next;
        }
        cout << "----------------------------------------\n";
    }

    void exportJSON(ofstream& out)
    {
        Node* curr = front;
        bool first = true;
        while(curr)
        {
            if(!first) out << ",\n";
            string type = (curr->value.priorityLevel == 2) ? "Express" : "Standard";
            out << "    { \"name\": \"" << curr->value.name << "\", \"type\": \"" << type 
                << "\", \"load\": " << curr->value.currentLoad << ", \"capacity\": " << curr->value.capacity << " }";
            first = false;
            curr = curr->next;
        }
    }
    
    // Simple helper to find a suitable rider without complex logic
    // Returns pointer to rider in queue if found (by rotating until match or full circle)
    Rider* findAvailableRider(int parcelPriority)
    {
        if(!front) return nullptr;
        
        Node* curr = front;
        int count = 0;
        int size = 0;
        
        // Count size first
        Node* temp = front;
        while(temp) { size++; temp = temp->next; }
        
        // Try to find best match
        for(int i=0; i<size; i++)
        {
            Rider* r = peek();
            
            // Check capacity
            if(r->currentLoad < r->capacity)
            {
                // Simple Priority Logic: 
                // If parcel is High Priority (3), look for Express Rider (Level 2)
                // If standard (1 or 2), any rider works.
                if(parcelPriority == 3)
                {
                    if(r->priorityLevel >= 2) return r; // Good match
                }
                else
                {
                    return r; // Any rider acts
                }
            }
            
            // Rotate to check next
            rotate();
        }
        
        // Final pass: just find ANY with capacity if strict match failed
        for(int i=0; i<size; i++)
        {
            Rider* r = peek();
            if(r->currentLoad < r->capacity) return r;
            rotate();
        }
        
        return nullptr;
    }
    
    // Decrement load for a rider
    void decrementLoad(string name)
    {
        Node* curr = front;
        while(curr)
        {
            if(curr->value.name == name)
            {
                if(curr->value.currentLoad > 0) curr->value.currentLoad--;
                return;
            }
            curr = curr->next;
        }
    }
    
    // Destructor to prevent memory leak
    ~RiderQueue() 
    {
        while(front) 
        {
            Node* temp = front;
            front = front->next;
            delete temp;
        }
    }
};


struct HistoryEvent 
{
    string description;
    string time;
    string location;
    HistoryEvent* next;
    HistoryEvent(string d, string t, string l) : description(d), time(t), location(l), next(nullptr) {}
};

class TrackingHistory 
{
private:
    HistoryEvent* head;
    HistoryEvent* tail;

public:
    TrackingHistory() : head(nullptr), tail(nullptr) {}
    
    void addEvent(string desc, string loc) 
    {
        HistoryEvent* newEvent = new HistoryEvent(desc, getCurrentTimestamp(), loc);
        if (!head) 
        {
            head = tail = newEvent;
        }
        else 
        {
            tail->next = newEvent;
            tail = newEvent;
        }
    }

    void printTimeline() 
    {
        setColor(COLOR_CYAN);
        cout << "\n\t   [ TRACKING HISTORY TIMELINE ]\n";
        cout << "\t   =============================\n\n";
        resetColor();
        
        HistoryEvent* curr = head;
        while (curr) 
        {
            setColor(COLOR_CYAN); cout << "\t    O   "; resetColor(); cout << curr->time << "\n";
            setColor(COLOR_CYAN); cout << "\t    |   "; resetColor(); cout << curr->description << "\n";
            setColor(COLOR_CYAN); cout << "\t    |   "; resetColor(); cout << "@ " << curr->location << "\n";
            setColor(COLOR_CYAN); cout << "\t    |\n"; resetColor();
            curr = curr->next;
        }
        setColor(COLOR_GREEN); cout << "\t   (V)  "; resetColor(); cout << "[End of Records]\n";
    }
    
    TrackingHistory(const TrackingHistory& other) : head(nullptr), tail(nullptr) 
    {
        HistoryEvent* curr = other.head;
        
        while (curr) 
        {
            addEvent(curr->description, curr->location);
            curr = curr->next;
        }
    }
};


struct Parcel 
{
    string id;
    string destination;
    double weight;
    int priority; 
    int status;
    int priorityScore;
    string assignedRider;
    
    string weightCategory; 
    string zone;           
    int deliveryAttempts;  
    
    long long dispatchTime; 
    long long lastUpdateTime; 
    long long arrivalTime;
    
    TrackingHistory* history; 
    
    Parcel() : weight(0), priority(1), status(0), priorityScore(0), history(new TrackingHistory()), dispatchTime(0), lastUpdateTime(0), arrivalTime(0), deliveryAttempts(0) {}
    
    Parcel(string pid, string dest, double w, int p, string z) : id(pid), destination(dest), weight(w), priority(p), status(STATUS_PICKUP_QUEUE), lastUpdateTime(0), arrivalTime(0), zone(z), deliveryAttempts(0), dispatchTime(0) 
    {
        
        priorityScore = p * 1000 + (int)w; 
        
        if (w < 5.0) 
        weightCategory = "Light";
        
        else if (w < 20.0) 
        weightCategory = "Medium";
        
        else weightCategory = "Heavy";

        history = new TrackingHistory();
        history->addEvent("Pickup Request Created", "Customer Loc");
    }
    
    void updateStatus(int newStatus, string desc, string loc) 
    {
        status = newStatus;
        history->addEvent(desc, loc);
        lastUpdateTime = time(0);
    }

    string getStatusString() const 
    {
        switch (status) 
        {
            case 0: return "Pickup Queue";
            case 1: return "Warehouse (Sorted)";
            case 2: return "Loading Vehicle";
            case 3: return "In Transit";
            case 4: return "Out for Delivery";
            case 5: return "Delivered";
            case 6: return "Returned to Sender";
            case 7: return "MISSING";
            case 8: return "Cancelled";
            case 9: return "Under Investigation";
            default: return "Unknown";
        }
    }
};

ostream& operator<<(ostream& os, const Parcel& p) 
{
    os << left << setw(6) << p.id 
       << setw(12) << p.destination 
       << setw(8) << p.weightCategory
       << setw(8) << p.zone
       << " [" << p.getStatusString() << "]";
    return os;
}



class StringQueue 
{
private:
    struct Node 
    { 
        string data; 
        Node* next; 
        
        Node(string d):data(d),next(nullptr){} 
    };
    
    Node* front; 
    Node* rear;

public:
    StringQueue() : front(nullptr), rear(nullptr) {}
    
    void enqueue(string val) 
    {
        Node* n = new Node(val);
        if(!rear) 
        front = rear = n; 
        
        else 
        {
            rear->next = n;
            rear = n; 
        }
    }
    
    string dequeue() 
    {
        if(!front) 
        return "";
        
        Node* temp = front; 
        string val = front->data; 
        front = front->next;
        
        if(!front) 
        rear = nullptr; 
        delete temp; 
        return val;
    }
    
    bool isEmpty() 
    { 
        return front == nullptr; 
    }
};

class IntArrayList 
{
private:
    int* data; 
    int capacity; 
    int count;
    
    void resize(int newCapacity) 
    {
        int* newData = new int[newCapacity];
        for (int i = 0; i < count; i++) 
        newData[i] = data[i];
        delete[] data; 
        data = newData; 
        capacity = newCapacity;
    }

public:
    IntArrayList() : capacity(50), count(0) { data = new int[capacity]; } 
    
    // Copy Constructor
    IntArrayList(const IntArrayList& other) 
    {
        capacity = other.capacity; count = other.count; data = new int[capacity];
        for(int i=0; i<count; i++) data[i] = other.data[i];
    }
    
    
    IntArrayList& operator=(const IntArrayList& other) 
    {
        if (this != &other) 
        {
            delete[] data;
            capacity = other.capacity;
            count = other.count;
            data = new int[capacity];
            for(int i=0; i<count; i++) 
            data[i] = other.data[i];
        }
        return *this;
    }

    ~IntArrayList() 
    {
        delete[] data; 
    }
    
    void add(int val) 
    { 
        if (count == capacity) 
        resize(capacity * 2); 
        data[count++] = val; 
    }
    
    int get(int index) const 
    { 
        return (index >= 0 && index < count) ? data[index] : -1;
    }
    
    int size() const 
    {
        return count; 
    }
    
    bool isEmpty() const 
    { 
        return count == 0; 
    }
};

class ParcelArrayList 
{
private:
    Parcel** data; 
    int capacity; 
    int count;

    void resize(int newCapacity) 
    {
        Parcel** newData = new Parcel*[newCapacity];
        for (int i = 0; i < count; i++) 
        newData[i] = data[i];
        delete[] data; 
        data = newData; 
        capacity = newCapacity;
    }

public:
    ParcelArrayList() : capacity(20), count(0) 
    { 
        data = new Parcel*[capacity]; 
    }
    
    ~ParcelArrayList() 
    {
        delete[] data; 
    }
    
    void add(Parcel* val) 
    { 
        if (count == capacity) 
        resize(capacity * 2); 
        data[count++] = val; 
    }
    
    void removeLast() 
    {
        if (count > 0) count--; 
    }
    
    Parcel* get(int index) const 
    { 
        return (index >= 0 && index < count) ? data[index] : nullptr; 
    }
    
    void set(int index, Parcel* val) 
    {
        if (index >= 0 && index < count) 
        data[index] = val; 
    }
    
    int size() const 
    {
        return count; 
    }
    
    bool isEmpty() const 
    { 
        return count == 0; 
    }
    
    void swap(int i, int j) 
    { 
        Parcel* temp = data[i]; 
        data[i] = data[j]; 
        data[j] = temp; 
    }
};


// Forward declaration for rider stats tracking
class LogisticsEngine;

struct ParcelNode 
{
    Parcel* data; 
    ParcelNode* next;
    
    ParcelNode(Parcel* val) : data(val), next(nullptr) {}
};

class ParcelLinkedList 
{
private:
    ParcelNode* head; 
    ParcelNode* tail;
    
public:
    ParcelLinkedList() : head(nullptr), tail(nullptr) {}
    
    void pushBack(Parcel* val) 
    {
        ParcelNode* newNode = new ParcelNode(val);
        if (!tail) 
        { 
            head = tail = newNode; 
        } 
        else 
        { 
            tail->next = newNode; tail = newNode; 
        }
    }

    void updateLifecycle(long long currentTime, LogisticsEngine* engine = nullptr);

    void showTransitStatus(long long currentTime)
    {
        ParcelNode* curr = head; bool headerPrinted = false;
        while(curr) 
        {
            if (curr->data->status == STATUS_IN_TRANSIT || curr->data->status == STATUS_LOADING) 
            {
                if (!headerPrinted) 
                { 
                    cout << "\n--- [LIVE TRANSIT MONITOR] ---\n"; 
                    headerPrinted = true; 
                }
                
                long long total = curr->data->arrivalTime - curr->data->dispatchTime;
                long long elapsed = currentTime - curr->data->dispatchTime;
                
                string state = (curr->data->status == STATUS_LOADING) ? "[LOADING] " : "[MOVING]  ";
                
                if(total<=0) total=1;
                double pct = (double)elapsed/total;
                if(pct>1.0) pct=1.0;
                
                cout << state << curr->data->id << " -> " << curr->data->destination << ": [";
                int bars = (int)(pct * 20);
                for(int i=0; i<20; i++) cout << (i < bars ? "=" : " ");
                cout << "] " << (int)(pct * 100) << "%\n";
            }
            curr = curr->next;
        }
        
        if (!headerPrinted) 
        cout << "\n(No parcels active in transit)\n";
    }
};

class ParcelHeap 
{
private:
    ParcelArrayList heap;
    void heapifyUp(int index) 
    {
        while (index > 0) 
        {
            int parent = (index - 1) / 2;
            if (heap.get(index)->priorityScore > heap.get(parent)->priorityScore) 
            {
                heap.swap(index, parent); index = parent;
            } 
            else 
            break;
        }
    }
    void heapifyDown(int index) 
    {
        int left = 2 * index + 1, right = 2 * index + 2, largest = index;
        
        if (left < heap.size() && heap.get(left)->priorityScore > heap.get(largest)->priorityScore) 
        largest = left;
        if (right < heap.size() && heap.get(right)->priorityScore > heap.get(largest)->priorityScore) 
        largest = right;
        if (largest != index) 
        { 
            heap.swap(index, largest);
            heapifyDown(largest);
        }
    }

public:
    
    void insert(Parcel* val) 
    { 
        heap.add(val); 
        heapifyUp(heap.size() - 1); 
    }
    
    Parcel* extractMax() 
    {
        if (heap.isEmpty()) 
        return nullptr;
        
        Parcel* maxVal = heap.get(0);
        heap.set(0, heap.get(heap.size() - 1));
        heap.removeLast();
        if (!heap.isEmpty()) 
        heapifyDown(0);
        
        return maxVal;
    }
    
    bool isEmpty() 
    { 
        return heap.isEmpty(); 
    }
};

struct HashEntry 
{ string key; 
    Parcel* value; 
    bool occupied; 
    HashEntry() : key(""), value(nullptr), occupied(false) {} 
};

class ParcelHashTable 
{
private:
    HashEntry* table; 
    int capacity;
    int hashFunction(const string& key) 
    {
        unsigned long hash = 5381;
        for (char c : key) hash = ((hash << 5) + hash) + c; 
        return hash % capacity;
    }

public:
    
    ParcelHashTable(int cap = 1007) : capacity(cap) 
    { 
        table = new HashEntry[capacity]; 
    }
    
    ~ParcelHashTable() 
    {
        delete[] table; 
    }
    
    void insert(string key, Parcel* value) 
    {
        int index = hashFunction(key); int i = 0;
        while (i < capacity) 
        {
            int probe = (index + i * i) % capacity;
            if (!table[probe].occupied || table[probe].key == key) 
            {
                table[probe].key = key; table[probe].value = value; table[probe].occupied = true; return;
            }
            i++;
        }
    }
    
    Parcel* search(string key) 
    {
        int index = hashFunction(key); int i = 0;
        while (i < capacity) 
        {
            int probe = (index + i * i) % capacity;

            if (!table[probe].occupied) 
            return nullptr;
            
            if (table[probe].key == key) 
            return table[probe].value;
            i++;
        }
        return nullptr;
    }
    void printAll() 
    {
        clearScreen();
        cout << "\n=========================================\n           SYSTEM DATABASE\n=========================================\n";
        cout << "ID     DEST         CAT     ZONE     STATUS\n";
        for(int i=0; i<capacity; i++) 
        if(table[i].occupied) 
        cout << *table[i].value << endl;
        cout << "-----------------------------------------\n";
    }

    void saveToFile(ofstream& out) 
    {
        for(int i=0; i<capacity; i++) if(table[i].occupied) 
        {
            Parcel* p = table[i].value;
            out << p->id << "," << p->destination << "," << p->weight << "," << p->priority << "," << p->status << "," << p->zone << endl;
        }
    }

    void exportJSON(ofstream& out)
    {
        bool first = true;
        for(int i=0; i<capacity; i++)
        {
            if(table[i].occupied)
            {
                if(!first) out << ",\n";
                Parcel* p = table[i].value;
                out << "    { \"id\": \"" << p->id << "\", \"status\": \"" << p->getStatusString() 
                    << "\", \"status_code\": " << p->status << ", \"destination\": \"" << p->destination 
                    << "\", \"zone\": \"" << p->zone << "\", \"rider\": \"" << (p->assignedRider.empty() ? "None" : p->assignedRider) << "\" }";
                first = false;
            }
        }
    }
    
    void runInvestigationProtocol() 
    {
        int resolved = 0;
        for(int i=0; i<capacity; i++) 
        {
            if(table[i].occupied && table[i].value->status == 9) // STATUS_UNDER_INVESTIGATION
            {
                Parcel* p = table[i].value;
                resolved++;
                
                cout << "   [CASE #" << p->id << "] Investigating...";
                
                // 30% Recovery Chance
                int roll = rand() % 100;
                if(roll < 30) 
                {
                    setColor(COLOR_GREEN); cout << " FOUND!\n"; resetColor();
                    p->updateStatus(1, "Parcel Recovered - Returned to Warehouse", "Warehouse"); // STATUS_WAREHOUSE
                    cout << "     >> Parcel Restored to Active Duty.\n";
                }
                else 
                {
                    setColor(COLOR_RED); cout << " CONFIRMED LOST.\n"; resetColor();
                    p->updateStatus(7, "Investigation Concluded - DECLARED LOST", "Unknown"); // STATUS_MISSING
                    
                    // Refund Calculation
                    double refund = (150.0 + (p->weight * 20.0)) * 1.5; // Base + Weight * 1.5 Insurance
                    cout << "     >> Generating Insurance Claim...\n";
                    setColor(COLOR_YELLOW);
                    cout << "     >> COMPENSATION VOUCHER GENERATED: Rs. " << (int)refund << ".00\n";
                    resetColor();
                }
                cout << "\n";
            }
        }
        
        if(resolved == 0) cout << "   >> No pending investigations found.\n";
    }
};


struct Edge 
{
    int dest;
    int weight;
    bool isBlocked;
    
    Edge(int d=0, int w=0) : dest(d), weight(w), isBlocked(false) {}
};


class EdgeArrayList 
{
private:
    Edge* data; 
    int capacity; 
    int count;
    
    void resize(int newCapacity) 
    {
        Edge* newData = new Edge[newCapacity]; 
        for(int i=0; i<count; i++) 
        newData[i] = data[i];
        
        delete[] data; 
        data = newData; 
        capacity = newCapacity;
    }

public:
    
    EdgeArrayList() : capacity(5), count(0) 
    { 
        data = new Edge[capacity]; 
    }
    
    
    EdgeArrayList(const EdgeArrayList& other) 
    {
        capacity = other.capacity; 
        count = other.count; 
        data = new Edge[capacity];
        
        for(int i=0; i<count; i++) 
        data[i] = other.data[i];
    }
    
    EdgeArrayList& operator=(const EdgeArrayList& other) 
    {
        if(this != &other) 
        {
            delete[] data; 
            capacity = other.capacity; 
            count = other.count; 
            data = new Edge[capacity];
            
            for(int i=0; i<count; i++) 
            data[i] = other.data[i];
        }
        return *this;
    }

    ~EdgeArrayList() 
    { 
        delete[] data; 
    }
    
    void add(Edge val) 
    { 
        if (count == capacity) 
        resize(capacity * 2); 
        data[count++] = val; 
    }
    
    int size() const 
    { 
        return count; 
    }
    
    // Get by value (safe)
    Edge get(int index) const 
    {
        if(index >= 0 && index < count) return data[index];
        return Edge(); // Return default Edge if out of bounds
    }
    
    // Get by reference (with bounds checking)
    Edge& getRef(int i) 
    { 
        if(i < 0 || i >= count) 
        {
            // Return first element as fallback (safer than crash)
            // In production, should throw exception or handle error properly
            static Edge fallback;
            return fallback;
        }
        return data[i]; 
    }
};

struct CityNode 
{ 
    string name;
    string zone; 
    EdgeArrayList edges; 
    
    CityNode(string n="", string z="") : name(n), zone(z) {} 
};

class MapGraph 
{
public:
    CityNode* cities; int cityCount; int cityCapacity;
    
    // Store found paths for user selection (Max 5 paths)
    IntArrayList availablePaths[5]; 
    int availablePathDistances[5];
    int pathCount;

    MapGraph() : cityCount(0), cityCapacity(15) 
    { 
        cities = new CityNode[cityCapacity]; 
    }
    
    // Destructor to prevent memory leak
    ~MapGraph() 
    {
        delete[] cities;
    }

    
    int addCity(string name, string zone) 
    { 
        if (cityCount == cityCapacity) 
        return -1; 
        cities[cityCount] = CityNode(name, zone); 
        return cityCount++; 
    }
    
    void addRoad(int u, int v, int dist) 
    { 
        cities[u].edges.add(Edge(v, dist)); 
        cities[v].edges.add(Edge(u, dist)); 
    }
    
    int getCityIndex(string name) 
    { 
        for(int i=0; i<cityCount; i++) 
        if (cities[i].name == name) 
        return i; 
        
        return -1; 
    }
    
    string getZone(string name) 
    {
        int idx = getCityIndex(name);
        if(idx != -1) 
        return cities[idx].zone;

        return "Unknown";
    }

    void blockRandomRoad() 
    {
        if (cityCount < 2) 
        return;
        int u = rand() % cityCount;
        if (cities[u].edges.size() > 0) 
        {
            int eIdx = rand() % cities[u].edges.size();
            cities[u].edges.getRef(eIdx).isBlocked = true;
            cout << ">>> [ALERT] Road near " << cities[u].name << " is now BLOCKED!\n";
        }
    }

    void displayNetwork() 
    {
        clearScreen();
        cout << "\n=========================================\n      LOGISTICS NETWORK MAP VIEW\n=========================================\n";
        for(int i=0; i<cityCount; i++) 
        {
            cout << " [" << cities[i].zone << "] " << left << setw(12) << cities[i].name << " connects to:\n";
            EdgeArrayList& edges = cities[i].edges;
            
            for(int k=0; k<edges.size(); k++) 
            {
                Edge& e = edges.getRef(k);
                cout << "    --> " << setw(12) << cities[e.dest].name << " | " << e.weight << "km";
                if(e.isBlocked) cout << " [BLOCKED]";
                cout << endl;
            }
            cout << "-----------------------------------------\n";
        }
    }

    void findAllPaths(int start, int end) 
    {
        bool* visited = new bool[cityCount];
        for(int i=0; i<cityCount; i++) 
        visited[i] = false;
        pathCount = 0;
        IntArrayList currentPath;
        
        solveDFS(start, end, visited, currentPath, 0);
        
        delete[] visited;
    }

    void solveDFS(int u, int d, bool visited[], IntArrayList currentPath, int currentDist) 
    {
        visited[u] = true;
        currentPath.add(u);
        
        if (u == d) 
        {
             if (pathCount < 5) 
             {
                availablePaths[pathCount] = currentPath; 
                availablePathDistances[pathCount] = currentDist;
                pathCount++;
            }
        } 
        else 
        {
            EdgeArrayList& edges = cities[u].edges;
            for(int i=0; i<edges.size(); i++) 
            {
                Edge& e = edges.getRef(i);
                if(!visited[e.dest] && !e.isBlocked) 
                {
                    solveDFS(e.dest, d, visited, currentPath, currentDist + e.weight);
                }
            }
        }
        visited[u] = false;
    }
    
    int getMinRouteIndex() 
    {
        int minIdx = -1;
        int minDist = INT_MAX;
        for(int i=0; i<pathCount; i++) 
        {
            if(availablePathDistances[i] < minDist) 
            {
                minDist = availablePathDistances[i];
                minIdx = i;
            }
        }
        return minIdx;
    }
    
    // Toggle Road Block Status
    void setRoadStatus(int u, int v, bool blocked) 
    {
        // Block edges in both directions (undirected graph)
        EdgeArrayList& edges1 = cities[u].edges;
        for(int i=0; i<edges1.size(); i++) 
        {
            if(edges1.getRef(i).dest == v) 
            {
                edges1.getRef(i).isBlocked = blocked;
            }
        }
        
        EdgeArrayList& edges2 = cities[v].edges;
        for(int i=0; i<edges2.size(); i++) 
        {
            if(edges2.getRef(i).dest == u) 
            {
                edges2.getRef(i).isBlocked = blocked;
            }
        }
    }
    
    void printGraph() 
    {
        for (int i = 0; i < cityCount; i++) 
        {
            cout << " [" << i << "] " << cities[i].name << " (" << cities[i].zone << ") -> ";
            EdgeArrayList& edges = cities[i].edges;
            for (int j = 0; j < edges.size(); j++) 
            {
                Edge e = edges.getRef(j);
                cout << cities[e.dest].name << "(" << e.weight << "km)" << (e.isBlocked ? "[BLOCKED] " : " ");
            }
            cout << endl;
        }
    }
    
    void saveToFile(ofstream& out) 
    {
        out << cityCount << endl;
        for(int i=0; i<cityCount; i++) 
        {
            out << cities[i].name << "," << cities[i].zone << endl;
        }
        
        for(int i=0; i<cityCount; i++) 
        {
            EdgeArrayList& edges = cities[i].edges;
            for(int j=0; j<edges.size(); j++) 
            {
                Edge& e = edges.getRef(j);
                if (i < e.dest) // Store each road only once
                { 
                     out << cities[i].name << "," << cities[e.dest].name << "," << e.weight << endl;
                }
            }
        }
    }
};

struct User 
{
    string username;
    string password;
    string role; // "ADMIN" or "CUSTOMER"
    
    User(string u="", string p="", string r="CUSTOMER") : username(u), password(p), role(r) {}
};

struct UserEntry 
{ 
    string key; 
    User* value; 
    bool occupied; 
    UserEntry() : key(""), value(nullptr), occupied(false) {} 
};

class UserHashTable 
{
private:
    UserEntry* table; 
    int capacity;
    
    int hashFunction(const string& key) 
    {
        unsigned long hash = 5381;
        for (char c : key) hash = ((hash << 5) + hash) + c; 
        return hash % capacity;
    }

public:
    UserHashTable(int cap = 101) : capacity(cap) 
    { 
        table = new UserEntry[capacity]; 
    }
    
    ~UserHashTable() 
    {
        // Delete all User objects before deleting table
        for(int i=0; i<capacity; i++) 
        {
            if(table[i].occupied && table[i].value) 
            {
                delete table[i].value;
            }
        }
        delete[] table; 
    }
    
    void insert(string username, User* user) 
    {
        int index = hashFunction(username); int i = 0;
        while (i < capacity) 
        {
            int probe = (index + i * i) % capacity;
            if (!table[probe].occupied || table[probe].key == username) 
            {
                table[probe].key = username; table[probe].value = user; table[probe].occupied = true; return;
            }
            i++;
        }
    }
    
    User* search(string username) 
    {
        int index = hashFunction(username); int i = 0;
        while (i < capacity) 
        {
            int probe = (index + i * i) % capacity;
            if (!table[probe].occupied) return nullptr;
            if (table[probe].key == username) return table[probe].value;
            i++;
        }
        return nullptr;
    }
    
    void saveToFile(ofstream& out) 
    {
         for(int i=0; i<capacity; i++) 
         {
             if(table[i].occupied) 
             {
                 User* u = table[i].value;
                 out << u->username << "," << u->password << "," << u->role << endl;
             }
         }
    }
};


struct UndoAction 
{ 
    string type; 
    string parcelId; 
};

struct UndoNode 
{ 
    UndoAction data; 
    UndoNode* next; 
    UndoNode(UndoAction d) : data(d), next(nullptr) {} 
};

class ActionStack 
{
private:
    UndoNode* topNode;

public:

    ActionStack() : topNode(nullptr) {}
    
    void push(string type, string id) 
    { 
        UndoNode* n = new UndoNode({type, id}); 
        n->next = topNode; 
        topNode = n; 
    }
    
    bool pop(UndoAction& dest) 
    {
        if(!topNode) 
        return false;
        
        UndoNode* temp = topNode; 
        dest = temp->data; 
        topNode = topNode->next; 
        delete temp; 
        
        return true;
    }
};

class LogisticsEngine 
{
private:
    ParcelHashTable database;
    ParcelHeap sortingQueue;   
    ParcelLinkedList shippingList; 
    RiderQueue riderQueue;     
    MapGraph map;
    ActionStack undoStack;
    UserHashTable users;
    User* currentUser;

    
public:
    LogisticsEngine() : currentUser(nullptr)
    {
        srand(time(0));
        setupUsers();
        setupRiders();
        
        // Try loading all data
        if(!loadAllData()) 
        {
            setupDefaultMap(); // Fallback if no data file
        }
    }
    
    void setupUsers() 
    {
        // Default accounts
        users.insert("admin", new User("admin", "admin", "ADMIN"));
        users.insert("user", new User("user", "user", "CUSTOMER"));
    }
    
    // Returns true if login successful
    bool login() 
    {
        clearScreen();
        cout << "\n=== LOGIN ===\n";
        string u, p;
        cout << "Username: "; cin >> u;
        cout << "Password: "; cin >> p;
        
        User* found = users.search(u);
        if (found && found->password == p) 
        {
            currentUser = found;
            cout << ">> Welcome, " << u << " (" << found->role << ")!\n";
            pauseFunc();
            return true;
        }
        cout << ">> Invalid Credentials!\n";
        pauseFunc();
        return false;
    }
    
    void registerUser() 
    {
        clearScreen();
        cout << "\n=== REGISTER NEW ACCOUNT ===\n";
        string u, p, r;
        cout << "Username: "; cin >> u;
        if (users.search(u)) 
        {
            cout << ">> Username already exists!\n";
            pauseFunc();
            return;
        }
        cout << "Password: "; cin >> p;
        // Default to CUSTOMER, only Admin can create Admins conceptually, but for now simple registration
        users.insert(u, new User(u, p, "CUSTOMER"));
        cout << ">> Account Created! Please Login.\n";
        pauseFunc();
    }
    
    void logout() 
    {
        currentUser = nullptr;
    }

    bool isAdmin() { return currentUser && currentUser->role == "ADMIN"; }
    
    // --- GUI EXPORT HELPERS ---
    
    // Returns true if city is "known" and has fixed hardcoded coordinates
    bool getStaticCoordinates(string name, int& x, int& y)
    {
        if(name == "Lahore")      { x = 500; y = 220; return true; }
        else if(name == "Islamabad") { x = 460; y = 70; return true; }
        else if(name == "Karachi")   { x = 200; y = 500; return true; } 
        else if(name == "Peshawar")  { x = 400; y = 50; return true; }
        else if(name == "Multan")    { x = 420; y = 320; return true; }
        else if(name == "Faisalabad"){ x = 460; y = 250; return true; }
        else if(name == "Quetta")    { x = 150; y = 320; return true; }
        else if(name == "Hafizabad") { x = 470; y = 200; return true; }
        else if(name == "Rawalpindi"){ x = 460; y = 80; return true; }
        else if(name == "Sukkur")    { x = 280; y = 420; return true; }
        return false;
    }

    struct VisNode { int x, y; bool resolved; };

    void exportToJSON()
    {
        ofstream out("monitor_data.json");
        if(!out) return;

        
        VisNode* coords = new VisNode[map.cityCount];
        
        
        for(int i=0; i<map.cityCount; i++) 
        {
            if(getStaticCoordinates(map.cities[i].name, coords[i].x, coords[i].y))
                coords[i].resolved = true;
            else
                coords[i].resolved = false;
        }
        
        
        
        for(int pass=0; pass<3; pass++)
        {
            for(int i=0; i<map.cityCount; i++)
            {
                if(!coords[i].resolved)
                {
                    // Check neighbors
                    EdgeArrayList& edges = map.cities[i].edges;
                    for(int j=0; j<edges.size(); j++)
                    {
                        int neighborIdx = edges.getRef(j).dest;
                        if(coords[neighborIdx].resolved)
                        {
                            
                            int distKm = edges.getRef(j).weight;
                            
                            
                            double angle = (i * 1.0) + (neighborIdx * 0.5); 
                            
                            
                            double pxDist = distKm * 0.6;
                            
                            coords[i].x = coords[neighborIdx].x + (int)(cos(angle) * pxDist);
                            coords[i].y = coords[neighborIdx].y + (int)(sin(angle) * pxDist);
                            
                            
                            if(coords[i].x < 50) coords[i].x = 50;
                            if(coords[i].x > 750) coords[i].x = 750;
                            if(coords[i].y < 50) coords[i].y = 50;
                            if(coords[i].y > 550) coords[i].y = 550;
                            
                            coords[i].resolved = true;
                            break; 
                        }
                    }
                }
            }
        }
        
        
         for(int i=0; i<map.cityCount; i++)
         {
             if(!coords[i].resolved)
             {
                 coords[i].x = 100 + (rand() % 600);
                 coords[i].y = 100 + (rand() % 400);
                 coords[i].resolved = true;
             }
         }

        out << "{\n";
        
        // 1. Export Cities
        out << "  \"cities\": [\n";
        for(int i=0; i<map.cityCount; i++)
        {
            out << "    { \"name\": \"" << map.cities[i].name << "\", \"zone\": \"" << map.cities[i].zone 
                << "\", \"x\": " << coords[i].x << ", \"y\": " << coords[i].y << " }";
            if(i < map.cityCount - 1) out << ",";
            out << "\n";
        }
        out << "  ],\n";

        delete[] coords; // Clean up

        // 2. Export Roads
        out << "  \"roads\": [\n";
        bool first = true;
        for(int i=0; i<map.cityCount; i++)
        {
            EdgeArrayList& edges = map.cities[i].edges;
            for(int j=0; j<edges.size(); j++)
            {
                Edge& e = edges.getRef(j);
                if(i < e.dest) // Undirected, only print once
                {
                    if(!first) out << ",\n";
                    out << "    { \"u\": \"" << map.cities[i].name << "\", \"v\": \"" << map.cities[e.dest].name 
                        << "\", \"weight\": " << e.weight << ", \"blocked\": " << (e.isBlocked ? "true" : "false") << " }";
                    first = false;
                }
            }
        }
        out << "\n  ],\n";

        // 3. Export Riders
        out << "  \"riders\": [\n";
        riderQueue.exportJSON(out);
        out << "\n  ],\n";

        // 4. Export Parcels
        out << "  \"parcels\": [\n";
        database.exportJSON(out);
        out << "\n  ]\n";
        
        out << "}\n";
        out.close();
    }

    void addCityInteractive()
    {
        if(!isAdmin()) { cout << "Access Denied.\n"; pauseFunc(); return; }
        clearScreen();
        string name, zone;
        cout << "Enter City Name: "; cin.ignore(); getline(cin, name);
        cout << "Enter Zone (Zone A/B/C/D): "; getline(cin, zone);
        
        if(map.addCity(name, zone) != -1) 
             cout << ">> City Added.\n";
        else 
             cout << ">> Map Full!\n";
        pauseFunc();
    }
    
    void addRoadInteractive() 
    {
        if(!isAdmin()) { cout << "Access Denied.\n"; pauseFunc(); return; }
        clearScreen();
        cout << "Available Cities:\n";
        for(int i=0; i<map.cityCount; i++) cout << i << ". " << map.cities[i].name << "\n";
        
        int u, v, w;
        cout << "Enter Source ID: "; cin >> u;
        cout << "Enter Dest ID: "; cin >> v;
        cout << "Enter Distance (km): "; cin >> w;
        
        if(u>=0 && u<map.cityCount && v>=0 && v<map.cityCount) 
        {
            map.addRoad(u, v, w);
            cout << ">> Road Added.\n";
        }
        else cout << ">> Invalid Cities.\n";
        pauseFunc();
    }
    
    // NEW: Block Road
    void blockRoadInteractive() 
    {
        if(!isAdmin()) { cout << "Access Denied.\n"; pauseFunc(); return; }
        clearScreen();
        cout << "Current Map Status:\n";
        map.printGraph();
        
        int u, v, status;
        cout << "\nToggle Road Block:\n";
        cout << "Enter City A ID: "; cin >> u;
        cout << "Enter City B ID: "; cin >> v;
        cout << "Block(1) or Unblock(0)? "; cin >> status;
        
        if(u>=0 && u<map.cityCount && v>=0 && v<map.cityCount) 
        {
            map.setRoadStatus(u, v, (status==1));
            cout << ">> Road Status Updated.\n";
        }
        else cout << ">> Invalid IDs.\n";
        pauseFunc();
    }
    
    // NEW: Report Missing with Advanced Features
    void reportMissingInteractive() 
    {
        if(!isAdmin()) { cout << "Access Denied.\n"; pauseFunc(); return; }
        clearScreen();
        printBanner();
        string id, reason;
        
        cout << "\n   [ REPORT MISSING PARCEL ]\n";
        cout << "   Enter Missing Parcel ID: "; cin >> id;
        cout << "   Enter Reason (e.g. Theft, Lost Signal): "; cin.ignore(); getline(cin, reason);
        
        Parcel* p = database.search(id);
        if(p) 
        {
            // 1. Log the Incident
            p->updateStatus(STATUS_UNDER_INVESTIGATION, "Reported Missing: " + reason, "Unknown");
            
            // 2. Operational Cleanup: Free the Rider
            if (p->assignedRider != "") 
            {
               releaseRider(p->assignedRider);
               setColor(COLOR_YELLOW);
               cout << "   >> Alert: Rider capacity freed for " << p->assignedRider << ".\n";
               p->assignedRider = ""; // Unassign
               resetColor();
            }
            
            setColor(COLOR_RED);
            cout << "   >> Parcel marked as UNDER INVESTIGATION.\n";
            resetColor();
        }
        else 
        {
            setColor(COLOR_RED); cout << "   >> Parcel Not Found.\n"; resetColor();
        }
        pauseFunc();
    }
    
    void resolveInvestigations()
    {
        if(!isAdmin()) { cout << "Access Denied.\n"; pauseFunc(); return; }
        clearScreen();
        printBanner();
        
        cout << "\n   [ CASE FILE RESOLUTIONS ]\n";
        cout << "   Reviewing active investigations...\n\n";
        
        int resolvedCount = 0;

        
        database.runInvestigationProtocol();
        
        pauseFunc();
    }

    double calculateShippingCost(double weight, double dist, int priority) 
    {
        double base = 150.0;
        double weightCost = weight * 20.0;
        double distCost = dist * 0.5;
        double total = base + weightCost + distCost;
        if(priority == 2) total *= 1.2;
        if(priority == 3) total *= 1.5; // Urgent
        return total;
    }

    void setupRiders() 
    {
        
        riderQueue.enqueue(Rider("Ali Khan", 1, 1));      // Standard
        riderQueue.enqueue(Rider("Bilal Ahmed", 1, 1));   // Standard
        riderQueue.enqueue(Rider("Asad Malik", 1, 2));    // Express
        riderQueue.enqueue(Rider("Usman Tariq", 1, 2));   // Express
        riderQueue.enqueue(Rider("Fahad Mustafa", 1, 1)); // Standard
        riderQueue.enqueue(Rider("Ahmed Raza", 1, 1));    // Standard
        riderQueue.enqueue(Rider("Imran Haider", 1, 2));  // Express
        riderQueue.enqueue(Rider("Zain Sheikh", 1, 2));   // Express
        riderQueue.enqueue(Rider("Hamza Ali", 1, 1));     // Standard
        riderQueue.enqueue(Rider("Hassan Bib", 1, 1));    // Standard
    }
    
    // Display Functions
    void displayRiders()
    {
        clearScreen();
        riderQueue.displayAll();
        pauseFunc();
    }
    
    // Helper to free up rider capacity
    void releaseRider(string name)
    {
        riderQueue.decrementLoad(name);
    }

    void setupDefaultMap() 
    {
        int lhr = map.addCity("Lahore", "Zone A"); 
        int isb = map.addCity("Islamabad", "Zone B"); 
        int khi = map.addCity("Karachi", "Zone C");
        int pew = map.addCity("Peshawar", "Zone B"); 
        int mux = map.addCity("Multan", "Zone A"); 
        int fsd = map.addCity("Faisalabad", "Zone A");
        int qta = map.addCity("Quetta", "Zone D"); 
        int hfd = map.addCity("Hafizabad", "Zone A"); 
        int rwp = map.addCity("Rawalpindi", "Zone B");
        int suk = map.addCity("Sukkur", "Zone C");
        
        map.addRoad(lhr, isb, 375); 
        map.addRoad(lhr, fsd, 180); 
        map.addRoad(lhr, mux, 345); 
        map.addRoad(lhr, hfd, 105);
        map.addRoad(isb, pew, 155); 
        map.addRoad(isb, rwp, 20); 
        map.addRoad(fsd, hfd, 90); 
        map.addRoad(fsd, mux, 240);
        map.addRoad(mux, suk, 490); 
        map.addRoad(suk, khi, 470); 
        map.addRoad(suk, qta, 390); 
        map.addRoad(qta, khi, 690);

    }
    
    void requestPickup(string id, string dest, double w, int p) 
    {
        // Input validation
        if(w <= 0) 
        {
            clearScreen();
            cout << "Error: Weight must be greater than 0.\n";
            pauseFunc();
            return;
        }
        
        if(p < 1 || p > 3) 
        {
            clearScreen();
            cout << "Error: Priority must be 1 (Low), 2 (Medium), or 3 (High).\n";
            pauseFunc();
            return;
        }
        
        if(id.empty()) 
        {
            clearScreen();
            cout << "Error: Parcel ID cannot be empty.\n";
            pauseFunc();
            return;
        }
        
        // Interactive City Selection if dest is empty or invalid
        if(dest == "" || map.getCityIndex(dest) == -1) 
        {
             clearScreen();
             cout << ">>> SELECT DESTINATION CITY:\n";
             for(int i=0; i<map.cityCount; i++) 
             {
                 cout << " " << i << ". " << map.cities[i].name << " (" << map.cities[i].zone << ")\n";
             }
             int choice;
             cout << "Enter City ID: "; cin >> choice;
             if(choice >=0 && choice < map.cityCount) 
                 dest = map.cities[choice].name;
             else 
             {
                 setColor(COLOR_RED); cout << "Invalid City.\n"; resetColor(); pauseFunc(); return;
             }
        }
    
        clearScreen();
        if (database.search(id)) 
        {
            setColor(COLOR_RED); cout << "Error: ID exists.\n"; resetColor();
            pauseFunc(); 
            return; 
        }
        
        // Calculate Cost
        int start = map.getCityIndex("Lahore"); // Assume picked up from HQ for now
        int end = map.getCityIndex(dest);
        double dist = 0;
        
        map.findAllPaths(start, end);
        if(map.pathCount > 0) dist = map.availablePathDistances[map.getMinRouteIndex()];
        
        double cost = calculateShippingCost(w, dist, p);
        
        cout << "\n--------------------------------\n";
        cout << "      SHIPPING QUOTE\n";
        cout << "--------------------------------\n";
        cout << " Source:      Lahore (Central Warehouse)\n";
        cout << " Destination: " << dest << "\n";
        cout << " Distance:    " << dist << " km\n";
        cout << " Weight:      " << w << " kg\n";
        cout << " Priority:    " << p << "\n";
        cout << " TOTAL COST:  Rs. " << cost << "\n";
        cout << "--------------------------------\n";
        cout << "Confirm Pickup Request? (y/n): ";
        char c; cin >> c;
        if(c != 'y' && c != 'Y') { cout << "Cancelled.\n"; pauseFunc(); return; }

        string zone = map.getZone(dest);
        Parcel* newP = new Parcel(id, dest, w, p, zone);
        database.insert(id, newP); 
        
        // Link parcel to user history (simulated by simple log for now, or could add list to User)
        newP->history->addEvent("Created by User: " + (currentUser ? currentUser->username : "Unknown"), "Portal");

        newP->updateStatus(STATUS_WAREHOUSE, "Arrived at Warehouse", "Central Hub");
        
        sortingQueue.insert(newP); 
        
        undoStack.push("ADD", id);
        cout << ">> Pickup Request Logged.\n";
        pauseFunc();
    }


    void processNext() 
    {
        if(!isAdmin()) { cout << "Access Denied.\n"; pauseFunc(); return; }
        
        clearScreen();
        if(sortingQueue.isEmpty()) 
        { 
            cout << ">> Warehouse Queue Empty.\n"; 
            pauseFunc(); 
            return; 
        }
        
        Parcel* p = sortingQueue.extractMax();
        if(!p) 
        {
             cout << "Warehouse Queue is Empty.\n";
             pauseFunc();
             return;
        }
        
        // Use Intelligent Rider Selection Algorithm
        cout << ">> Analyzing best rider for parcel " << p->id << "...\n";
        
        Rider* r = riderQueue.findAvailableRider(p->priority);
        
        if(!r) 
        {
            cout << "No Suitable Riders Available! (All busy or at capacity)\n";
            sortingQueue.insert(p); // Put back
            pauseFunc();
            return;
        }
        
        // Display selection
        cout << "✓ Selected Rider: " << r->name << " (" << ((r->priorityLevel==2)?"Express":"Standard") << ")\n";
        cout << "  Current Load: " << r->currentLoad << "/" << r->capacity << " parcels\n\n";
        
        // Find and display all available routes
        int start = map.getCityIndex("Lahore");
        int end = map.getCityIndex(p->destination);
        
        cout << ">> Calculating routes for " << p->id << " to " << p->destination << "...\n";
        map.findAllPaths(start, end);
        
        if (map.pathCount == 0) 
        {
            cout << ">> ALERT: No paths available. Returning to Sender.\n";
            p->updateStatus(STATUS_RETURNED, "No Route Available", "Warehouse");
            pauseFunc();
            return;
        }
        
        // Display all available routes
        int minIdx = map.getMinRouteIndex();
        cout << "\n========================================\n";
        cout << "     AVAILABLE ROUTES TO " << p->destination << "\n";
        cout << "========================================\n";
        
        for(int i=0; i<map.pathCount; i++) 
        {
            cout << "\n[Route " << i << "] ";
            if(i == minIdx) cout << "⭐ RECOMMENDED - SHORTEST";
            cout << "\n";
            cout << "  Distance: " << map.availablePathDistances[i] << " km\n";
            cout << "  Path: ";
            
            IntArrayList& path = map.availablePaths[i];
            for(int j=0; j<path.size(); j++) 
            {
                cout << map.cities[path.get(j)].name;
                if(j < path.size()-1) cout << " -> ";
            }
            cout << "\n";
        }
        
        cout << "========================================\n";
        
        // User selects route
        int choice;
        cout << "\n>> Select Route ID to Dispatch [0-" << (map.pathCount-1) << "] (Recommended: " << minIdx << "): ";
        cin >> choice;
        
        if (choice < 0 || choice >= map.pathCount) 
        {
            cout << "Invalid selection. Using Recommended Route.\n";
            choice = minIdx;
        }
        
        // Check for random road blockage
        if (rand() % 10 < 2) 
        { 
            cout << "\n>>> [LIVE UPDATE] Road Blockage Detected on selected route!\n";
            map.blockRandomRoad();
            cout << ">>> Re-calculating Best Route automatically...\n";
            map.findAllPaths(start, end); 
            
            if (map.pathCount > 0) 
            {
                choice = map.getMinRouteIndex(); 
                cout << ">>> Rerouted to new optimal path.\n";
            } 
            else 
            {
                 cout << ">>> Rerouting Failed. Returning to Sender.\n";
                 p->updateStatus(STATUS_RETURNED, "Dynamic Blockage - RTS", "Warehouse");
                 pauseFunc();
                 return;
            }
        }
        
        // Assign to rider
        r->currentLoad++;
        
        p->assignedRider = r->name;
        // Simplified status update
        p->updateStatus(STATUS_LOADING, "Loaded onto " + r->name + "'s Vehicle", "Warehouse");
        shippingList.pushBack(p);
        
        long long travelSecs = 10 + (rand() % 21);
        p->dispatchTime = time(0);
        p->arrivalTime = time(0) + travelSecs + 5;
        
        undoStack.push("DISPATCH", p->id);
        
        cout << "\n";
        setColor(COLOR_GREEN); cout << ">> PARCEL DISPATCHED SUCCESSFULLY.\n"; resetColor();
        cout << "   Parcel: " << p->id << " (Priority " << p->priority << ", " << p->weight << " kg)\n";
        cout << "   Rider: " << r->name << "\n";
        cout << "   Route: " << choice << " (Distance: " << map.availablePathDistances[choice] << " km)\n";
        cout << "   ETA: " << travelSecs << "s (transit) + 5s (loading)\n";
        cout << "   Rider Load: ";
        if(r->currentLoad >= r->capacity) setColor(COLOR_RED); else setColor(COLOR_GREEN);
        cout << r->currentLoad << "/" << r->capacity; resetColor(); cout << " parcels\n";
        
        pauseFunc();
    }

    void showMap() 
    { 
        map.displayNetwork(); 
        pauseFunc(); 
    }

    void undoLast() 
    {
        clearScreen();
        UndoAction act;
        if(undoStack.pop(act)) 
        {
            Parcel* p = database.search(act.parcelId);
            if(p) 
            {
                if(act.type == "ADD") 
                {
                    p->updateStatus(STATUS_CANCELLED, "Undo: Creation Reverted", "N/A");
                    cout << ">> Undo Add: Parcel " << p->id << " marked as cancelled.\n";
                } 
                else if (act.type == "DISPATCH") 
                {
                    p->updateStatus(STATUS_WAREHOUSE, "Undo: Dispatch Reverted", "Warehouse");
                    p->arrivalTime = 0; sortingQueue.insert(p); 
                    cout << ">> Undo Dispatch: Parcel " << p->id << " returned to warehouse.\n";
                }
            }
        } 
        else 
        { 
            cout << ">> Nothing to undo.\n"; 
        }
        
        pauseFunc();
    }

    void updateRealTime() 
    {
        long long now = time(0);
        shippingList.updateLifecycle(now, this);
    }
    
    void liveMonitor() 
    {
        cout << ">> Entering Live Monitor Mode. Press 'x' to exit.\n";
        while (true) 
        {
            if (_kbhit()) 
            {
                char ch = _getch();
                if (ch == 'x' || ch == 'X') break;
            }

            clearScreen();
            long long now = time(0);
            updateRealTime(); // Process lifecycle updates
            shippingList.showTransitStatus(now); // Show CLI status
            exportToJSON(); // Update JSON for HTML monitor
            
            cout << "\n[LIVE MONITOR ACTIVE] - Updating every second...\n";
            cout << "Press 'x' to return to menu.\n";
            
            Sleep(1000); // 1-second refresh rate
        }
    }

    void viewParcel(string id) 
    {
        clearScreen();
        printBanner();
        Parcel* p = database.search(id);
        if(p) 
        {
            setColor(COLOR_CYAN);
            cout << "\n .------------------------------------------------.\n";
            cout << " |            PARCEL STATUS REPORT                |\n";
            cout << " '------------------------------------------------'\n";
            resetColor();
            cout << "  ID:          " << p->id << "\n";
            cout << "  DESTINATION: " << p->destination << " (" << p->zone << ")\n";
            cout << "  WEIGHT:      " << p->weight << " kg (" << p->weightCategory << ")\n";
            cout << "  PRIORITY:    " << p->priority << "\n";
            cout << "  STATUS:      ["; 
            
            if(p->status == STATUS_DELIVERED) setColor(COLOR_GREEN);
            else if(p->status == STATUS_MISSING || p->status == STATUS_RETURNED || p->status == STATUS_CANCELLED) setColor(COLOR_RED);
            else if(p->status == STATUS_UNDER_INVESTIGATION) { setColor(COLOR_RED); }
            else setColor(COLOR_YELLOW);
            
            cout << p->getStatusString();
            resetColor();
            cout << "]\n";
            
            if(p->status == STATUS_UNDER_INVESTIGATION)
            {
                 setColor(COLOR_YELLOW);
                 cout << "\n   [!] This parcel is currently under ACTIVE INVESTIGATION.\n";
                 cout << "   [!] Please contact support or wait for case resolution.\n";
                 resetColor();
            }
            
            setColor(COLOR_CYAN);
            cout << " --------------------------------------------------\n";
            resetColor();
            
            p->history->printTimeline();
            
            if (p->status == STATUS_IN_TRANSIT || p->status == STATUS_LOADING) 
            {
                long long now = time(0);
                long long rem = p->arrivalTime - now;
                cout << "\n";
                if(rem > 0) {
                     cout << "   >>> EST. DELIVERY IN: ";
                     setColor(COLOR_YELLOW); cout << rem << " seconds"; resetColor(); cout << "\n";
                }
                else {
                    setColor(COLOR_GREEN); cout << "   >>> ARRIVING MOMENTARILY...\n"; resetColor();
                }
            }
        } 
        else 
        { 
            setColor(COLOR_RED);
            cout << "\n [ERROR] Parcel with ID '" << id << "' not found in the database.\n"; 
            resetColor();
        }
        
        pauseFunc();
    }

    void listAll() 
    { 
        if(!isAdmin()) { cout << "Restricted to Admins.\n"; pauseFunc(); return; }
        database.printAll(); 
        pauseFunc(); 
    }

    
    void saveToFile() 
    {
        ofstream f("logistics_data.txt");
        if(f.is_open()) 
        { 
            f << "---MAP_START---\n";
            map.saveToFile(f);
            
            f << "---USERS_START---\n";
            users.saveToFile(f);
            
            f << "---PARCELS_START---\n";
            database.saveToFile(f); 
            f.close(); 
            cout << "All Data Saved (Map, Users, Parcels).\n"; 
        }
    }

    bool loadAllData() 
    {
        ifstream f("logistics_data.txt");
        if(!f.is_open()) return false;
        
        string line;
        bool processingEdges = false;
        
        // Skip until MAP_START
        while(getline(f, line) && line != "---MAP_START---");
        if(f.eof()) return false;
        
        // Read City Count
        if(!(f >> line)) return false; 
        int cCount = stoi(line);
        f.ignore(); 
        
        // Read Cities
        for(int i=0; i<cCount; i++) 
        {
            string line2, name, zone;
            getline(f, line2);
            stringstream ss(line2);
            getline(ss, name, ','); getline(ss, zone, ',');
            map.addCity(name, zone);
        }
        
        // Read Edges
        while(getline(f, line)) 
        {
            if(line == "---USERS_START---") break;
            
            stringstream ss(line);
            string uName, vName, wStr;
            getline(ss, uName, ','); getline(ss, vName, ','); getline(ss, wStr, ',');
            
            int u = map.getCityIndex(uName);
            int v = map.getCityIndex(vName);
            if(u!=-1 && v!=-1) 
            {
               map.addRoad(u, v, stoi(wStr));
            }
        }
        
        // Read Users
        while(getline(f, line)) 
        {
            if(line == "---PARCELS_START---") break;
            
            stringstream ss(line);
            string user, pass, role;
            getline(ss, user, ','); getline(ss, pass, ','); getline(ss, role, ',');
            if(user!="") users.insert(user, new User(user, pass, role));
        }
        
        // Read Parcels
        string id, dest, temp, zone;
        while(getline(f, line)) 
        {
            stringstream ss(line);
            getline(ss, id, ','); getline(ss, dest, ',');
            getline(ss, temp, ','); double w = stod(temp);
            getline(ss, temp, ','); int p = stoi(temp);
            getline(ss, temp, ','); int s = stoi(temp);
            getline(ss, zone, ',');
            Parcel* newP = new Parcel(id, dest, w, p, zone);
            newP->status = s;
            database.insert(id, newP);
            if(s == STATUS_WAREHOUSE) sortingQueue.insert(newP);
            if(s >= STATUS_LOADING && s <= STATUS_DELIVERY_ATTEMPT) shippingList.pushBack(newP);
        }
            
        f.close();
        return true;
    }
    
    // Legacy support removed
    void loadFromFile() {}


    
    void cancelParcel(string id) 
    {
        clearScreen();
        Parcel* p = database.search(id);
        if(p && p->status <= STATUS_WAREHOUSE) 
        {
            p->updateStatus(STATUS_CANCELLED, "Cancelled by User", "Warehouse");
            cout << ">> Parcel " << id << " cancelled.\n";
        } 
        else 
        cout << ">> Error: Cannot cancel (In Transit or Missing).\n";
        
        pauseFunc();
    }
};

void ParcelLinkedList::updateLifecycle(long long currentTime, LogisticsEngine* engine) 
{
    ParcelNode* curr = head;
    while(curr) 
    {
        Parcel* p = curr->data;
        if (p->status == STATUS_LOADING) 
        {
            if (currentTime >= p->lastUpdateTime + 5) 
            {
                p->updateStatus(STATUS_IN_TRANSIT, "Vehicle Departed", "On Road");
            }
        }
        else if (p->status == STATUS_IN_TRANSIT) 
        {
            if (rand() % 1000 == 0) 
            {
                p->updateStatus(STATUS_MISSING, "Signal Lost - Investigation Started", "Unknown");
                // Update rider: just release capacity
                if(engine && p->assignedRider != "")
                {
                    engine->releaseRider(p->assignedRider);
                }
            }
            else if (currentTime >= p->arrivalTime) 
            {
                p->updateStatus(STATUS_DELIVERY_ATTEMPT, "Arrived at Destination Hub", p->destination);
            }
        }
        else if (p->status == STATUS_DELIVERY_ATTEMPT) 
        {
            if (rand() % 10 < 8) 
            {
                p->updateStatus(STATUS_DELIVERED, "Handed to Recipient", "Doorstep");
                // Update rider: just release capacity
                if(engine && p->assignedRider != "")
                {
                    engine->releaseRider(p->assignedRider);
                }
            } 
            else 
            {
                p->deliveryAttempts++;
                if (p->deliveryAttempts >= 3) 
                {
                    p->updateStatus(STATUS_RETURNED, "Max Attempts Reached - RTS", "Local Hub");
                    // Update rider: just release capacity
                    if(engine && p->assignedRider != "")
                    {
                        engine->releaseRider(p->assignedRider);
                    }
                } 
                else 
                {
                    p->updateStatus(STATUS_IN_TRANSIT, "Recipient Unavailable - Retrying", "Local Hub");
                    p->arrivalTime = currentTime + 5; 
                }
            }
        }
        curr = curr->next;
    }
}

int main() 
{
    // Auto-launch GUI components
    cout << ">> Initializing Logistics Monitor...\n";
    // Start Python Server minimized
    system("start /min python -m http.server 8000");
    // Brief pause to let server bind
    Sleep(1000); 
    // Open Browser
    system("start http://localhost:8000/monitor.html");

    LogisticsEngine engine;
    int choice;
    
    while(true) 
    {
        clearScreen();
        printBanner();
        cout << "    [1] LOGIN\n";
        cout << "    [2] REGISTER NEW ACCOUNT\n";
        cout << "    [3] EXIT SYSTEM\n";
        cout << "\n   Select Option: ";
        
        int accChoice; 
        if(!(cin >> accChoice)) { cin.clear(); cin.ignore(1000, '\n'); continue; }
        
        if(accChoice == 3) {
            cout << "\n   Shutting down systems... Goodbye!\n";
            break;
        }
        if(accChoice == 2) { engine.registerUser(); continue; }
        if(accChoice == 1) 
        {
             if(!engine.login()) continue;
             
             // Main App Loop
             while (true) 
             {
                clearScreen();
                printBanner();
                
                if(engine.isAdmin()) 
                {
                    setColor(COLOR_BLUE);
                    cout << "   ADMIN DASHBOARD           User: admin\n";
                    cout << "   ========================================\n";
                    resetColor();
                    
                    setColor(COLOR_YELLOW); cout << "   [ OPERATIONS ]\n"; resetColor();
                    cout << "    1. New Parcel & Dispatch\n";
                    cout << "    2. Process Warehouse Queue\n";
                    cout << "    3. Cancel Parcel\n";
                    cout << "    4. Undo Last Action\n";
                    cout << "    5. Report Missing Operation\n";
                    cout << "    6. Resolve Pending Investigations\n\n";
                    
                    setColor(COLOR_YELLOW); cout << "   [ MONITORING ]\n"; resetColor();
                    cout << "    7. Track a Parcel\n";
                    cout << "    8. Global Map View\n";
                    cout << "    9. Live Transit Monitor\n";
                    cout << "   10. List All Parcels\n";
                    cout << "   11. Fleet Status (Riders)\n\n";
                    
                    setColor(COLOR_YELLOW); cout << "   [ SYSTEM CONFIG ]\n"; resetColor();
                    cout << "   12. Add New City\n";
                    cout << "   13. Add New Road\n";
                    cout << "   14. Block/Unblock Road\n";
                    
                    setColor(COLOR_RED); cout << "\n   99. LOGOUT\n"; resetColor();
                } 
                else 
                {
                    setColor(COLOR_GREEN);
                    cout << "   CUSTOMER DASHBOARD        User: Customer\n";
                    cout << "   ========================================\n";
                    resetColor();
                    
                    cout << "    1. Request Pickup\n";
                    cout << "    2. Track My Parcel\n";
                    cout << "    3. View Coverage Map\n";
                    setColor(COLOR_RED); cout << "\n   99. LOGOUT\n"; resetColor();
                }
                
                cout << "\n   >> Enter Choice: ";
                if (!(cin >> choice)) 
                { 
                    cin.clear(); cin.ignore(1000, '\n'); continue; 
                }
                
                if(choice == 99) { engine.logout(); break; }
                
                // Map choices to functions
                if(engine.isAdmin())
                {
                    switch(choice) 
                    {
                        case 1: 
                        {
                            string id, dest; double w; int p;
                            cout << "\n   [ NEW PARCEL ]\n";
                            cout << "   ID: "; cin >> id; cin.ignore(1000, '\n'); 
                            cout << "   Destination (Enter for List): "; getline(cin, dest);
                            cout << "   Weight (kg): "; cin >> w; 
                            cout << "   Priority (1=Std, 2=Exp, 3=Urg): "; cin >> p;
                            engine.requestPickup(id, dest, w, p);
                            break;
                        }
                        case 2: engine.processNext(); break;
                        case 3: { string id; cout << "   Enter ID to Cancel: "; cin >> id; engine.cancelParcel(id); break; }
                        case 4: engine.undoLast(); break;
                        case 5: engine.reportMissingInteractive(); break;
                        case 6: engine.resolveInvestigations(); break;
                        
                        case 7: { string id; cout << "   Enter ID to Track: "; cin >> id; engine.viewParcel(id); break; }
                        case 8: engine.showMap(); break;
                        case 9: engine.liveMonitor(); break;
                        case 10: engine.listAll(); break;
                        case 11: engine.displayRiders(); break;
                        
                        case 12: engine.addCityInteractive(); break;
                        case 13: engine.addRoadInteractive(); break;
                        case 14: engine.blockRoadInteractive(); break;
                    }
                }
                else
                {
                    // Customer Menu Mappings
                    switch(choice)
                    {
                        case 1: 
                        {
                            string id, dest; double w; int p;
                            cout << "\n   [ REQUEST PICKUP ]\n";
                            cout << "   ID (Unique): "; cin >> id; cin.ignore(1000, '\n'); 
                            cout << "   Destination: "; getline(cin, dest);
                            cout << "   Weight (kg): "; cin >> w; 
                            cout << "   Priority (1=Std, 2=Exp, 3=Urg): "; cin >> p;
                            engine.requestPickup(id, dest, w, p);
                            break;
                        }
                        case 2: { string id; cout << "   Enter Your Parcel ID: "; cin >> id; engine.viewParcel(id); break; }
                        case 3: engine.showMap(); break;
                    }
                }
                
                if(choice != 99) 
                {
                    engine.updateRealTime();
                    engine.exportToJSON(); // Auto-update monitor
                }
             }
             engine.saveToFile(); // Auto-save on logout
        }
    }
    
    
    return 0;






}
#include <iostream>
#include <fstream>
#include "Stack.h"
#include "Queue.h"
#include <vector>
#include <cmath>

using namespace std;

struct NodeItem{
    int row;
    int col;
    int weight; // weight = abs(x-root.row) + abs(y-root.y)
    NodeItem* parent; // used in shortest-path
    NodeItem(int r, int c, int d):row(r),col(c),weight(d),parent(0){}
    NodeItem operator=(NodeItem* n) {
        this->row = n->row;
        this->col = n->col;
        this->weight = n->weight;
        this->parent = n->parent;
        return *this;
    } 
};

class Robot{
    private:
        NodeItem* root;
        int num_node;
        int row, col, battery;
        int** map;
        bool** mapSP; // original bool map
        bool** mapcopy; // use in BFS
        bool** mapRemain;
        stack<NodeItem*> track;
        queue<NodeItem*> step; 
    public:
        //constructor; initinalize map, battery life, num_node and root node 
        Robot(int r, int c, int b, char** input):row(r),col(c),battery(b){
            // construct map
            map = new int*[r];
            mapSP = new bool*[r];
            mapcopy = new bool*[r];
            mapRemain = new bool*[r];
            for(int i =0; i < r; i++)
            {
                map[i] = new int[c];
                mapSP[i] = new bool[c];
                mapcopy[i] = new bool[c];
                mapRemain[i] = new bool[c];
            }
            // mapping input & map and count nodes
            num_node =0;
            for(int i = 0; i < r; i++) 
            {
                for(int j = 0; j < c; j++)
                {
                    if(input[i][j]=='R')
                    {
                        map[i][j] = 1;
                        mapSP[i][j] = true;
                        root = new NodeItem(i, j, 0);
                        track.push(root);
                        step.push(root);
                    }
                    else if(input[i][j]=='1'){
                        map[i][j] = 2;
                        mapSP[i][j] = false;
                    }
                    else{
                        map[i][j] = 0;
                        num_node++;
                        mapSP[i][j] = true;
                    }
                }
            }
            cout<<"num_node: "<<num_node<<"\nRoot: ("<<track.top()->row<<", "<<track.top()->col<<")"<<" track.size(): "<<track.size()<<endl;
            /*
            1 1 1 1         2 2 2 2 
            1 0 0 1   ===>  2 0 0 2  ===> num_node = 4, not including root
            1 1 0 1         2 2 0 2
            1 0 R 1         2 0 1 2   
            */
        }
        void mapCopyInit();
        bool AllClean();
        void Move();
        void PrintStep();
        void PrintMap();
        NodeItem*** ShortestPath_to_R_BFS(NodeItem* from, NodeItem* to);
        bool Deadend(NodeItem* top);
        bool isValidStep(NodeItem*,int);
        int steps() { return step.size()-1; }
        void outStep(ofstream&);
        NodeItem* bestTravel(NodeItem* current);
        int countZeros(NodeItem*** path, NodeItem* from, NodeItem* to);
        int countSteps(NodeItem*** path, NodeItem* from, NodeItem* to);
        void pushSteps(NodeItem*** path, NodeItem* from, NodeItem* to);
};

bool Robot::Deadend(NodeItem* top) {
     int count = 0;
     int r = top->row;
     int c = top->col;
     if(r-1 >= 0 && map[r-1][c] == 0) count++;
     if(r+1 <= row-1 && map[r+1][c] == 0 ) count++;
     if(c-1 >= 0 && map[r][c-1] == 0 ) count++;
     if(c+1 <= col-1 && map[r][c+1] == 0) count++;
     
     if(count) return false;
     else return true;
 }

void Robot::mapCopyInit() {
    for(int i = 0; i < row; i++) {
        for(int j = 0; j < col; j++) {
            mapcopy[i][j] = mapSP[i][j];
        }
    }
}

// shortest but also record it //return a linked-list
/*
FFFFFF
FTTTTF
FSTTTD
FFFFFF
*/
NodeItem*** Robot::ShortestPath_to_R_BFS(NodeItem* from, NodeItem* to) {
    NodeItem*** path = new NodeItem**[row];
    for(int i = 0; i < row; i++) {
        path[i] = new NodeItem*[col];
    }
    for(int i = 0; i < row; i++) {
        for(int j = 0; j < col; j++) {
            path[i][j] = NULL;
        }
    }
    
    mapCopyInit();
    queue<NodeItem*> bfs;
    stack<NodeItem*> s;

    bfs.push(from);
    while(!bfs.empty()) {
        NodeItem* current = bfs.front();
        int r = current->row;
        int c = current->col;
        mapcopy[r][c] = false;
        bfs.pop();
        
        // case 0 : from == to
        if(r == to->row && c == to->col) { path[r][c] = current; break;}
        
        int weightUp = abs(r-1 - root->row)+ abs(c - root->col);
        int weightDown = abs(r+1 - root->row) + abs(c - root->col);
        int weightLeft = abs(r - root->row) + abs(c-1 - root->col);
        int weightRight = abs(r - root->row) + abs(c+1 - root->col);

        if(r-1 >= 0 && mapcopy[r-1][c] == true ) { NodeItem* up = new NodeItem(r-1, c, weightUp); mapcopy[r-1][c] =false; up->parent = current; path[r-1][c] = up; bfs.push(up); if(r-1 == to->row && c == to->col) break;}
        if(r+1 <= row-1 && mapcopy[r+1][c] == true) { NodeItem* down = new NodeItem(r+1, c, weightDown); mapcopy[r+1][c] = false; down->parent = current; path[r+1][c] = down; bfs.push(down); if(r+1 == to->row && c == to->col) break;} 
        if(c-1 >= 0 && mapcopy[r][c-1] == true ) { NodeItem* left = new NodeItem(r, c-1, weightLeft); mapcopy[r][c-1] = false; left->parent = current; path[r][c-1] = left; bfs.push(left); if(r == to->row && c-1 == to->col) break;}
        if(c+1 <= col-1 && mapcopy[r][c+1] == true) { NodeItem* right = new NodeItem(r, c+1, weightRight); mapcopy[r][c+1] = false; right->parent = current; path[r][c+1] = right; bfs.push(right); if(r == to->row && c+1 == to->col) break;}
    }
    cout<<endl;
    return path;
}
int Robot::countZeros(NodeItem*** path, NodeItem* from, NodeItem* to) {
    NodeItem* check = path[to->row][to->col];    
    int num_of_zero = 0;

    int r = 0;
    int c = 0;
    while(check != from) {
        r = check->row;
        c = check->col;
        if (map[r][c] == 0) {
            num_of_zero++;
        }
        check = check->parent; 
    }
    return num_of_zero;
}
int Robot::countSteps(NodeItem*** path, NodeItem* from, NodeItem* to) {
    NodeItem* check = path[to->row][to->col];    
    int steps = 0;

    int r = 0;
    int c = 0;
    while(check != from) {
        check = check->parent;
        steps++;
    }
    return steps;
}
NodeItem* Robot::bestTravel(NodeItem* current) {
    NodeItem* target;
    // partial[i][j] stores how many 0s is on the shortest path to a certain position
    int partial[row][col];
    for (int i  = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            partial[i][j] = 0;
        }
    }
    // get every partial[i][j]
    for (int i  = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (map[i][j] == 0) {
                target = new NodeItem(i,j,0);
                NodeItem*** path = ShortestPath_to_R_BFS(current, target);
                int zero = countZeros(path, current, target);
                partial[i][j] = zero;
            }    
        }
    }
    // return the best option
    int best = 0;
    int x = 0;
    int y = 0; 
    for (int i  = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (map[i][j] == 0 && partial[i][j] > best ) {
                best = partial[i][j];
                x = i;
                y = j;
            } 
        }
    }
    NodeItem* bestoption = new NodeItem(x,y,0);
    cout<<"how many zeros on path: "<<best<<endl;
    return bestoption;
}

bool Robot::AllClean() {
    for(int i = 0; i < row; i++) 
    {
        for(int j = 0; j < col; j++)
        {
            if(map[i][j] == 0) return false;
        }
    }
    return true;
}

bool Robot::isValidStep(NodeItem* now, int b) {
    stack<NodeItem*> stk;
    int r = now->row;
    int c = now->col;
    int w = now->weight;
    NodeItem* next;
    // Move Up Valid
    if(r-1 >= 0 && map[r-1][c] == 0 ) {
        next = new NodeItem(r-1,c,w+1);
        //vector<vector<NodeItem*> > valid = ShortestPath_to_R_BFS(next, root);
        NodeItem*** valid = ShortestPath_to_R_BFS(next, root);
        NodeItem* check = valid[root->row][root->col];
        int toRoot = 0;
        while(!stk.empty()) {
            stk.pop();
        }
        while(check != next) {
            toRoot++;
            check = check->parent;
        }
        if(b-toRoot >0)return true;
    }
    // Move Down Valid
    else if(r+1 <= row-1 && map[r+1][c] == 0 ) {
        next = new NodeItem(r+1,c,w+1);
        //vector<vector<NodeItem*> > valid = ShortestPath_to_R_BFS(next, root);
        NodeItem*** valid = ShortestPath_to_R_BFS(next, root);
        NodeItem* check = valid[root->row][root->col];
        int toRoot = 0;

        while(!stk.empty()) {
            stk.pop();
        }
        while(check != next) {
            toRoot++;
            check = check->parent;
        }
        if(b-toRoot >0)return true;
    }
    //Move Left Valid
    else if(c-1 >= 0 && map[r][c-1] == 0 ) {
        next = new NodeItem(r,c-1,w+1);
        //vector<vector<NodeItem*> > valid = ShortestPath_to_R_BFS(next, root);
        NodeItem*** valid = ShortestPath_to_R_BFS(next, root);
        NodeItem* check = valid[root->row][root->col];
        int toRoot = 0;

        while(!stk.empty()) {
            stk.pop();
        }
        while(check != next) {
            toRoot++;
            check = check->parent;
        }
        if(b-toRoot >0)return true;
    }
    //Move Right Valid
    else if(c+1 <= col-1 && map[r][c+1] == 0 ) {
        next = new NodeItem(r,c+1,w+1);
        //vector<vector<NodeItem*> > valid = ShortestPath_to_R_BFS(next, root);
        NodeItem*** valid = ShortestPath_to_R_BFS(next, root);
        NodeItem* check = valid[root->row][root->col];
        int toRoot = 0;
        while(!stk.empty()) {
            stk.pop();
        }
        while(check != next) {
            toRoot++;
            check = check->parent;
        }
        if(b-toRoot >0)return true;
    }
    return false;
}

void Robot::pushSteps(NodeItem*** path, NodeItem* from, NodeItem* to) {
    stack<NodeItem*> s;
    NodeItem* temp = path[to->row][to->col];
    while (!s.empty()) {s.pop();}
    int r = 0;
    int c = 0;
    while(temp != from) {
        s.push(temp);
        temp = temp->parent;
    } 
    while (!s.empty()) {
        step.push(s.top());
        r = s.top()->row;
        c = s.top()->col; 
        map[r][c] = 1;
        track.push(s.top());
        cout<<"[ "<< s.top()->row<<", "<<s.top()->col<<" ]"<<endl;
        s.pop();
    }
}
void Robot::Move() {
    int count = 0;
    stack<NodeItem*> s1;
    stack<NodeItem*> s2;
    stack<NodeItem*> s3;
    stack<NodeItem*> s4;
    stack<NodeItem*> back;
    int batterylife = battery;

    while(!AllClean()) 
    {
        
        cout<<"Energy now: "<<batterylife<<endl;
        NodeItem* now = track.top();
        cout<<"Now position: "<<"[ "<< now->row<<", "<<now->col<<" ]"<<endl;
        int r = now->row;
        int c = now->col;
        int w = now->weight;
        /* Calculate the shortest path from now to root */
        NodeItem*** valid = ShortestPath_to_R_BFS(now, root);
        NodeItem* check = valid[root->row][root->col];
        int toRoot = 0;
        while(!s4.empty()) {
            s4.pop();
        }
        while(check != now) {
            toRoot++;
            s4.push(check);
            check = check->parent;
        }
        cout<<endl;
        // Normal situation: battery is still enough 
        if (isValidStep(now, batterylife) ) {
            cout<<"GO! "<<endl;
            cout<<endl;
            NodeItem* newnode;
            // Moving up
            if(r-1 >= 0 && map[r-1][c] == 0 ) {
                //visiting
                newnode = new NodeItem(r-1, c, w+1);
                map[r-1][c] = 1;
                track.push(newnode);
                step.push(newnode);
                batterylife--;
            }
            // Moving down
            else if(r+1 <= row-1 && map[r+1][c] == 0 ) {
                //visiting
                newnode = new NodeItem(r+1, c, w+1);
                map[r+1][c] = 1;
                track.push(newnode);
                step.push(newnode);
                batterylife--;
            }
            // Moving Left
            else if(c-1 >= 0 && map[r][c-1] == 0 ) {
                //visiting
                newnode = new NodeItem(r, c-1, w+1);
                map[r][c-1] = 1;
                track.push(newnode);
                step.push(newnode);
                batterylife--;    
            }
            // Moving Right
            else if(c+1 <= col-1 && map[r][c+1] == 0 ) {
                //visiting
                newnode = new NodeItem(r, c+1, w+1);
                map[r][c+1] = 1;
                track.push(newnode);
                step.push(newnode);
                batterylife--;
            }
            
        }
        // Dead-End: 1. Search for the node that has unvisited adjancey node(s)
        else if ( Deadend(track.top()) ) {
            cout<<"DeadEnd Occurs"<<endl;
            NodeItem* from = track.top();
            cout<<"From the dead-end node: "<<"[ "<< from->row<<", "<<from->col<<" ]"<<endl;
            // fix here : where to go is better? shortest path = feasible + most unvisited nodes ==> scan the whole board
            while(Deadend(track.top())) {
                track.pop();
            }
            NodeItem* to = track.top();
            
            cout<<"Back to the node with unvisited adjancey nodes: "<<"[ "<< to->row<<", "<<to->col<<" ]"<<endl;
            cout<<endl;
            
            NodeItem*** destination = ShortestPath_to_R_BFS(from, to);
            cout<<endl;
            int counts = 0;
            NodeItem* temp = destination[to->row][to->col];
            //clear s1
            while(!s1.empty()) {
                s1.pop();
            }
            cout<<"to ==> from"<<endl;
            while(temp != from) {    
                s1.push(temp);
                temp = temp->parent;
                cout<<"[ "<< s1.top()->row<<", "<<s1.top()->col<<" ]"<<endl;
                counts++;
            }
           
            // if (to == root) then goRoot->parent == NULLs
            NodeItem***goRoot = ShortestPath_to_R_BFS(to, root);
            int counting = 0;
            NodeItem* temps = goRoot[root->row][root->col];

            // clear s2
            while(!s2.empty()) {
                s2.pop();
            }
            
            while(temps != to) {
                temps = temps->parent;
                s2.push(temps);
                counting++;
            } 
            
            // it is safe to go 
            if((batterylife - counts - counting  -2) >= 0) {
                cout<<"Safe to go!"<<endl;
                while(!s1.empty()) {
                    step.push(s1.top());
                    map[s1.top()->row][s1.top()->col] = 1;
                    s1.pop();
                    batterylife--;
                }
            }
            //  need to recharge first
            else {
                cout<<"\nRecharge: "<<(counts+counting)<<" >= "<<batterylife<<endl;
                /*Recharge*/
                NodeItem***goCharge = ShortestPath_to_R_BFS(from, root);
                NodeItem* tempss = goCharge[root->row][root->col];
                // clear s3
                while(!s3.empty()) {
                    s3.pop();
                }
                while(tempss != from) {
                    s3.push(tempss);
                    tempss = tempss->parent;
                }
                while(!s3.empty()) {
                    cout<<"[ "<< s3.top()->row<<", "<<s3.top()->col<<" ]"<<endl;
                    step.push(s3.top());
                    map[s3.top()->row][s3.top()->col] = 1;                    
                    s3.pop();
                }
                batterylife = battery;
                /*go to the best path*/
                NodeItem* best = bestTravel(root);
                cout<<"go to the best node after recharge: "<<"[ "<< best->row<<", "<<best->col<<" ]"<<endl;
                NodeItem*** newplace = ShortestPath_to_R_BFS(root, best);
                int steps = countSteps(newplace, root, best);
                pushSteps(newplace, root , best);
                track.push(best);
                batterylife -= steps;
            }
        }
        // Rechrge time: 
        else {
            cout<<"Recharge time!"<<endl;
            /* Recharge */
            while(!s4.empty()) {
                cout<<"[ "<< s4.top()->row<<", "<<s4.top()->col<<" ]"<<endl;
                step.push(s4.top());
                track.push(s4.top());
                back.push(s4.top());
                s4.pop();
            }
            batterylife = battery;
            cout<<"Now at: "<<"[ "<<track.top()->row<<", "<<track.top()->col<<" ]"<<endl;
            cout<<"energy: "<<batterylife<<endl;
            NodeItem* best = bestTravel(track.top());
            cout<<"go to the best node after recharge: "<<"[ "<< best->row<<", "<<best->col<<" ]"<<endl;
            NodeItem*** newplace = ShortestPath_to_R_BFS(track.top(), best);
            int steps = countSteps(newplace, track.top(), best); // top == root
            stack<NodeItem*> s;
            NodeItem* temp = newplace[best->row][best->col];
            while (!s.empty()) {s.pop();}
            int r = 0;
            int c = 0;
            while(temp != track.top()) {
                s.push(temp);
                temp = temp->parent;
            } 
            while (!s.empty()) {
                step.push(s.top());
                r = s.top()->row;
                c = s.top()->col; 
                map[r][c] = 1;
                track.push(s.top());
                cout<<"[ "<< s.top()->row<<", "<<s.top()->col<<" ]"<<endl;
                s.pop();
            }
            cout<<"steps: "<<steps<<endl;
            batterylife -= steps;
        }
        PrintMap();
        cout<<"step: "<<step.size()-1<<endl;
        cout<<endl;      
    }
    //while loop ends
    /*
    After all clean if (stack.top() != root) then shortestPath back to root
    */
   cout<<"Timetogohome! now at position: "<<"[ "<< track.top()->row<<", "<<track.top()->col<<" ]"<<endl;
   NodeItem***Home = ShortestPath_to_R_BFS(track.top(), root);
   NodeItem* temp = Home[root->row][root->col];
    while(temp != track.top()) {
        s3.push(temp);
        temp = temp->parent;
    }
    while(!s3.empty()) {
        step.push(s3.top());
        s3.pop();
    }
    cout<<"Total: "<<steps()<<endl;
}

void Robot::outStep(ofstream& outFile) {
    while(!step.empty())
    {
        outFile<<step.front()->row<<" "<<step.front()->col<<endl;
        step.pop();
    }
}

void Robot::PrintMap(){
    for(int i = 0; i < row; i++)
    {
        for(int j = 0; j < col;j++)
        {
            cout<<map[i][j];
        }
        cout<<endl;
    }
}

int main() {

    ifstream file("floor.data");
    int r,c,b;
    file>>r>>c>>b;
    char** input = new char*[r];
    for(int i = 0; i < r; i++)
    {
        input[i] = new char[c];
    }
    for(int i = 0; i < r; i++)
    {
        for(int j = 0; j < c;j++)
        {
            file>>input[i][j];
        }
    }
    Robot robot(r,c,b,input);
    robot.PrintMap();
    cout<<endl;
    robot.Move();
    ofstream outFile("final.path", ios::out); 
    outFile<<robot.steps()<<endl;
    robot.outStep(outFile);
}

/*
        // choose the farthest node
        int weightUp = abs(r-1 - root->row)+ abs(c - root->col);
        int weightDown = abs(r+1 - root->row) + abs(c - root->col);
        int weightLeft = abs(r - root->row) + abs(c-1 - root->col);
        int weightRight = abs(r - root->row) + abs(c+1 - root->col);
    
        int* weightArray = new int[4];
        weightArray[0] = weightUp; weightArray[1] = weightDown; weightArray[2] = weightLeft; weightArray[3] = weightRight;
        bool* Go = new bool[4];
        for(int i = 0; i < 4; i++) {
             int farthest = weightArray[i];
             Go[i] = false;
             cout<< farthest<<" ";
        }
        cout<<endl;
        if(r-1 >= 0 && map[r-1][c] == 0 ) Go[0] = true;
        if(r+1 <= row-1 && map[r+1][c] == 0 ) Go[1] = true;
        if(c-1 >= 0 && map[r][c-1] == 0 ) Go[2] = true;
        if(c+1 >= 0 && map[r][c+1] == 0 ) Go[3] = true;
        
        int farthest = 0;
        for(int i = 0; i < 4; i++) {
            if (farthest < weightArray[i] && Go[i]) { 
                farthest = weightArray[i];      
            }
        }
        
        // Moving up
        if(farthest == weightUp && Go[0]) {
            //visiting
            newnode = new NodeItem(r-1, c, w+1);
            map[r-1][c] = 1;
            track.push(newnode);
        }
        // Moving down
        else if(farthest == weightDown && Go[1]) {
            //visiting
            newnode = new NodeItem(r+1, c, w+1);
            map[r+1][c] = 1;
            track.push(newnode);
        }
        // Moving Left
        else if(farthest == weightLeft && Go[2]) {
            //visiting
            newnode = new NodeItem(r, c-1, w+1);
            map[r][c-1] = 1;
            track.push(newnode);    
        }
        // Moving Right
        else if(farthest == weightRight && Go[3]) {
            //visiting
            newnode = new NodeItem(r, c+1, w+1);
            map[r][c+1] = 1;
            track.push(newnode);
        }
        else {
            map[r][c] = 2;
            track.pop();
        }
 */
        